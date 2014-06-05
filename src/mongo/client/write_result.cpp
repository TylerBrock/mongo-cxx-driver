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

    void WriteResult::mergeCommandResult(Operations opType, const std::vector<WriteOperation*>& ops, const BSONObj& result) {
        int64_t affected = result.hasField("n") ? result.getIntField("n") : 0;

        // Handle Write Batch
        switch(opType) {
            case dbInsert:
                _nInserted += affected;
                break;

            case dbDelete:
                _nRemoved += affected;
                break;

            case dbUpdate:
                if (result.hasField("upserted")) {
                    BSONElement upserted = result.getField("upserted");
                    int nUpserted = 0;

                    std::vector<BSONElement> upsertedArray = upserted.Array();
                    nUpserted = upsertedArray.size();

                    std::vector<BSONElement>::const_iterator it;
                    for (it = upsertedArray.begin(); it != upsertedArray.end(); ++it) {
                        BSONObj upsertedObj = (*it).Obj();
                        int batchIndex = upsertedObj.getIntField("index");

                        BSONObjBuilder bob;
                        bob.append("index", static_cast<long long>(ops[batchIndex]->getSequenceId()));
                        bob.appendAs(upsertedObj["_id"], "_id");

                        _upserted.push_back(bob.obj());
                    }

                    _nUpserted += nUpserted;
                    _nMatched += (affected - nUpserted);

                } else {
                    _nMatched += affected;
                }

                // SERVER-13001 - mixed shared cluster could return nModified for
                // (servers >= 2.6) or not (servers <= 2.4). If any call does not
                // return nModified we cannot report a valid final count.
                if (result.hasField("nModified"))
                    _nModified += result.getIntField("nModified");
                else
                    _hasModifiedCount = false;

                break;

            default:
                uassert(0, "Cannot make WriteResult non write opType", false);
        }

        // Handle Write Errors
        if (result.hasField("writeErrors")) {
            std::vector<BSONElement> writeErrors = result.getField("writeErrors").Array();
            std::vector<BSONElement>::const_iterator it;

            for (it = writeErrors.begin(); it != writeErrors.end(); ++it) {
                BSONObj writeError = (*it).Obj();
                int batchIndex = writeError.getIntField("index");

                BSONObjBuilder bob;
                bob.append("index", static_cast<long long>(ops[batchIndex]->getSequenceId()));
                bob.append("code", writeError.getIntField("code"));
                bob.append("errmsg", writeError.getStringField("errmsg"));

                BSONObjBuilder builder;
                ops[batchIndex]->appendSelfToBSONObj(&builder);
                bob.append("op", builder.obj());

                if (writeError.hasField("errInfo"))
                    bob.append("details", writeError.getObjectField("errInfo"));

                _writeErrors.push_back(bob.obj());
            }
        }

        // Handle Write Concern Errors
        if (result.hasField("writeConcernError")) {
            BSONObj writeConcernError = result.getObjectField("writeConcernError");
            BSONObjBuilder bob;

            bob.append("code", writeConcernError.getIntField("code"));
            bob.append("errmsg", writeConcernError.getStringField("errmsg"));

            if (writeConcernError.hasField("errInfo"))
                bob.append("details", writeConcernError.getObjectField("errInfo"));

            _writeConcernErrors.push_back(result.getObjectField("writeConcernError"));
        }
    }

    void WriteResult::mergeGleResult(Operations opType, const std::vector<WriteOperation*>& ops, const BSONObj& result) {
        int64_t affected = result.hasField("n") ? result.getIntField("n") : 0;

        // Handle Errors
        std::string errmsg;
        if (result.hasField("errmsg"))
            errmsg = result.getStringField("errmsg");
        else if (result.hasField("err"))
            errmsg = result.getStringField("err");

        if (!errmsg.empty()) {
            if (result.hasField("wtimeout")) {
                BSONObjBuilder bob;
                bob.append("errmsg", errmsg);
                bob.append("code", kWriteConcernErrorCode);
                _writeConcernErrors.push_back(bob.obj());
            } else {
                int code = result.hasField("code") ? result.getIntField("code") : kUnknownError;
                BSONObjBuilder bob;
                bob.append("index", static_cast<long long>(ops.front()->getSequenceId()));
                bob.append("code", code);
                bob.append("errmsg", errmsg);

                BSONObjBuilder builder;
                ops.front()->appendSelfToBSONObj(&builder);
                bob.append("op", builder.obj());

                _writeErrors.push_back(bob.obj());

                return;
            }
        }

        // Handle Write Batch
        switch(opType) {
            case dbInsert:
                _nInserted += 1;
                break;

            case dbDelete:
                _nRemoved += affected;
                break;

            case dbUpdate:
                if (result.hasField("upserted")) {
                    BSONElement upserted = result.getField("upserted");
                    BSONObjBuilder bob;
                    bob.append("index", static_cast<long long>(ops.front()->getSequenceId()));
                    bob.appendAs(upserted, "_id");
                    _upserted.push_back(bob.obj());
                    _nUpserted += affected;
                } else {
                    _nMatched += affected;
                }

                // SERVER-13001 - mixed shared cluster could return nModified for
                // (servers >= 2.6) or not (servers <= 2.4). If any call does not
                // return nModified we cannot report a valid final count.
                if (result.hasField("nModified"))
                    _nModified += result.getIntField("nModified");
                else
                    _hasModifiedCount = false;

                break;

            default:
                uassert(0, "Cannot make WriteResult non write opType", false);
        }
    }

    void WriteResult::requireDetailedInsertResults() {
        _requiresDetailedInsertResults = true;
    }

    bool WriteResult::requiresDetailedInsertResults() const {
        return _requiresDetailedInsertResults;
    }

} // namespace mongo