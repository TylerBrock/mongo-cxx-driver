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

#include "driver/base/bulk_write.hpp"

#include "driver/base/private/bulk_write.hpp"
#include "driver/libmongoc.hpp"
#include "driver/util/libbson.hpp"

namespace mongo {
namespace driver {

using namespace bson::libbson;

bulk_write::bulk_write(bulk_write&&) = default;
bulk_write& bulk_write::operator=(bulk_write&&) = default;
bulk_write::~bulk_write() = default;

bulk_write::bulk_write(bool ordered) : _impl(new impl{libmongoc::bulk_operation_new(ordered)}) {}

void bulk_write::append(const model::write& operation) {
    switch (operation.type()) {
        case model::write_type::kInsertOne: {
            scoped_bson_t doc(operation.get_insert_one().document());

            mongoc_bulk_operation_insert(_impl->operation_t, doc.bson());
            break;
        }
        case model::write_type::kUpdateOne: {
            scoped_bson_t filter(operation.get_update_one().filter());
            scoped_bson_t update(operation.get_update_one().update());
            bool upsert = operation.get_update_one().upsert().value_or(false);

            mongoc_bulk_operation_update_one(_impl->operation_t, filter.bson(), update.bson(),
                                             upsert);
            break;
        }
        case model::write_type::kUpdateMany: {
            scoped_bson_t filter(operation.get_update_many().filter());
            scoped_bson_t update(operation.get_update_many().update());
            bool upsert = operation.get_update_many().upsert().value_or(false);

            mongoc_bulk_operation_update(_impl->operation_t, filter.bson(), update.bson(), upsert);
            break;
        }
        case model::write_type::kDeleteOne: {
            scoped_bson_t filter(operation.get_delete_one().filter());
            mongoc_bulk_operation_remove_one(_impl->operation_t, filter.bson());
            break;
        }
        case model::write_type::kDeleteMany: {
            scoped_bson_t filter(operation.get_delete_many().filter());
            mongoc_bulk_operation_remove(_impl->operation_t, filter.bson());
            break;
        }
        case model::write_type::kReplaceOne: {
            scoped_bson_t filter(operation.get_replace_one().filter());
            scoped_bson_t replace(operation.get_replace_one().replacement());
            bool upsert = operation.get_replace_one().upsert().value_or(false);

            mongoc_bulk_operation_replace_one(_impl->operation_t, filter.bson(), replace.bson(),
                                              upsert);
            break;
        }
        case model::write_type::kUninitialized:
            break;  // TODO: something exceptiony
    }
}

void bulk_write::write_concern(class write_concern wc) { _impl->_write_concern = std::move(wc); }

optional<class write_concern> bulk_write::write_concern() const { return _impl->_write_concern; }

}  // namespace driver
}  // namespace mongo
