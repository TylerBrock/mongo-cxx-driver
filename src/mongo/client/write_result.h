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

#include "mongo/client/write_concern_error.h"
#include "mongo/client/write_error.h"
#include "mongo/util/net/operation.h"

namespace mongo {

    class BSONObj;
    class WriteOperation;

    class WriteResult {
    public:
        WriteResult();

        /* Introspection */
        bool hasErrors() const;
        bool hasWriteErrors() const;
        bool hasWriteConcernErrors() const;
        bool hasModifiedCount() const;

        /* Data */
        int nInserted() const;
        int nUpserted() const;
        int nMatched() const;
        int nModified() const;
        int nRemoved() const;

        const std::vector<BSONObj>& upserted() const;

        /* Errors Data */
        const std::vector<BSONObj>& writeErrors() const;
        const std::vector<BSONObj>& writeConcernErrors() const;

        /* Functional */
        void mergeCommandResult(const std::vector<WriteOperation*>& ops, const BSONObj& result);
        void mergeGleResult(const std::vector<WriteOperation*>& ops, const BSONObj& result);

        /* Modifiers */
        void requireDetailedInsertResults();
        bool requiresDetailedInsertResults() const;

    private:
        /* Helper Memeber Functions */
        void _setModified(const BSONObj& result);
        int _getIntOrDefault(const BSONObj& obj, const char* field, const int defaultValue = 0);
        int _createUpserts(const BSONElement& upsert, const std::vector<WriteOperation*>& ops);
        void _createUpsert(const BSONElement& upsert, const std::vector<WriteOperation*>& ops);
        void _createWriteError(const BSONObj& error, const std::vector<WriteOperation*>& ops);
        void _createWriteConcernError(const BSONObj& error);

        /* Member Variables */
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
