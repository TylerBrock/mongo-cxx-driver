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

#include "bson.h"
#include "bson/builder.hpp"
#include "bson/util/itoa.hpp"
#include "bson/util/stack.hpp"
#include "bson/types.hpp"

#include <cstring>
#include <deque>
#include <vector>

namespace bson {

class builder::impl {
   public:
    impl() : has_user_key(false) { bson_init(&_root); }

    ~impl() {
        while (!_stack.empty()) {
            _stack.pop_back();
        }

        bson_destroy(&_root);
    }

    void reinit() {
        while (!_stack.empty()) {
            _stack.pop_back();
        }

        bson_reinit(&_root);

        has_user_key = false;
    }

    document::value steal() {
        while (!_stack.empty()) {
            _stack.pop_back();
        }

        uint32_t buf_len;
        uint8_t* buf_ptr = bson_destroy_with_steal(&_root, true, &buf_len);
        bson_init(&_root);

        return document::value{buf_ptr, buf_len};
    }

    bson_t* back() {
        if (_stack.empty()) {
            return &_root;
        } else {
            return &_stack.back().bson;
        }
    }

    void push_back_document(const char* key, std::size_t len) {
        _stack.emplace_back(back(), key, len, false);
    }

    void push_back_array(const char* key, std::size_t len) {
        _stack.emplace_back(back(), key, len, true);
    }

    void pop_back() { _stack.pop_back(); }

    const string_or_literal& next_key() {
        if (is_array()) {
            itoa_key = _stack.back().n++;
            user_key = string_or_literal{itoa_key.c_str(), itoa_key.length()};
        } else if (!has_user_key) {
            throw std::runtime_error("no user specified key and not in an array context");
        }

        has_user_key = false;

        return user_key;
    }

    void push_key(string_or_literal sol) {
        user_key = std::move(sol);
        has_user_key = true;
    }

    bson_t* root() { return &_root; }

    bool is_array() { return _stack.empty() ? false : _stack.back().is_array; }

   private:
    struct frame {
        frame(bson_t* parent, const char* key, std::size_t len, bool is_array)
            : n(0), is_array(is_array), parent(parent) {
            if (is_array) {
                bson_append_array_begin(parent, key, len, &bson);
            } else {
                bson_append_document_begin(parent, key, len, &bson);
            }
        }

        ~frame() {
            if (is_array) {
                bson_append_array_end(parent, &bson);
            } else {
                bson_append_document_end(parent, &bson);
            }
        }

        std::size_t n;
        bool is_array;
        bson_t bson;
        bson_t* parent;
    };

    util::stack<frame, 4> _stack;

    bson_t _root;

    util::itoa itoa_key;
    string_or_literal user_key;

    bool has_user_key;
};

namespace builder_helpers {
open_doc_t open_doc;
close_doc_t close_doc;
open_array_t open_array;
close_array_t close_array;
}

builder::builder() : _impl(new impl()) {}
builder::builder(builder&&) = default;
builder& builder::operator=(builder&&) = default;
builder::~builder() = default;

builder::operator key_ctx<closed_ctx>() { return key_ctx<closed_ctx>(this); }

builder::document_ctx<builder::key_ctx<builder>> builder::operator<<(string_or_literal rhs) {
    key_ctx<builder> ctx(this);

    return ctx << std::move(rhs);
}

builder& builder::operator<<(builder_helpers::concat concat) {
    concat_append(concat);

    return *this;
}

void builder::key_append(string_or_literal key) {
    if (_impl->is_array()) {
        throw(std::runtime_error("in subarray"));
    }
    _impl->push_key(std::move(key));
}

void builder::value_append(const types::b_double& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_double(_impl->back(), key.c_str(), key.length(), value.value);
}

void builder::value_append(const types::b_utf8& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_utf8(_impl->back(), key.c_str(), key.length(), value.value.c_str(),
                     value.value.length());
}

void builder::value_append(const types::b_document& value) {
    const string_or_literal& key = _impl->next_key();
    bson_t bson;
    bson_init_static(&bson, value.value.get_buf(), value.value.get_len());

    bson_append_document(_impl->back(), key.c_str(), key.length(), &bson);
}

void builder::value_append(const types::b_array& value) {
    const string_or_literal& key = _impl->next_key();
    bson_t bson;
    bson_init_static(&bson, value.value.get_buf(), value.value.get_len());

    bson_append_array(_impl->back(), key.c_str(), key.length(), &bson);
}

void builder::value_append(const types::b_binary& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_binary(_impl->back(), key.c_str(), key.length(),
                       static_cast<bson_subtype_t>(value.sub_type), value.bytes, value.size);
}

void builder::value_append(const types::b_undefined&) {
    const string_or_literal& key = _impl->next_key();

    bson_append_undefined(_impl->back(), key.c_str(), key.length());
}

void builder::value_append(const types::b_oid& value) {
    const string_or_literal& key = _impl->next_key();
    bson_oid_t oid;
    std::memcpy(&oid.bytes, value.value.bytes(), sizeof(oid.bytes));

    bson_append_oid(_impl->back(), key.c_str(), key.length(), &oid);
}

void builder::value_append(const types::b_bool& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_bool(_impl->back(), key.c_str(), key.length(), value.value);
}

void builder::value_append(const types::b_date& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_date_time(_impl->back(), key.c_str(), key.length(), value.value);
}

void builder::value_append(const types::b_null&) {
    const string_or_literal& key = _impl->next_key();

    bson_append_null(_impl->back(), key.c_str(), key.length());
}

void builder::value_append(const types::b_regex& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_regex(_impl->back(), key.c_str(), key.length(), value.regex.c_str(),
                      value.options.c_str());
}

void builder::value_append(const types::b_dbpointer& value) {
    const string_or_literal& key = _impl->next_key();

    bson_oid_t oid;
    std::memcpy(&oid.bytes, value.value.bytes(), sizeof(oid.bytes));

    bson_append_dbpointer(_impl->back(), key.c_str(), key.length(), value.collection.c_str(), &oid);
}

void builder::value_append(const types::b_code& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_code(_impl->back(), key.c_str(), key.length(), value.code.c_str());
}

void builder::value_append(const types::b_symbol& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_symbol(_impl->back(), key.c_str(), key.length(), value.symbol.c_str(),
                       value.symbol.length());
}

void builder::value_append(const types::b_codewscope& value) {
    const string_or_literal& key = _impl->next_key();

    bson_t bson;
    bson_init_static(&bson, value.scope.get_buf(), value.scope.get_len());

    bson_append_code_with_scope(_impl->back(), key.c_str(), key.length(), value.code.c_str(),
                                &bson);
}

void builder::value_append(const types::b_int32& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_int32(_impl->back(), key.c_str(), key.length(), value.value);
}

void builder::value_append(const types::b_timestamp& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_timestamp(_impl->back(), key.c_str(), key.length(), value.increment,
                          value.timestamp);
}

void builder::value_append(const types::b_int64& value) {
    const string_or_literal& key = _impl->next_key();

    bson_append_int64(_impl->back(), key.c_str(), key.length(), value.value);
}

void builder::value_append(const types::b_minkey&) {
    const string_or_literal& key = _impl->next_key();

    bson_append_minkey(_impl->back(), key.c_str(), key.length());
}

void builder::value_append(const types::b_maxkey&) {
    const string_or_literal& key = _impl->next_key();

    bson_append_maxkey(_impl->back(), key.c_str(), key.length());
}

void builder::value_append(double value) {
    value_append(types::b_double{value});
}

void builder::value_append(string_or_literal value) {
    value_append(types::b_utf8{std::move(value)});
}

void builder::value_append(std::int32_t value) {
    value_append(types::b_int32{value});
}

void builder::value_append(const oid& value) {
    value_append(types::b_oid{value});
}

void builder::value_append(std::int64_t value) {
    value_append(types::b_int64{value});
}

void builder::value_append(bool value) {
    value_append(types::b_bool{value});
}

void builder::open_doc_append() {
    const string_or_literal& key = _impl->next_key();

    _impl->push_back_document(key.c_str(), key.length());
}

void builder::open_array_append() {
    const string_or_literal& key = _impl->next_key();

    _impl->push_back_array(key.c_str(), key.length());
}

void builder::concat_append(const document::view& view) {
    bson_t other;
    bson_init_static(&other, view.get_buf(), view.get_len());

    if (_impl->is_array()) {
        bson_iter_t iter;
        bson_iter_init(&iter, &other);

        while (bson_iter_next(&iter)) {
            const string_or_literal& key = _impl->next_key();

            bson_append_iter(_impl->back(), key.c_str(), key.length(), &iter);
        }

    } else {
        bson_concat(_impl->back(), &other);
    }
}

void builder::value_append(const element& value) {
    const string_or_literal& key = _impl->next_key();

    bson_iter_t iter;
    iter.raw = value._raw;
    iter.len = value._len;
    iter.next_off = value._off;
    bson_iter_next(&iter);

    bson_append_iter(_impl->back(), key.c_str(), key.length(), &iter);
}

void builder::close_doc_append() {
    if (_impl->is_array()) {
        // TODO handle insufficient stack
        throw(std::runtime_error("in subdocument or insufficient stack"));
    }

    _impl->pop_back();
}

void builder::close_array_append() {
    if (!_impl->is_array()) {
        // TODO handle stack
        throw(std::runtime_error("in subdocument or insufficient stack"));
    }

    _impl->pop_back();
}

document::view builder::view() const {
    return document::view(bson_get_data(_impl->root()), _impl->root()->len);
}

document::value builder::extract() {
    return document::view(bson_get_data(_impl->root()), _impl->root()->len);
}

void builder::clear() { _impl->reinit(); }

}  // namespace bson
