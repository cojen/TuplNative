TuplNative
==========

Partial reimplementation of a Tupl like B+Tree in C++This is a re-implementation of the mini branch of Tupl in an unsafe programming
language (Standard C++11).

If performance is superior, then this may turn into a feature complete
re-implementation of Tupl in idiomatic C++. The current effort is to complete a
proof of concept for the sake of analysis and will likely preserve Java'ish
nature of the API.

Naming Conventions:

Members:

*Private members are prefixed by "m". Example: "mName". Except:

**Private members that form part of the class's interface in the case of
  friendship are not prefixed

*Members that are meant to not be accessed directly get a "_" suffix.
 Example boost::intrusive member hooks

**Members that are private and are not meant to be accessed directly get
  both the "m" prefix and the "_" suffix. Example: "mAlwaysUseAccessor_"
  
