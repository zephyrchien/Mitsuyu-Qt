#ifndef UTILS_H
#define UTILS_H

#include <memory>
namespace utils
{
    // support normal pointer only
    template<typename T, typename ...Args>
    typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T> >::type
    make_unique(Args&& ...args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif // UTILS_H
