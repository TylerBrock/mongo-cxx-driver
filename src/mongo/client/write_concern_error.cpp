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

#include "mongo/client/write_concern_error.h"

namespace mongo {

    WriteConcernError::WriteConcernError(int32_t code, std::string message, BSONObj details)
        : _code(code)
        , _message(message)
        , _details(details)
    {}

    int32_t WriteConcernError::code() const {
        return _code;
    }

    std::string WriteConcernError::message() const {
        return _message;
    }

    const BSONObj& WriteConcernError::details() const {
        return _details;
    }

} // namespace mongo
