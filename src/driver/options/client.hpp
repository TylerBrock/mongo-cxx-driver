// Copyright 2014 MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "driver/config/prelude.hpp"

#include <string>

#include "driver/options/ssl.hpp"
#include "stdx/optional.hpp"

namespace mongo {
namespace driver {
namespace options {

// NOTE: client_t options interface still evolving
class LIBMONGOCXX_EXPORT client_t {

   public:
    void ssl_opts(ssl_t ssl_opts);
    const optional<ssl_t>& ssl_opts() const;

   private:
    optional<ssl_t> _ssl_opts;

}; // class client_t

}  // namespace options
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
