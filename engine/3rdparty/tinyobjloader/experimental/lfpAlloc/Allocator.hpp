#ifndef LF_POOL_ALLOCATOR
#define LF_POOL_ALLOCATOR

#include <memory>
#include <thread>
#include <lfpAlloc/PoolDispatcher.hpp>

namespace lfpAlloc {
template <typename T, std::size_t NumPools = 70>
class lfpAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = T const&;

    template <typename U>
    struct rebind {
        typedef lfpAllocator<U, NumPools> other;
    };

    lfpAllocator() {}

    template <typename U>
    lfpAllocator(lfpAllocator<U, NumPools>&&) noexcept {}

    template <typename U>
    lfpAllocator(const lfpAllocator<U, NumPools>&) noexcept {}

    T* allocate(std::size_t count) {
        if (sizeof(T) * count <=
            alignof(std::max_align_t) * NumPools - sizeof(void*)) {
            return reinterpret_cast<T*>(
                dispatcher_.allocate(sizeof(T) * count));
        } else {
            return new T[count];
        }
    }

    void deallocate(T* p, std::size_t count) noexcept {
        if (sizeof(T) * count <=
            alignof(std::max_align_t) * NumPools - sizeof(void*)) {
            dispatcher_.deallocate(p, sizeof(T) * count);
        } else {
            delete[] p;
        }
    }

    // Should not be required, but allocator_traits is not complete in
    // gcc 4.9.1
    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new (p) U(std::forward<Args>(args)...);
    }

    template <typename Ty, typename U, std::size_t N, std::size_t M>
    friend bool operator==(const lfpAllocator<Ty, N>&,
                           const lfpAllocator<U, M>&) noexcept;

    template <typename U, std::size_t M>
    friend class lfpAllocator;

private:
    static PoolDispatcher<NumPools> dispatcher_;
};

template <typename T, std::size_t N>
PoolDispatcher<N> lfpAllocator<T, N>::dispatcher_;

template <typename T, typename U, std::size_t N, std::size_t M>
inline bool operator==(const lfpAllocator<T, N>&,
                       const lfpAllocator<U, M>&) noexcept {
    return N == M;
}

template <typename T, typename U, std::size_t N, std::size_t M>
inline bool operator!=(const lfpAllocator<T, N>& left,
                       const lfpAllocator<U, M>& right) noexcept {
    return !(left == right);
}
}

#endif
