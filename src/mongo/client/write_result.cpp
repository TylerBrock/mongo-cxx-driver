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
#include "mongo/client/write_result.h"

namespace mongo {

    WriteResult::WriteResult()
        : _nInserted(0)
        , _nUpserted(0)
        , _nMatched(0)
        , _nModified(0)
        , _nRemoved(0)
    {}

    bool WriteResult::hasErrors() const {
        return !(_writeErrors.empty() && _writeConcernErrors.empty());
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
        return _nModified;
    }

    int WriteResult::nRemoved() const {
        return _nRemoved;
    }

    void WriteResult::merge(Operations opType, const std::vector<int>& sequenceIds, const BSONObj& result) {
        int affected = result.hasField("n") ? result.getIntField("n") : 0;

        // Handle Write Operation
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

                    if (upserted.isSimpleType()) {
                        nUpserted += 1;
                        BSONObjBuilder bob;
                        bob.append("index", 0);
                        bob.append("_id", upserted.OID());
                        _upserted.push_back(bob.obj());
                    } else {
                        std::vector<BSONElement> upsertedArray = upserted.Array();
                        nUpserted = upsertedArray.size();

                        std::vector<BSONElement>::const_iterator it;
                        for (it = upsertedArray.begin(); it != upsertedArray.end(); ++it) {
                            int batchIndex = (*it).Obj().getIntField("index");
                            const OID id = (*it).Obj()["_id"].OID();

                            BSONObjBuilder bob;
                            bob.append("index", sequenceIds[batchIndex]);
                            bob.append("_id", id);

                            _upserted.push_back(bob.obj());
                        }
                    }

                    _nUpserted += nUpserted;
                    _nMatched += (affected - nUpserted);

                } else {
                    _nMatched += affected;
                }

                _nModified += result.getIntField("nModified");
                // TODO: SERVER-13001
                break;

            default:
                uassert(0, "something really bad happened", false);
        }

        // Handle Write Errors
        if (result.hasField("writeErrors")) {
            std::vector<BSONElement> writeErrors = result.getField("writeErrors").Array();
            std::vector<BSONElement>::const_iterator it;

            for (it = writeErrors.begin(); it != writeErrors.end(); ++it) {
                _writeErrors.push_back((*it).Obj());
            }
        }

        // Handle Write Concern Errors
        if (result.hasField("writeConcernError")) {
            _writeConcernErrors.push_back(result.getObjectField("writeConcernError"));
        }
    }

} // namespace mongo
