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

#include <string>

#include "mongo/bson/bsonobj.h"

namespace mongo {

    class WriteError {
    public:
        WriteError(int64_t index, int32_t code, std::string message, BSONObj op, BSONObj details);
        int64_t index() const;
        int32_t code() const;
        std::string message() const;
        const BSONObj& op() const;
        const BSONObj& details() const;

    private:
        int64_t _index;
        int32_t _code;
        std::string _message;
        BSONObj _op;
        BSONObj _details;
    };

} // namespace mongo
