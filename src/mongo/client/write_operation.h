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

#include "mongo/util/net/message.h"

namespace mongo {

    /**
     * Represents a single server side write operation and encapsulates
     * the process for encoding the operation into either a wire protocl
     * request message or a command.
     */
    class WriteOperation {
    public:
        virtual ~WriteOperation() {}

        /**
         * Returns the MongoDB wire protocol operation type represented
         * by an instance of this particular write operation.
         */
        virtual Operations operationType() const = 0;

        /**
         * Appends the preamble for a wire protocol message into the supplied
         * BufBuilder.
         *
         * This is the part of the wire protocol message after the header but
         * before the bson document portion. It typically contains the namespace,
         * flags, and potentially space reserved by the protocol for future use.
         *
         * NOTE: The size of the preamble is fixed but operation type dependant.
         */
        virtual void startRequest(const std::string& ns, BufBuilder* b) const = 0;

        /**
         * Appends a document (or documents in the case of update) which describe
         * the write operation represented by an instance of this class into the
         * supplied BufBuilder.
         *
         * This method will be called multiple times by a WireProtocolriter in order
         * to batch operations of the same type into a single wire protocol request.
         *
         * It returns a boolean value that indicates whether adding this particular
         * operation's data to the request was successful. If it returns false, this
         * signals to the WireProtocolWriter that the batch should be considered full.
         *
         * NOTE: The size of this portion of the message is flexible but the size of
         * the message itself is bounded by the server's maxMessageSizeBytes.
         */
        virtual bool appendSelfToRequest(int maxSize, BufBuilder* b) const = 0;

        /**
         * Appends the preamble for a write command into the supplied BSONObjBuilder.
         *
         * Typically this involves a single bson element having a key that represents
         * the operation type and a value which represents the collection to which the
         * operation will be applied.
         */
        virtual void startCommand(const std::string& ns, BSONObjBuilder* b) const = 0;

        /**
         * Appends a single document that describes the write operation represented by
         * an instance of this class into the supplied BSONArrayBuilder.
         *
         * This method will be called multiple times by a DBClientWriter in order to batch
         * operations of the same type into a single command.
         *
         * It returns a boolean value that indicates whether adding this particular
         * operation's data to the request was successful. If it returns false, this
         * signals to the CommandWriter that the batch should be considered full.
         *
         * NOTE: The ultimate size of the of the command is bounded by the sever's
         * maxBsonObjectSize.
         */
        virtual bool appendSelfToCommand(BSONArrayBuilder* batch) const = 0;

        /**
         * Ends a single write command by appending the BSONArray representing a
         * batch of write operations into the command and injecting the ordered
         * excecution parameter (which can be operation parameter dependent).
         */
        virtual void endCommand(BSONArrayBuilder* batch, BSONObjBuilder* command) const = 0;
    };

} // namespace mongo
