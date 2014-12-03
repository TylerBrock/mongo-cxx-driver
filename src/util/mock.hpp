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

#include <iostream>
#include <deque>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <type_traits>
#include <thread>

#include "driver/util/optional.hpp"

namespace mongo {
namespace util {

template <typename T>
class mock {};

template <typename R, typename... Args>
class mock<R (*)(Args...)> {
   public:
    using ptr = R (*)(Args...);
    using callback = std::function<R(ptr, Args...)>;
    using conditional = std::function<bool(Args...)>;

    class rule {
        friend class mock;

       public:
        rule(callback callback) : _callback(std::move(callback)) { times(1); }

        void times(int n) {
            drop_when([n](Args...) mutable->bool {
                if (n < 0) {
                    return true;
                } else if (n == 0) {
                    return false;
                } else {
                    return n--;
                }
            });
        }

        void forever() { return times(-1); }

        void drop_when(conditional conditional) { _conditional = std::move(conditional); }

       private:
        callback _callback;
        conditional _conditional;
    };

    class MockInstance {
    friend class mock;
    public:
     MockInstance(const MockInstance&) = delete;
     MockInstance* operator=(const MockInstance&) = delete;
     ~MockInstance() {
         _parent->destroy_active_instance();
     }

     rule& interpose(std::function<R(Args...)> func) {
         _callbacks.emplace_back([=](ptr, Args... args) { return func(args...); });

         return _callbacks.back();
     }

     template <typename T, typename... U>
     typename std::enable_if<std::is_same<T, R>::value, rule&>::type interpose(T r, U... rs) {
         std::vector<R> vec = {r, rs...};
         std::size_t i = 0;

         _callbacks.emplace_back([ vec, i ](ptr, Args... args) mutable->R {
             if (i == vec.size()) {
                 i = 0;
             }
             return vec[i++];
         });
         _callbacks.back().times(vec.size());

         return _callbacks.back();
     }

     rule& visit(std::function<void(Args...)> func) {
         _callbacks.emplace_back([=](ptr, Args... args) {
             func(args...);

             return _parent->_func(args...);
         });

         return _callbacks.back();
     }

     std::size_t depth() const { return _callbacks.size(); }

     bool empty() const { return _callbacks.empty(); }

    private:
     MockInstance(mock* parent) : _parent(parent) {}
     mock* _parent;
     std::deque<rule> _callbacks;
    };

    friend class MockInstance;

    mock(ptr func) : _func(func) {}

    R operator()(Args... args) {
        driver::optional<MockInstance*> instance = active_instance();
        if (instance) {
            MockInstance* instance_value = instance.value();
            while (!instance_value->_callbacks.empty()) {
                if (instance_value->_callbacks.front()._conditional(args...)) {
                    return instance_value->_callbacks.front()._callback(_func, args...);
                } else {
                    instance_value->_callbacks.pop_front();
                }
            }
        }

        return _func(args...);
    }

    std::unique_ptr<MockInstance> create_instance() {
        if (active_instance()) {
            throw std::runtime_error("Cannot create second mock instance in thread");
        }
        std::unique_ptr<MockInstance> mock_instance(new MockInstance(this));
        active_instance(&*mock_instance);
        return mock_instance;
    }

   private:
    driver::optional<MockInstance*> active_instance() {
        std::thread::id id = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(_active_instances_lock);
        auto iterator = _active_instances.find(id);
        if (iterator == _active_instances.end()) {
            return driver::optional<MockInstance*>();
        }
        return driver::optional<MockInstance*>(iterator->second);
    }
    void active_instance(MockInstance* instance) {
        std::thread::id id = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(_active_instances_lock);
        _active_instances[id] = instance;
    }

    void destroy_active_instance() {
        std::thread::id id = std::this_thread::get_id();
        std::lock_guard<std::mutex> lock(_active_instances_lock);
        _active_instances.erase(id);
    }

    std::map<std::thread::id, MockInstance*> _active_instances;
    std::mutex _active_instances_lock;
    ptr _func;
};

}  // namespace util
}  // namespace mongo
