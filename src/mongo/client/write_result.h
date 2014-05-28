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

#include "mongo/bson/bsonobj.h"
#include "mongo/util/net/operation.h"

namespace mongo {

    class WriteResult {
    public:
        bool hasErrors();
        bool hasWriteError();
        bool hasWriteConcernError();
        void merge(Operations opType, const std::vector<int>& indexes, const BSONObj& result);

    private:
        std::vector<BSONObj> _writeErrors;
        std::vector<BSONObj> _writeConcernErrors;
        int _nInserted;
        int _nUpserted;
        int _nMatched;
        int _nModified;
        int _nRemoved;
        std::vector<BSONObj> _upserted;
    };

} // namespace mongo
