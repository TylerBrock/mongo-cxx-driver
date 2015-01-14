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

#include "driver/model/write.hpp"

namespace mongo {
namespace driver {
namespace model {

write_t::write_t(insert_one_t value) : _type(write_type_t::k_insert_one), _insert_one(std::move(value)) {}
write_t::write_t(delete_one_t value) : _type(write_type_t::k_delete_one), _delete_one(std::move(value)) {}
write_t::write_t(delete_many_t value)
    : _type(write_type_t::k_delete_many), _delete_many(std::move(value)) {}
write_t::write_t(update_one_t value) : _type(write_type_t::k_update_one), _update_one(std::move(value)) {}
write_t::write_t(update_many_t value)
    : _type(write_type_t::k_update_many), _update_many(std::move(value)) {}
write_t::write_t(replace_one_t value)
    : _type(write_type_t::k_replace_one), _replace_one(std::move(value)) {}

write_t::write_t(write_t&& rhs) : _type(write_type_t::k_uninitialized) { *this = std::move(rhs); }

void write_t::destroy_member() {
    switch (_type) {
        case write_type_t::k_insert_one:
            _insert_one.~insert_one_t();
            break;
        case write_type_t::k_update_one:
            _update_one.~update_one_t();
            break;
        case write_type_t::k_update_many:
            _update_many.~update_many_t();
            break;
        case write_type_t::k_delete_one:
            _delete_one.~delete_one_t();
            break;
        case write_type_t::k_delete_many:
            _delete_many.~delete_many_t();
            break;
        case write_type_t::k_replace_one:
            _replace_one.~replace_one_t();
            break;
        case write_type_t::k_uninitialized:
            break;
    }

    _type = write_type_t::k_uninitialized;
}

write_t& write_t::operator=(write_t&& rhs) {
    destroy_member();

    switch (rhs._type) {
        case write_type_t::k_insert_one:
            _insert_one = std::move(rhs._insert_one);
            break;
        case write_type_t::k_update_one:
            _update_one = std::move(rhs._update_one);
            break;
        case write_type_t::k_update_many:
            _update_many = std::move(rhs._update_many);
            break;
        case write_type_t::k_delete_one:
            _delete_one = std::move(rhs._delete_one);
            break;
        case write_type_t::k_delete_many:
            _delete_many = std::move(rhs._delete_many);
            break;
        case write_type_t::k_replace_one:
            _replace_one = std::move(rhs._replace_one);
            break;
        case write_type_t::k_uninitialized:
            break;
    }

    _type = rhs._type;

    return *this;
}

write_type_t write_t::type() const { return _type; }

const insert_one_t& write_t::get_insert_one() const { return _insert_one; }
const update_one_t& write_t::get_update_one() const { return _update_one; }
const update_many_t& write_t::get_update_many() const { return _update_many; }
const delete_one_t& write_t::get_delete_one() const { return _delete_one; }
const delete_many_t& write_t::get_delete_many() const { return _delete_many; }
const replace_one_t& write_t::get_replace_one() const { return _replace_one; }

write_t::~write_t() { destroy_member(); }

}  // namespace model
}  // namespace driver
}  // namespace mongo
