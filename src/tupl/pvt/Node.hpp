/*
 *  Copyright (C) 2012-2014 Brian S O'Neill
 *  Copyright (C) 2014 Vishal Parakh
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef _TUPL_PVT_NODE_HPP
#define _TUPL_PVT_NODE_HPP

#include <vector>

namespace tupl { namespace pvt {

class PageAllocator;
class Page;
class TreeCursorFrame;

class Node {
    class Header {};
    class Split  {};
    class Id {};
    
public:
    Node();
    
private:
    Header mHeader;
    
    // FIXME: Node.java has this volatile, but that makes being typed
    //        complicated. Understand and verify usage
    Id mId;
    
    // FIX DOC: Links within usage list, guarded by Database.mUsageLatch.
    Node* mMoreUsed; // points to more recently used node
    Node* mLessUsed; // points to less recently used node
    
    // Links within dirty list, guarded by OrderedPageAllocator.
    Node* mNextDirty;
    Node* mPrevDirty;
    
    // Linked stack of TreeCursorFrames bound to this Node.    
    TreeCursorFrame* mLastCursorFrame;
    
    // Raw contents of node.
    Page* mPage;
    
    // TODO: Memory managment foo, THINK, overallocate Node??
    std::vector<Node*> mChildNodes;

    // Records a partially completed split
    Split mSplit;
    
    friend class ::tupl::pvt::PageAllocator;
};

/*
  Nodes define the contents of Trees and UndoLogs. All node types start
  with a two byte header.

  +----------------------------------------+
  | byte:   node type                      |  header
  | byte:   reserved (must be 0)           |
  -                                        -

  There are two types of tree nodes, having a similar structure and
  supporting a maximum page size of 65536 bytes. The ushort type is an
  unsigned byte pair, and the ulong type is eight bytes. All multibyte
  types are little endian encoded.

  +----------------------------------------+
  | byte:   node type                      |  header
  | byte:   reserved (must be 0)           |
  | ushort: garbage in segments            |
  | ushort: pointer to left segment tail   |
  | ushort: pointer to right segment tail  |
  | ushort: pointer to search vector start |
  | ushort: pointer to search vector end   |
  +----------------------------------------+
  | left segment                           |
  -                                        -
  |                                        |
  +----------------------------------------+
  | free space                             | <-- left segment tail (exclusive)
  -                                        -
  |                                        |
  +----------------------------------------+
  | search vector                          | <-- search vector start (inclusive)
  -                                        -
  |                                        | <-- search vector end (inclusive)
  +----------------------------------------+
  | free space                             |
  -                                        -
  |                                        | <-- right segment tail (exclusive)
  +----------------------------------------+
  | right segment                          |
  -                                        -
  |                                        |
  +----------------------------------------+

  The left and right segments are used for allocating variable sized entries,
  and the tail points to the next allocation. Allocations are made toward the
  search vector such that the free space before and after the search vector
  remain the roughly the same. The search vector may move if a contiguous
  allocation is not possible on either side.

  The search vector is used for performing a binary search against keys. The
  keys are variable length and are stored anywhere in the left and right
  segments. The search vector itself must point to keys in the correct order,
  supporting binary search. The search vector is also required to be aligned to
  an even address, contain fixed size entries, and it never has holes. Adding
  or removing entries from the search vector requires entries to be
  shifted. The shift operation can be performed from either side, but the
  smaller shift is always chosen as a performance optimization.
      
  Garbage refers to the amount of unused bytes within the left and right
  allocation segments. Garbage accumulates when entries are deleted and updated
  from the segments. Segments are not immediately shifted because the search
  vector would also need to be repaired. A compaction operation reclaims
  garbage by rebuilding the segments and search vector. A copying garbage
  collection algorithm is used for this.

  The compaction implementation allocates all surviving entries in the left
  segment, leaving an empty right segment. There is no requirement that the
  segments be balanced -- this only applies to the free space surrounding the
  search vector.

  Leaf nodes support variable length keys and values, encoded as a pair, within
  the segments. Entries in the search vector are ushort pointers into the
  segments. No distinction is made between the segments because the pointers
  are absolute.

  Entries start with a one byte key header:

  0b0pxx_xxxx: key is 1..64 bytes 0b1pxx_xxxx: key is 0..16383 bytes

  When the 'p' bit is zero, the entry is a normal key. Otherwise, it indicates
  that the key starts with the node key prefix.

  For keys 1..64 bytes in length, the length is defined as ((header & 0x3f) +
  1). For keys 0..16383 bytes in length, a second header byte is used. The
  second byte is unsigned, and the length is defined as (((header & 0x3f) << 8)
  | header2). The key contents immediately follow the header byte(s).

  The value follows the key, and its header encodes the entry length:

  0b0xxx_xxxx: value is 0..127 bytes 0b1f0x_xxxx: value/entry is 1..8192 bytes
  0b1f10_xxxx: value/entry is 1..1048576 bytes 0b1111_1111: ghost value (null)

  When the 'f' bit is zero, the entry is a normal value. Otherwise, it is a
  fragmented value, defined by Database.fragment.

  For entries 1..8192 bytes in length, a second header byte is used. The length
  is then defined as ((((h0 & 0x1f) << 8) | h1) + 1). For larger entries, the
  length is ((((h0 & 0x0f) << 16) | (h1 << 8) | h2) + 1).  Node limit is
  currently 65536 bytes, which limits maximum entry length.

  The "values" for internal nodes are actually identifiers for child nodes. The
  number of child nodes is always one more than the number of keys. For this
  reason, the key-value format used by leaf nodes cannot be applied to internal
  nodes. Also, the identifiers are always a fixed length, ulong type.

  Child node identifiers are encoded immediately following the search
  vector. Free space management must account for this, treating it as an
  extension to the search vector.

*/

} }

#endif 
