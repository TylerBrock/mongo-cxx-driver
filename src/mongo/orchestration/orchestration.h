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

#include <iostream>
#include <string>

#include "third_party/restclient/restclient.h"

namespace mongo {
namespace orchestration {

    using namespace std;

    class Resource {
    public:
        Resource(string base_url);

    protected:
        RestClient::response get(string relative_path="");
        RestClient::response put(string relative_path="", string payload="{}");
        RestClient::response post(string relative_path="", string payload="{}");
        RestClient::response del(string relative_path="");

        string make_url(string relative_path);

        static const char _content_type[];
        string _url;
    };

    class Hosts;
    class ReplicaSets;
    class Clusters;

    class API : public Resource {
    public:
        API(string url);
        Hosts hosts();
        ReplicaSets replica_sets();
        Clusters clusters();
    private:
    };

    class Host;

    class Hosts : public Resource {
    public:
        Hosts(string url);
        Host* create(string process_type = "mongod");
        Host* ensure(string id);
    };

    class Host : public Resource {
    public:
        Host(string url);
        void start();
        void stop();
        void restart();
        void destroy();
        string uri();

    private:
        RestClient::response status();
    };


    class ReplicaSets {
    };

    class ReplicaSet {
    };

    class Clusters{
    };

    class Cluster{
    };

} // namespace orchestration
} // namespace mongo
