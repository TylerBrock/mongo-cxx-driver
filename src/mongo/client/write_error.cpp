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

#include "mongo/client/write_error.h"

namespace mongo {

    WriteError::WriteError(int64_t index, int32_t code, std::string message, BSONObj op, BSONObj details)
        : _index(index)
        , _code(code)
        , _message(message)
        , _op(op)
        , _details(details)
    {}

    int64_t WriteError::index() const {
        return _index;
    }

    int32_t WriteError::code() const {
        return _code;
    }

    std::string WriteError::message() const {
        return _message;
    }

    const BSONObj& WriteError::op() const {
        return _op;
    }

    const BSONObj& WriteError::details() const {
        return _details;
    }

} // namespace mongo
