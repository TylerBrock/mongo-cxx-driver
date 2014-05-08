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

#include "mongo/bson/bsonobj.h"
#include "mongo/client/write_concern.h"
#include "mongo/db/namespace_string.h"
#include "mongo/util/net/message.h"

#pragma once

namespace mongo {

    class WriteOperation {
    public:
        virtual ~WriteOperation() {}

        virtual Operations operationType() const = 0;

        virtual void startRequest(std::string ns, int flags, BufBuilder* b) const = 0;
        virtual bool appendSelfToRequest(int maxSize, BufBuilder* b) const = 0;

        virtual void startCommand(std::string ns, int flags, BSONObjBuilder* b) const = 0;
        virtual bool appendSelfToCommand(BSONArrayBuilder* batch) const = 0;
        virtual void endCommand(BSONArrayBuilder* batch, BSONObjBuilder* command) const = 0;
    };

} // namespace mongo
