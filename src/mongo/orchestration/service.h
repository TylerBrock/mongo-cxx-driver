/**
 * Copyright 2014 MongoDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "mongo/orchestration/cluster.h"
#include "mongo/orchestration/replica_set.h"
#include "mongo/orchestration/server.h"

namespace mongo {
namespace orchestration {

    using namespace std;

    class Service : public Resource {

    public:
        Service(string url);
        vector<Server> servers() const;
        vector<ReplicaSet> replica_sets() const;
        vector<Cluster> clusters() const;

        string createMongod(const Json::Value& params = Json::Value());
        string createMongos(const Json::Value& params = Json::Value());
        string createReplicaSet(const Json::Value& params = Json::Value());
        string createCluster(const Json::Value& params = Json::Value());

        Server server(const string& id) const;
        ReplicaSet replica_set(const string& id) const;
        Cluster cluster(const string& id) const;
    };

} // namespace orchestration
} // namespace mongo
