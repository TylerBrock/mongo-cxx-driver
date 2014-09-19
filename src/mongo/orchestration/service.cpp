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
    } // namespace

    Service::Service(const std::string& url)
        : Resource(url)
    {}

    std::vector<Server> Service::servers() const {
        return pluralResource<Server>(kServers);
    }

    std::vector<ReplicaSet> Service::replica_sets() const {
        return pluralResource<ReplicaSet>(kReplicaSets);
    }

    std::vector<ShardedCluster> Service::clusters() const {
        return pluralResource<ShardedCluster>(kShardedClusters);
    }

    Server Service::server(const std::string& id) const {
        return Server(relativeUrl(std::string(kServers).append("/").append(id)));
    }

    ReplicaSet Service::replicaSet(const std::string& id) const {
        return ReplicaSet(relativeUrl(std::string(kReplicaSets).append("/").append(id)));
    }

    std::string Service::createMongod(Document params) {
        params["name"] = "mongod";
        params["procParams"]["setParameter"]["enableTestCommands"] = 1;

        Json::FastWriter writer;

        RestClient::response result = post(kServers, writer.write(params));
        Document result_doc = handleResponse(result);
        return result_doc["id"].asString();
    }

    std::string Service::createReplicaSet(const Document& params) {
        Json::FastWriter writer;
        Document result_doc = handleResponse(post(kReplicaSets, writer.write(params)));
        return result_doc["id"].asString();
    }

    std::string Service::createShardedCluster(const Document& params) {
        return "Not yet implemented";
    }

} // namespace orchestration
} // namespace mongo
