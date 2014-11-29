#define BOOST_TEST_MODULE SlowNodeTest

#include <boost/test/unit_test.hpp>
#include <boost/container/string.hpp>

#include "tupl/pvt/slow/Node.hpp"

using std::ostringstream;
using std::string;
using tupl::pvt::slow::InsertResult;
using tupl::pvt::slow::LeafNode;

std::ostream& operator<<(std::ostream& os, const tupl::Bytes& bytes) {
    os.write((char*) bytes.data(), bytes.size());
    return os;
}

std::ostream& operator<<(std::ostream& os, LeafNode& node) {
    os << '{';
    for (const auto& kv : node) {
        os << kv.first << ':' << kv.second << ", ";
    }    
    os << '}';
    
    return os;
}

bool isOrdered(LeafNode& node) {
    if (node.size() < 2) { return true; }
    
    const auto end = node.end();
    auto prev = node.begin();
    auto it = prev;
    ++it;

    while (it != node.end()) {
        if (!(prev->first < it->first)) { return false; }
        ++it;
        ++prev;
    }
    
    return true;
}

BOOST_AUTO_TEST_CASE(LeafNodeBasicTest) {
    LeafNode node;

    size_t expectedBytes;
    
    const string key = "key-0";
    const string value = "value-0";
        
    BOOST_CHECK_EQUAL(0, node.size());
        
    BOOST_CHECK(
        InsertResult::INSERTED == node.insert(key, value));
        
    expectedBytes = key.size() + value.size();
    
    BOOST_CHECK_EQUAL(1, node.size());
    BOOST_CHECK_EQUAL(expectedBytes, node.bytes());
    
    BOOST_CHECK_THROW(node.insert(key, value), std::invalid_argument);
    BOOST_CHECK(isOrdered(node));
    
    for (size_t i = 1; node.bytes() < node.capacity(); ++i) {
        ostringstream keyStr;
        keyStr << "key-"  << i;
        
        ostringstream valueStr;
        valueStr << "value-"  << i;
        
        auto insertResult = node.insert(keyStr.str(), valueStr.str());
        
        if (insertResult == InsertResult::INSERTED) {
            expectedBytes += keyStr.str().size() + valueStr.str().size();
            BOOST_CHECK_EQUAL(expectedBytes, node.bytes());
        } else {
            BOOST_CHECK(insertResult == InsertResult::FAILED_NO_SPACE);
            break;
        }
    }    
}

BOOST_AUTO_TEST_CASE(LeafNodeSplitTest) {
    auto insertWithoutSplit = [](const string& key, const string& value,
                                 LeafNode& node)
    {
        auto insertResult = node.insert(key, value);
        
        if (insertResult != InsertResult::INSERTED) {
            BOOST_CHECK(insertResult == InsertResult::FAILED_NO_SPACE);
            BOOST_CHECK(isOrdered(node));

            const size_t origSize = node.size();
            
            LeafNode sibling;
            node.splitAndInsert(key, value, sibling);
            
            BOOST_CHECK(isOrdered(node));
            BOOST_CHECK(isOrdered(sibling));
            
            BOOST_CHECK_EQUAL(origSize + 1, node.size() + sibling.size());
            
            BOOST_CHECK_LE(origSize / 2, node.size());
            BOOST_CHECK_LE(origSize / 2, sibling.size());
            
            return false;
        }
        
        return true;
    };
    
    {
        LeafNode node;
        
        // Test ascending insert
        for (size_t i = 100; node.bytes() < node.capacity(); ++i) {
            ostringstream keyStr;
            keyStr << "bKey-"  << i;
        
            ostringstream valueStr;
            valueStr << "value-"  << i;

            if (!insertWithoutSplit(keyStr.str(), valueStr.str(), node)) {
                break;
            }
        }
    }
    
    {
        LeafNode node;
        // Test descending insert
        for (size_t i = 999; node.bytes() < node.capacity(); --i) {
            ostringstream keyStr;
            keyStr << "bKey-"  << i;
        
            ostringstream valueStr;
            valueStr << "value-"  << i;

            if (!insertWithoutSplit(keyStr.str(), valueStr.str(), node)) {
                break;
            }
        }
    }
}
