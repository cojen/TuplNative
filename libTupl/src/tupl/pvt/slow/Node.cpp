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
    const size_t keySize = residentSizeOf(key);
    // ensure that a split always produces siblings with at least two keys
    if (keySize > maxBytes() / 4) { 
        throw std::invalid_argument("key too big");
    }
    
    const auto newBytes = mBytes + keySize;
    
    if (newBytes > maxBytes()) { return InsertResult::FAILED_NO_SPACE; }
    
    const auto oldSize = mChildren.size();
    
    mChildren.emplace_hint(
        position.mIt, Buffer{key.data(), key.size()}, &node);

    if (oldSize == mChildren.size()) { throw std::logic_error("key exists"); }

    mBytes = newBytes;
    
    return InsertResult::INSERTED;
}

InsertResult LeafNode::insert(
    Iterator position, RawBytes key, RawBytes value)
{    
    const size_t keySize = residentSizeOf(key);
    if (keySize > maxBytes() / 4) {
        throw std::invalid_argument("key too big");
    }

    const size_t kvSize = residentSizeOf(key, value);
    if ((kvSize - keySize) > maxBytes() / 4) {
        throw std::invalid_argument("value too big");
    }
    
    const auto newBytes = mBytes + kvSize;
    
    if (newBytes > maxBytes()) { return InsertResult::FAILED_NO_SPACE; }
    const auto oldSize = mValues.size();

    mValues.insert(position.base(),
                   std::make_pair(Buffer{key.data(), key.size()},
                                  Buffer{value.data(), value.size()}));
    
    if (oldSize == mValues.size()) { throw std::logic_error("key exists"); }
    
    mBytes = newBytes;
    
    return InsertResult::INSERTED;
}

LeafNode::Iterator LeafNode::moveEntries(
    LeafNode& src, Iterator srcBegin, Iterator srcEnd,
    LeafNode& /* dst */, Iterator tgtBegin)
{
    auto srcBeginIt = srcBegin.base();
    auto srcEndIt = srcEnd.base();
    
    auto retVal = std::move(srcBeginIt, srcEndIt, tgtBegin.base());
    
    src.mValues.erase(srcBeginIt, srcEndIt);
    
    return {retVal, BufferPairToBytesPair()};
}

} } }
