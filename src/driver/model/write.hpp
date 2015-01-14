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

#include <cstdint>

#include "driver/base/write_type.hpp"
#include "driver/model/insert_one.hpp"
#include "driver/model/delete_one.hpp"
#include "driver/model/delete_many.hpp"
#include "driver/model/update_one.hpp"
#include "driver/model/update_many.hpp"
#include "driver/model/replace_one.hpp"
#include "stdx/optional.hpp"

namespace mongo {
namespace driver {
namespace model {

class LIBMONGOCXX_EXPORT write_t {

   public:
    write_t(insert_one_t value);
    write_t(update_one_t value);
    write_t(update_many_t value);
    write_t(delete_one_t value);
    write_t(delete_many_t value);
    write_t(replace_one_t value);

    write_t(write_t&& rhs);
    write_t& operator=(write_t&& rhs);

    write_t(const write_t& rhs) = delete;
    write_t& operator=(const write_t& rhs) = delete;

    ~write_t();

    write_type_t type() const;

    const insert_one_t& get_insert_one() const;
    const update_one_t& get_update_one() const;
    const update_many_t& get_update_many() const;
    const delete_one_t& get_delete_one() const;
    const delete_many_t& get_delete_many() const;
    const replace_one_t& get_replace_one() const;

   private:
    void destroy_member();

    write_type_t _type;

    union {
        insert_one_t _insert_one;
        update_one_t _update_one;
        update_many_t _update_many;
        delete_one_t _delete_one;
        delete_many_t _delete_many;
        replace_one_t _replace_one;
    };

}; // class write_t

}  // namespace model
}  // namespace driver
}  // namespace mongo

#include "driver/config/postlude.hpp"
