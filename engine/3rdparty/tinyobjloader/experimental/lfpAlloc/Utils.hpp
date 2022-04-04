#include <cstdint>

namespace lfpAlloc {
namespace detail {
template <std::size_t Val, std::size_t base = 2>
struct Log {
    enum { value = 1 + Log<Val / base, base>::value };
};

template <std::size_t base>
struct Log<1, base> {
    enum { value = 0 };
};

template <std::size_t base>
struct Log<0, base> {
    enum { value = 0 };
};
}
}
