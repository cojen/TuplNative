#ifndef _TUPL_MAKE_UNIQUE_HPP
#define _TUPL_MAKE_UNIQUE_HPP

#include <memory>

#if __cplusplus == 201103L

#include <utility>

namespace tupl {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

namespace std {

using tupl::make_unique;

}

#endif

#endif
