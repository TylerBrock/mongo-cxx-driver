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

#pragma once

#include <vector>

#include "mongo/util/net/operation.h"

namespace mongo {

    class BSONObj;
    class WriteOperation;

    class WriteResult {
    public:
        WriteResult();

        bool hasErrors() const;
        bool hasWriteError() const;
        bool hasWriteConcernError() const;
        bool hasModifiedCount() const;

        int nInserted() const;
        int nUpserted() const;
        int nMatched() const;
        int nModified() const;
        int nRemoved() const;

        std::vector<BSONObj> upserted() const;

        //std::vector<WriteError> writeErrors() const;
        //std::vector<WriteConcernError> writeConcernErrors() const;

        void mergeCommandResult(Operations opType, const std::vector<WriteOperation*>& ops, const BSONObj& result);
        void mergeGleResult(Operations opType, const std::vector<WriteOperation*>& ops, const BSONObj& result);
        void requireDetailedInsertResults();
        bool requiresDetailedInsertResults() const;

    private:
        int _nInserted;
        int _nUpserted;
        int _nMatched;
        int _nModified;
        int _nRemoved;

        std::vector<BSONObj> _upserted;
        std::vector<BSONObj> _writeErrors;
        std::vector<BSONObj> _writeConcernErrors;

        bool _hasModifiedCount;
        bool _requiresDetailedInsertResults;
    };

} // namespace mongo
