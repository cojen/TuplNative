#include "Node.hpp"

namespace tupl { namespace pvt { namespace slow {

std::size_t residentSizeOf(const RawBytes& key) {
    return key.size() + 8; // whatever
}

std::size_t residentSizeOf(const RawBytes& key, const RawBytes& value) {
    return key.size() + value.size() + 8;
}

InternalNode::Iterator InternalNode::lowerBound(RawBytes key) {
    const Buffer keyBuf(key.data(), key.size());
    return mChildren.lower_bound(keyBuf);
}

InsertResult InternalNode::insert(
    Iterator position, RawBytes key, Node& node)
{
    const auto newBytes = mBytes + residentSizeOf(key);
    
    if (newBytes > capacity()) { return InsertResult{false}; }
    
    const auto oldSize = mChildren.size();
    
    mChildren.emplace_hint(
        position.mIt, Buffer{key.data(), key.size()}, &node);

    if (oldSize == mChildren.size()) { throw std::logic_error("key exists"); }

    mBytes = newBytes;
    
    return InsertResult{true};
}

InsertResult LeafNode::insert(
    Iterator position, RawBytes key, RawBytes value)
{
    const auto newBytes = mBytes + residentSizeOf(key, value);
    
    if (newBytes > capacity()) { return InsertResult{false}; }
    
    const auto oldSize = mValues.size();
    
    mValues.emplace_hint(position.mIt,
                         Buffer{key.data(), key.size()},
                         Buffer{value.data(), value.size()});
    
    if (oldSize == mValues.size()) { throw std::logic_error("key exists"); }
    
    mBytes = newBytes;
    
    return InsertResult{true};
}

} } }
