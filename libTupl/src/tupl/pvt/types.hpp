#ifndef _TUPL_PVT_TYPES_HPP
#define _TUPL_PVT_TYPES_HPP

#include <stddef.h>

namespace tupl { namespace pvt {

// GCC < 4.9 doesn't have this available in cstddef under std::
typedef long double MaxAlignT;
    
} } // namespace tupl::pvt

#endif
