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

#include <memory>
#include <string>
#include <vector>

#include "third_party/jsoncpp/json.h"
#include "third_party/restclient/restclient.h"

// Posts to resources must not end with a slash or you will get a 404
// Posts containing a JSON encoded body must conform to strict JSON

namespace mongo {
namespace orchestration {

    using namespace std;

    typedef Json::Value Document;

    class Resource {
    public:
        Resource(string url);

    protected:
        RestClient::response get(string relative_path="") const;
        RestClient::response put(string relative_path="", string payload="{}");
        RestClient::response post(string relative_path="", string payload="{}");
        RestClient::response del(string relative_path="");

        RestClient::response action(string action);
        string make_url(string relative_path) const;
        auto_ptr<Document> handle_response(RestClient::response response) const;

        string _url;

        template <typename T>
        vector<T> plural_rooted_resource(const string& resource_name) const {
            vector<T> resources;
            auto_ptr<Document> doc = handle_response(get("secondaries"));

            for (unsigned i=0; i<doc->size(); i++) {
                string secondary_uri = (*doc)[i]["uri"].asString();
                T resource(_url.substr(0, _url.find("/")) + secondary_uri);
                resources.push_back(resource);
            }

            return resources;
        }

        template <typename T>
        vector<T> plural_resource(const string& resource_name) const {
            vector<T> resources;

            RestClient::response response = get(resource_name);
            Document result_array;
            Json::Reader reader;
            reader.parse(response.body.c_str(), result_array);

            for (unsigned i=0; i<result_array.size(); i++) {
                T temp(make_url(resource_name + "/" + result_array[i].asString()));
                resources.push_back(temp);
            }

            return resources;
        }
    };

} // namespace orchestration
} // namesace mongo
