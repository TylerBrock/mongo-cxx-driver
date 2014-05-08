/*    Copyright 2014 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "mongo/client/dbclientinterface.h"
#include "mongo/client/update_write_operation.h"
#include "mongo/db/namespace_string.h"

namespace {
    const char kCommandKey[] = "update";
    const char kBatchKey[] = "updates";
    const char kMultiKey[] = "multi";
    const char kUpsertKey[] = "upsert";
    const char kSelectorKey[] = "q";
    const char kUpdateKey[] = "u";
    const char kOrderedKey[] = "ordered";
} // namespace

namespace mongo {

    UpdateWriteOperation::UpdateWriteOperation(const BSONObj& selector, const BSONObj& update, int flags)
        : _selector(selector)
        , _update(update)
        , _flags(flags)
        {}

    UpdateWriteOperation::~UpdateWriteOperation() {
    }

    Operations UpdateWriteOperation::operationType() const {
        return dbUpdate;
    }

    void UpdateWriteOperation::startRequest(const std::string& ns, BufBuilder* builder) const {
        builder->appendNum(0);
        builder->appendStr(ns);
        builder->appendNum(_flags);
    }

    bool UpdateWriteOperation::appendSelfToRequest(int maxSize, BufBuilder* builder) const {
        if (builder->getSize() + _selector.objsize() + _update.objsize() > maxSize)
            return false;

        _selector.appendSelfToBufBuilder(*builder);
        _update.appendSelfToBufBuilder(*builder);
        return true;
    }

    void UpdateWriteOperation::startCommand(const std::string& ns, BSONObjBuilder* builder) const {
        builder->append(kCommandKey, nsToCollectionSubstring(ns));
    }

    bool UpdateWriteOperation::appendSelfToCommand(BSONArrayBuilder* batch) const {
        BSONObjBuilder updateBuilder;
        updateBuilder.append(kSelectorKey, _selector);
        updateBuilder.append(kUpdateKey, _update);
        updateBuilder.append(kMultiKey, bool(_flags & UpdateOption_Multi));
        updateBuilder.append(kUpsertKey, bool(_flags & UpdateOption_Upsert));
        batch->append(updateBuilder.obj());
        return true;
    }

    void UpdateWriteOperation::endCommand(BSONArrayBuilder* batch, BSONObjBuilder* builder) const {
        builder->append(kBatchKey, batch->arr());
        builder->append(kOrderedKey, true);
    }

} // namespace mongo
