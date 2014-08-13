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

    namespace {
        const char kRequiredVersion[] = "0.9";
        const char kAPIVersion[] = "v1";
        const char kServers[] = "servers";
        const char kReplicaSets[] = "replica_sets";
        const char kShardedClusters[] = "sharded_clusters";
    }

    namespace Resources {
    }

    Service::Service(const string& url)
        : Resource(url)
    {}

    vector<Server> Service::servers() const {
        return plural_resource<Server>(kServers);
    }

    vector<ReplicaSet> Service::replica_sets() const {
        return plural_resource<ReplicaSet>(kReplicaSets);
    }

    vector<ShardedCluster> Service::clusters() const {
        return plural_resource<ShardedCluster>(kShardedClusters);
    }

    Server Service::server(const string& id) const {
        return Server(relative_url(kServers + string("/") + id));
    }

    ReplicaSet Service::replica_set(const string& id) const {
        return ReplicaSet(relative_url(kReplicaSets + string("/") + id));
    }

    string Service::createMongod(const Document& params) {
        Document doc(params);

        doc["name"] = "mongod";
        doc["procParams"]["setParameter"]["enableTestCommands"] = 1;

        Json::FastWriter writer;

        RestClient::response result = post(kServers, writer.write(doc));
        Document result_doc = handle_response(result);
        return result_doc["id"].asString();
    }

    string Service::createReplicaSet(const Document& params) {
        Json::FastWriter writer;
        Document result_doc = handle_response(post(kReplicaSets, writer.write(params)));
        return result_doc["id"].asString();
    }

    string Service::createShardedCluster(const Document& params) {
        return "";
    }

} // namespace orchestration
} // namespace mongo
