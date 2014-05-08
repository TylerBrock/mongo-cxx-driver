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

#include "mongo/db/namespace_string.h"
#include "mongo/client/command_writer.h"

namespace mongo {

    CommandWriter::CommandWriter(DBClientBase* client) : _client(client) {
    }

    CommandWriter::~CommandWriter() {
    }

    std::vector<BSONObj> CommandWriter::write(
        const StringData& ns,
        const std::vector<WriteOperation*>& write_operations,
        int flags,
        const WriteConcern* wc
    ) {
        bool inRequest = false;
        int opsInRequest = 0;
        Operations requestType;

        std::vector<BSONObj> responses;
        BSONObjBuilder command;
        BSONArrayBuilder batch;

        std::vector<WriteOperation*>::const_iterator iter;
        iter = write_operations.begin();

        while (iter != write_operations.end()) {
            // We don't have a pending command yet
            if (!inRequest) {
                (*iter)->startCommand(ns.toString(), flags, &command);
                inRequest = true;
                requestType = (*iter)->operationType();
            }

            // Now we have a pending request, can we add to it?
            if (requestType == (*iter)->operationType() && opsInRequest < 1000) {

                // We can add to the request, so try and the write op
                bool addedToRequest = (*iter)->appendSelfToCommand(&batch);

                // We were able to add this write op into the request so don't send it off yet
                if (addedToRequest) {
                    ++opsInRequest;
                    ++iter;
                    continue;
                }
            }

            // Send the current request to the server, record the response, start a new request
            (*iter)->endCommand(&batch, &command);
            responses.push_back(_send(&command, wc, ns));
            inRequest = false;
            opsInRequest = 0;
        }

        // Last batch
        if (opsInRequest != 0)
            --iter;
            (*iter)->endCommand(&batch, &command);
            responses.push_back(_send(&command, wc, ns));

        return responses;
    }

    BSONObj CommandWriter::_send(BSONObjBuilder* builder, const WriteConcern* wc, const StringData& ns) {
        BSONObj result;
        const WriteConcern* operation_wc = wc ? wc : &_client->getWriteConcern();
        builder->append("writeConcern", operation_wc->obj());
        builder->append("ordered", true);
        _client->runCommand(nsToDatabase(ns), builder->obj(), result);
        return result;
    }

} // namespace mongo
