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

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "boost/scoped_ptr.hpp"
#include "third_party/restclient/restclient.h"
#include "third_party/rapidjson/document.h"

namespace mongo {
namespace orchestration {

    using namespace std;
    using namespace boost;
    using namespace rapidjson;

    class Resource {
    public:
        Resource(string base_url);

    protected:
        RestClient::response get(string relative_path="") const;
        RestClient::response put(string relative_path="", string payload="{}");
        RestClient::response post(string relative_path="", string payload="{}");
        RestClient::response del(string relative_path="");

        string make_url(string relative_path) const;
        auto_ptr<Document> handle_response(RestClient::response response) const;

        static const char _content_type[];
        string _url;
    };

    class Host;
    class ReplicaSet;
    class Cluster;

    class API : public Resource {

    public:
        API(string url);
        vector<Host> hosts() const;
        vector<ReplicaSet> replica_sets() const;
        vector<Cluster> clusters() const;

        string createMongod(const string& id = "");
        string createMongos(const string& id = "");
        string createReplicaSet(const string& id = "");
        string createCluster(Document params);

        Host host(const string& id) const;
        ReplicaSet replica_set(const string& id) const;
        Cluster cluster(const string& id) const;

    private:
        template <typename T>
        vector<T> get_plural_resource(const string& resource_name) const {
            vector<T> resources;

            RestClient::response result = get(resource_name);
            Document result_array;
            result_array.Parse(result.body.c_str());
            assert(result_array.IsArray());

            Value::ConstValueIterator resource_iter = result_array.Begin();
            while (resource_iter != result_array.End()) {
                T temp(make_url(resource_name + "/" + resource_iter->GetString()));
                resources.push_back(temp);
                ++resource_iter;
            }

            return resources;
        }
    };

    class Host : public Resource {

        friend class API;
        friend class ReplicaSet;
        friend class Cluster;

    public:
        void start();
        void stop();
        void restart();
        void destroy();
        string uri() const;

    private:
        Host(const string& url);
        RestClient::response status() const;
    };

    class ReplicaSet : public Resource {

        friend class API;
        friend class Cluster;

    public:
        Host primary() const;
        void destroy();
        string uri() const;
        RestClient::response status() const;
        vector<Host> secondaries() const;
        vector<Host> arbiters() const;
        vector<Host> hidden() const;
        vector<Host> members() const;

    private:
        ReplicaSet(const string& url);
    };

    class Cluster : public Resource {

        friend class API;

    public:
        vector<Host> members() const;
        vector<Host> configs() const;
        vector<Host> routers() const;

    private:
        Cluster(const string& url);
    };

} // namespace orchestration
} // namespace mongo
