#ifndef SKYMESH_TEST_UTILS_H
#define SKYMESH_TEST_UTILS_H

#include <utility>

// Provide compatibility layer for index sequence
namespace testing {
namespace internal {
    template<std::size_t... Ints>
    using IndexSequence = std::index_sequence<Ints...>;

    template<std::size_t N>
    using MakeIndexSequence = std::make_index_sequence<N>;

    // Additional compatibility helpers
    template<typename... T>
    using IndexSequenceFor = std::index_sequence_for<T...>;

    // Legacy macro compatibility
    #ifndef GTEST_DISALLOW_COPY_AND_ASSIGN_
    #define GTEST_DISALLOW_COPY_AND_ASSIGN_(type) \
        type(const type&) = delete; \
        type& operator=(const type&) = delete
    #endif
}
}

#endif // SKYMESH_TEST_UTILS_H

