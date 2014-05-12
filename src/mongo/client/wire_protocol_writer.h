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

#include "mongo/client/dbclient_writer.h"
#include "mongo/client/dbclientinterface.h"

#pragma once

namespace mongo {

    class WireProtocolWriter : public DBClientWriter {
    public:
        WireProtocolWriter(DBClientBase* client);

        virtual ~WireProtocolWriter();

        virtual std::vector<BSONObj> write(
            const StringData& ns,
            const std::vector<WriteOperation*>& write_operations,
            const WriteConcern* wc
        );

    private:
        BSONObj _send(Operations opCode, BufBuilder& builder, const WriteConcern* wc, const StringData& ns);
        DBClientBase* _client;
    };

} // namespace mongo
