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

#include <string>
#include <vector>

#include "third_party/jsoncpp/json.h"
#include "third_party/restclient/restclient.h"

namespace mongo {
namespace orchestration {

    using namespace std;

    class Resource {
    public:
        Resource(string base_url);

    protected:
        RestClient::response get(string relative_path="") const;
        RestClient::response put(string relative_path="", string payload="{}");
        RestClient::response post(string relative_path="", string payload="{}");
        RestClient::response del(string relative_path="");

        RestClient::response action(string action);
        string make_url(string relative_path) const;
        auto_ptr<Json::Value> handle_response(RestClient::response response) const;

        static const char _content_type[];
        string _url;

        template <typename T>
        vector<T> get_plural_resource(const string& resource_name) const {
            vector<T> resources;

            RestClient::response response = get(resource_name);
            Json::Value result_array;
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
