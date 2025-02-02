#include <iostream>
#include <algorithm>

#include "HeadlessUtils.h"
#include "BiquadFilter.h"
#include "QuadFilterUnit.h"
#include "MemoryPool.h"

#include "catch2/catch2.hpp"

inline size_t align_diff(const void *ptr, std::uintptr_t alignment) noexcept
{
    auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
    return (iptr % alignment);
}

TEST_CASE("Biquad Aligned", "[infra]")
{
    SECTION("Is it aligned?")
    {
        std::vector<BiquadFilter *> pointers;
        for (int i = 0; i < 5000; ++i)
        {
            auto *f = new BiquadFilter();
            REQUIRE(align_diff(f, 16) == 0);
            pointers.push_back(f);
            if (rand() % 100 > 70 && !pointers.empty())
            {
                for (auto *d : pointers)
                    delete d;
                pointers.clear();
            }
        }
        if (pointers.empty())
            for (auto *d : pointers)
                delete d;
    }
}

TEST_CASE("QFU is Aligned", "[infra]")
{
    SECTION("Single QFU")
    {
        std::vector<QuadFilterUnitState *> pointers;
        for (int i = 0; i < 5000; ++i)
        {
            auto *f = new QuadFilterUnitState();
            REQUIRE(align_diff(f, 16) == 0);
            pointers.push_back(f);
            if (rand() % 100 > 70 && !pointers.empty())
            {
                for (auto *d : pointers)
                    delete d;
                pointers.clear();
            }
        }
        if (pointers.empty())
            for (auto *d : pointers)
                delete d;
    }

    SECTION("QFU Array")
    {
        int nqfus = 5;
        std::vector<QuadFilterUnitState *> pointers;
        for (int i = 0; i < 5000; ++i)
        {
            auto *f = new QuadFilterUnitState[nqfus]();
            for (int j = 0; j < nqfus; ++j)
            {
                auto *q = &f[j];
                REQUIRE(align_diff(q, 16) == 0);
                if (j >= 1)
                {
                    auto *qp = &f[j - 1];
                    REQUIRE((size_t)q - (size_t)qp == sizeof(QuadFilterUnitState));
                }
            }
            delete[] f;
        }
    }
}

// A is just a test index to separate the calsses
template <int A> struct CountAlloc
{
    CountAlloc()
    {
        alloc++;
        ct++;
    }
    ~CountAlloc() { ct--; }
    static int ct, alloc;
};
template <int A> int CountAlloc<A>::ct{0};
template <int A> int CountAlloc<A>::alloc{0};

TEST_CASE("Memory Pool Works", "[infra]")
{
    SECTION("Lots of random gets and returns")
    {
        {
            auto pool = std::make_unique<Surge::Memory::MemoryPool<CountAlloc<1>, 32, 4, 500>>();
            std::deque<CountAlloc<1> *> tmp;
            for (int i = 0; i < 100; ++i)
            {
                tmp.push_back(pool->getItem());
            }

            for (int i = 0; i < 100; ++i)
            {
                auto q = tmp.front();
                tmp.pop_front();
                pool->returnItem(q);
            }
        }
        REQUIRE(CountAlloc<1>::alloc == 100);
        REQUIRE(CountAlloc<1>::ct == 0);
    }

    SECTION("ReSize up")
    {
        {
            auto pool = std::make_unique<Surge::Memory::MemoryPool<CountAlloc<2>, 32, 4, 500>>();
            pool->setupPoolToSize(180);
        }
        REQUIRE(CountAlloc<2>::alloc == 180);
        REQUIRE(CountAlloc<2>::ct == 0);
    }

    SECTION("ReSize up and ReAlloc down")
    {
        {
            auto pool = std::make_unique<Surge::Memory::MemoryPool<CountAlloc<3>, 32, 4, 500>>();
            pool->setupPoolToSize(160);
            REQUIRE(CountAlloc<3>::ct == 160);
            pool->returnToPreAllocSize();
            REQUIRE(CountAlloc<3>::ct == 32);
        }
        REQUIRE(CountAlloc<3>::alloc == 160);
        REQUIRE(CountAlloc<3>::ct == 0);
    }
}