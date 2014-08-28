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

#include "mongo/orchestration/service.h"

namespace mongo {
namespace orchestration {

    namespace Resources {
        const char kServers[] = "servers";
        const char kReplicaSets[] = "replica_sets";
        const char kShardedClusters[] = "sharded_clusters";
    }

    Service::Service(string url) : Resource(url) {}

    vector<Server> Service::servers() const {
        return plural_resource<Server>(Resources::kServers);
    }

    vector<ReplicaSet> Service::replica_sets() const {
        return plural_resource<ReplicaSet>(Resources::kReplicaSets);
    }

    vector<Cluster> Service::clusters() const {
        return plural_resource<Cluster>(Resources::kShardedClusters);
    }

    Server Service::server(const string& id) const {
        return Server(_url + "/servers/" + id);
    }

    ReplicaSet Service::replica_set(const string& id) const {
        return ReplicaSet(_url + "/replica_sets/" + id);
    }

    string Service::createMongod(const Json::Value& params) {
        Json::Value doc(params);

        doc["name"] = "mongod";
        doc["procParams"]["setParameter"]["enableTestCommands"] = 1;

        Json::FastWriter writer;

        RestClient::response result = post(Resources::kServers, writer.write(doc));
        auto_ptr<Json::Value> result_doc = handle_response(result);
        return (*result_doc)["id"].asString();
    }

    string Service::createReplicaSet(const Json::Value& params) {
        Json::FastWriter writer;

        auto_ptr<Json::Value> result_doc = handle_response(post(Resources::kReplicaSets, writer.write(params)));
        return (*result_doc)["id"].asString();
    }

} // namespace orchestration
} // namespace mongo
