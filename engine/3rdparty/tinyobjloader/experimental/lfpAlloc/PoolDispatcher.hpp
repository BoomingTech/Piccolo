#ifndef LF_POOL_DISPATCHER
#define LF_POOL_DISPATCHER

#include <tuple>
#include <cassert>
#include <cstddef>
#include <lfpAlloc/Pool.hpp>

#ifndef LFP_ALLOCATIONS_PER_CHUNK
#define LFP_ALLOCATIONS_PER_CHUNK 64 * 100
#endif

namespace lfpAlloc {
namespace detail {

template <std::size_t Num, uint16_t... Ts>
struct Pools : Pools<Num - 1, alignof(std::max_align_t) * Num, Ts...> {};

template <uint16_t... Size>
struct Pools<0, Size...> {
    using type = std::tuple<Pool<Size, LFP_ALLOCATIONS_PER_CHUNK>...>;
};
}

template <std::size_t NumPools>
class PoolDispatcher {
public:
    void* allocate(std::size_t size) { return dispatchAllocate<0>(size); }

    void deallocate(void* p, std::size_t size) noexcept {
        dispatchDeallocate<0>(p, size);
    }

private:
    thread_local static typename detail::Pools<NumPools>::type pools_;
    static_assert(NumPools > 0, "Invalid number of pools");

    template <std::size_t Index>
        typename std::enable_if <
        Index<NumPools, void*>::type
        dispatchAllocate(std::size_t const& requestSize) {
        if (requestSize <= std::get<Index>(pools_).CellSize) {
            return std::get<Index>(pools_).allocate();
        } else {
            return dispatchAllocate<Index + 1>(requestSize);
        }
    }

    template <std::size_t Index>
    typename std::enable_if<!(Index < NumPools), void*>::type
    dispatchAllocate(std::size_t const&) {
        assert(false && "Invalid allocation size.");
        return nullptr;
    }

    template <std::size_t Index>
        typename std::enable_if <
        Index<NumPools>::type
        dispatchDeallocate(void* p, std::size_t const& requestSize) noexcept {
        if (requestSize <= std::get<Index>(pools_).CellSize) {
            std::get<Index>(pools_).deallocate(p);
        } else {
            dispatchDeallocate<Index + 1>(p, requestSize);
        }
    }

    template <std::size_t Index>
    typename std::enable_if<!(Index < NumPools)>::type
    dispatchDeallocate(void*, std::size_t const&) noexcept {
        assert(false && "Invalid deallocation size.");
    }
};

template <std::size_t NumPools>
thread_local typename detail::Pools<NumPools>::type
    PoolDispatcher<NumPools>::pools_;
}

#endif
