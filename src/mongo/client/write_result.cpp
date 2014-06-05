/*    Copyright 2014 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "mongo/platform/basic.h"

#include "mongo/db/jsobj.h"
#include "mongo/client/write_operation.h"
#include "mongo/client/write_result.h"

namespace mongo {

    const int kUnknownError = 8;
    const int kWriteConcernErrorCode = 64;

    WriteResult::WriteResult()
        : _nInserted(0)
        , _nUpserted(0)
        , _nMatched(0)
        , _nModified(0)
        , _nRemoved(0)
        , _hasModifiedCount(true)
        , _requiresDetailedInsertResults(false)
    {}

    bool WriteResult::hasErrors() const {
        return hasWriteErrors() || hasWriteConcernErrors();
    }

    bool WriteResult::hasWriteErrors() const {
        return !_writeErrors.empty();
    }

    bool WriteResult::hasWriteConcernErrors() const {
        return !_writeConcernErrors.empty();
    }

    bool WriteResult::hasModifiedCount() const {
        return _hasModifiedCount;
    }

    int WriteResult::nInserted() const {
        return _nInserted;
    }

    int WriteResult::nUpserted() const {
        return _nUpserted;
    }

    int WriteResult::nMatched() const {
        return _nMatched;
    }

    int WriteResult::nModified() const {
        uassert(0, "this result does not have a modified count", _hasModifiedCount);
        return _nModified;
    }

    int WriteResult::nRemoved() const {
        return _nRemoved;
    }

    std::vector<BSONObj> WriteResult::upserted() const {
        return _upserted;
    }

    std::vector<BSONObj> WriteResult::writeErrors() const {
        return _writeErrors;
    }

    std::vector<BSONObj> WriteResult::writeConcernErrors() const {
        return _writeConcernErrors;
    }

    void WriteResult::mergeCommandResult(
        const std::vector<WriteOperation*>& ops,
        const BSONObj& result
    ) {
        int affected = _getIntOrDefault(result, "n");

        // Handle Write Batch
        switch(ops.front()->operationType()) {
            case dbInsert:
                _nInserted += affected;
                break;

            case dbDelete:
                _nRemoved += affected;
                break;

            case dbUpdate:
                if (result.hasField("upserted")) {
                    int nUpserted = _createUpserts(result.getField("upserted"), ops);
                    _nUpserted += nUpserted;
                    _nMatched += (affected - nUpserted);
                } else {
                    _nMatched += affected;
                }

                _setModified(result);

                break;
        }

        // Handle Write Errors
        if (result.hasField("writeErrors")) {
            std::vector<BSONElement> writeErrors = result.getField("writeErrors").Array();
            std::vector<BSONElement>::const_iterator it;

            for (it = writeErrors.begin(); it != writeErrors.end(); ++it)
                _createWriteError((*it).Obj(), ops);
        }

        // Handle Write Concern Errors
        if (result.hasField("writeConcernError")) {
            BSONObj writeConcernError = result.getObjectField("writeConcernError");
            _createWriteConcernError(writeConcernError);
        }
    }

    void WriteResult::mergeGleResult(const std::vector<WriteOperation*>& ops, const BSONObj& result) {
        int64_t affected = result.hasField("n") ? result.getIntField("n") : 0;

        // Handle Errors
        std::string errmsg;

        if (result.hasField("errmsg"))
            errmsg = result.getStringField("errmsg");
        else if (result.hasField("err"))
            errmsg = result.getStringField("err");

        if (!errmsg.empty()) {
            result.hasField("wtimeout")
                ? _createWriteConcernError(result)
                : _createWriteError(result, ops);

                return;
        }

        // Handle Write Batch
        switch(ops.front()->operationType()) {
            case dbInsert:
                _nInserted += 1;
                break;

            case dbDelete:
                _nRemoved += affected;
                break;

            case dbUpdate:
                if (result.hasField("upserted")) {
                    _createUpsert(result.getField("upserted"), ops);
                    _nUpserted += affected;
                } else {
                    _nMatched += affected;
                }

                _setModified(result);

                break;
        }
    }

    void WriteResult::requireDetailedInsertResults() {
        _requiresDetailedInsertResults = true;
    }

    bool WriteResult::requiresDetailedInsertResults() const {
        return _requiresDetailedInsertResults;
    }

    void WriteResult::_setModified(const BSONObj& result) {
        // SERVER-13001 - mixed shared cluster could return nModified for
        // (servers >= 2.6) or not (servers <= 2.4). If any call does not
        // return nModified we cannot report a valid final count.
        if (result.hasField("nModified"))
            _nModified += result.getIntField("nModified");
        else
            _hasModifiedCount = false;
    }

    int WriteResult::_getIntOrDefault(const BSONObj& obj, const char* field, const int defaultValue) {
        return obj.hasField(field) ? obj.getIntField(field) : defaultValue;
    }

    int WriteResult::_createUpserts(const BSONElement& upserted, const std::vector<WriteOperation*>& ops) {
        int nUpserted = 0;

        std::vector<BSONElement> upsertedArray = upserted.Array();
        nUpserted = upsertedArray.size();

        std::vector<BSONElement>::const_iterator it;

        for (it = upsertedArray.begin(); it != upsertedArray.end(); ++it)
            _createUpsert(*it, ops);

        return nUpserted;
    }

    void WriteResult::_createUpsert(const BSONElement& upsert, const std::vector<WriteOperation*>& ops) {
        int batchIndex = 0;
        BSONElement id;

        if (upsert.isABSONObj()) {
            batchIndex = _getIntOrDefault(upsert.Obj(), "index");
            id = upsert["_id"];
        } else {
            id = upsert;
        }

        BSONObjBuilder bob;
        bob.append("index", static_cast<long long>(ops[batchIndex]->getSequenceId()));
        bob.appendAs(id, "_id");

        _upserted.push_back(bob.obj());
    }

    void WriteResult::_createWriteError(const BSONObj& error, const std::vector<WriteOperation*>& ops) {
        int batchIndex = _getIntOrDefault(error, "index");
        int code = _getIntOrDefault(error, "code", kUnknownError);

        BSONObjBuilder bob;
        bob.append("index", static_cast<long long>(ops[batchIndex]->getSequenceId()));
        bob.append("code", code);
        bob.append("errmsg", error.getStringField("errmsg"));

        BSONObjBuilder builder;
        ops[batchIndex]->appendSelfToBSONObj(&builder);
        bob.append("op", builder.obj());

        if (error.hasField("errInfo"))
            bob.append("details", error.getObjectField("errInfo"));

        _writeErrors.push_back(bob.obj());
    }

    void WriteResult::_createWriteConcernError(const BSONObj& error) {
        BSONObjBuilder bob;
        int code = _getIntOrDefault(error, "code", kWriteConcernErrorCode);

        bob.append("code", code);
        bob.append("errmsg", error.getStringField("errmsg"));

        if (error.hasField("errInfo"))
            bob.append("details", error.getObjectField("errInfo"));

        _writeConcernErrors.push_back(bob.obj());
    }

} // namespace mongo
