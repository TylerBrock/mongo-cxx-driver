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

#include "driver/base/write_concern.hpp"

#include "driver/base/private/write_concern.hpp"

#include "stdx/make_unique.hpp"
#include "driver/private/libmongoc.hpp"

namespace mongo {
namespace driver {

write_concern_t::write_concern_t()
    : _impl{stdx::make_unique<impl>(mongoc_write_concern_new())}
{}

write_concern_t::write_concern_t(std::unique_ptr<impl>&& implementation) {
    _impl.reset(implementation.release());
}

write_concern_t::write_concern_t(write_concern_t&&) noexcept = default;
write_concern_t& write_concern_t::operator=(write_concern_t&&) noexcept = default;

write_concern_t::write_concern_t(const write_concern_t& other)
    : _impl(stdx::make_unique<impl>(libmongoc::write_concern_copy(other._impl->write_concern_t))) {}

write_concern_t& write_concern_t::operator=(const write_concern_t& other) {
    _impl.reset(
        stdx::make_unique<impl>(libmongoc::write_concern_copy(other._impl->write_concern_t)).release());
    return *this;
}

write_concern_t::~write_concern_t() = default;

void write_concern_t::fsync(bool fsync) {
    libmongoc::write_concern_set_fsync(_impl->write_concern_t, fsync);
}

void write_concern_t::journal(bool journal) {
    libmongoc::write_concern_set_journal(_impl->write_concern_t, journal);
}

void write_concern_t::nodes(std::int32_t confirm_from) {
    libmongoc::write_concern_set_w(_impl->write_concern_t, confirm_from);
}

void write_concern_t::tag(const std::string& confirm_from) {
    libmongoc::write_concern_set_wtag(_impl->write_concern_t, confirm_from.c_str());
}

void write_concern_t::majority(std::chrono::milliseconds timeout) {
    libmongoc::write_concern_set_wmajority(_impl->write_concern_t, timeout.count());
}

void write_concern_t::timeout(std::chrono::milliseconds timeout) {
    libmongoc::write_concern_set_wtimeout(_impl->write_concern_t, timeout.count());
}

bool write_concern_t::fsync() const {
    return libmongoc::write_concern_get_fsync(_impl->write_concern_t);
}

bool write_concern_t::journal() const {
    return libmongoc::write_concern_get_journal(_impl->write_concern_t);
}

std::int32_t write_concern_t::nodes() const {
    return libmongoc::write_concern_get_w(_impl->write_concern_t);
}

// TODO: should this be an optional... probably
std::string write_concern_t::tag() const {
    const char* tag_str = libmongoc::write_concern_get_wtag(_impl->write_concern_t);
    return tag_str ? tag_str : std::string();
}

bool write_concern_t::majority() const {
    return libmongoc::write_concern_get_wmajority(_impl->write_concern_t);
}

std::chrono::milliseconds write_concern_t::timeout() const {
    return std::chrono::milliseconds(
        libmongoc::write_concern_get_wtimeout(_impl->write_concern_t)
    );
}

}  // namespace driver
}  // namespace mongo
