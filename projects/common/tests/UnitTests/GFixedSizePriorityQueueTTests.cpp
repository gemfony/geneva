/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GFixedSizePriorityQueueT.hpp"
#include "common/GTypeTraitsT.hpp"

using namespace Gem::Common;

// ---------------------------------------------------------------------------
// Minimal item type. The priority queue requires items to satisfy
// has_gemfony_common_interface (so load/clone helpers compile) and to expose
// clone<U>(), load(shared_ptr<T>) and compare(T, expectation, double).

namespace {

class TestItem : public gemfony_common_interface_indicator {
public:
    double value{0.0};
    TestItem() = default;
    explicit TestItem(double v) : value(v) {}

    template <typename U = TestItem>
    [[nodiscard]] std::shared_ptr<U> clone() const {
        return std::make_shared<U>(value);
    }

    void load(std::shared_ptr<TestItem> const &cp) {
        if(cp) {
            value = cp->value;
        }
    }

    void compare(TestItem const &other, expectation e, [[maybe_unused]] double limit) const {
        bool const eq = (value == other.value);
        if(e == expectation::INEQUALITY ? eq : not eq) {
            throw g_expectation_violation("TestItem compare mismatch");
        }
    }
};

// Concrete subclass — fills in the pure-virtual surface (isValid /
// evaluation / clone_) and inherits load_ / compare_ from the base.

class TestPQ : public GFixedSizePriorityQueueT<TestItem> {
public:
    TestPQ() = default;
    explicit TestPQ(std::size_t maxSize)
      : GFixedSizePriorityQueueT<TestItem>(maxSize) {}
    TestPQ(std::size_t maxSize, sortOrder so)
      : GFixedSizePriorityQueueT<TestItem>(maxSize, so) {}

protected:
    [[nodiscard]] bool   isValid   ([[maybe_unused]] std::shared_ptr<TestItem> const &p) const override { return true; }
    [[nodiscard]] double evaluation(std::shared_ptr<TestItem> const &p) const override { return p->value; }

private:
    [[nodiscard]] TestPQ *clone_() const override {
        return new TestPQ(*this);
    }
};

std::shared_ptr<TestItem> make_item(double v) {
    return std::make_shared<TestItem>(v);
}

} // namespace

// ---------------------------------------------------------------------------
// Empty queue accessors

TEST_CASE("GFixedSizePriorityQueueT: empty queue reports empty/size 0 and throws on accessors",
          "[common][priority-queue]") {
    TestPQ pq(5);
    CHECK(pq.empty());
    CHECK(pq.empty());
    CHECK_THROWS_AS(pq.best(),  geneva_exception);
    CHECK_THROWS_AS(pq.worst(), geneva_exception);
    CHECK_THROWS_AS(pq.pop(),   geneva_exception);
}

// ---------------------------------------------------------------------------
// Insertion + LOWERISBETTER ordering (default)

TEST_CASE("GFixedSizePriorityQueueT: LOWERISBETTER orders best=min, worst=max",
          "[common][priority-queue]") {
    TestPQ pq(10);   // default sortOrder is LOWERISBETTER
    pq.add(make_item(5.0), false);
    pq.add(make_item(1.0), false);
    pq.add(make_item(3.0), false);

    REQUIRE(pq.size() == 3);
    CHECK(pq.best()->value  == 1.0);
    CHECK(pq.worst()->value == 5.0);
}

TEST_CASE("GFixedSizePriorityQueueT: HIGHERISBETTER inverts best/worst",
          "[common][priority-queue]") {
    TestPQ pq(10, sortOrder::HIGHERISBETTER);
    pq.add(make_item(5.0), false);
    pq.add(make_item(1.0), false);
    pq.add(make_item(3.0), false);

    REQUIRE(pq.size() == 3);
    CHECK(pq.best()->value  == 5.0);
    CHECK(pq.worst()->value == 1.0);
}

// ---------------------------------------------------------------------------
// Size cap: when the queue is full, only items that are strictly better than
// the current worst replace it.

TEST_CASE("GFixedSizePriorityQueueT: full queue rejects items worse than current worst",
          "[common][priority-queue]") {
    TestPQ pq(3); // LOWERISBETTER
    pq.add(make_item(1.0), false);
    pq.add(make_item(2.0), false);
    pq.add(make_item(3.0), false);
    REQUIRE(pq.size() == 3);

    pq.add(make_item(5.0), false);  // worse than worst (3.0) → rejected (or trimmed)
    REQUIRE(pq.size() == 3);
    CHECK(pq.worst()->value == 3.0);

    pq.add(make_item(0.5), false);  // better than best → kept; worst now drops
    REQUIRE(pq.size() == 3);
    CHECK(pq.best()->value  == 0.5);
    CHECK(pq.worst()->value <= 3.0);
}

// ---------------------------------------------------------------------------
// pop semantics

TEST_CASE("GFixedSizePriorityQueueT: pop returns best item and shrinks the queue",
          "[common][priority-queue]") {
    TestPQ pq(5);
    pq.add(make_item(7.0), false);
    pq.add(make_item(3.0), false);
    pq.add(make_item(5.0), false);

    auto top = pq.pop();
    REQUIRE(top);
    CHECK(top->value == 3.0);
    CHECK(pq.size() == 2);
}

// ---------------------------------------------------------------------------
// Bulk add via iterators + add(vector, ..., replace).

TEST_CASE("GFixedSizePriorityQueueT: bulk add (vector, replace=true) starts a fresh queue",
          "[common][priority-queue]") {
    TestPQ pq(5);
    pq.add(make_item(99.0), false);   // pre-existing content

    std::vector<std::shared_ptr<TestItem>> const items{
        make_item(4.0), make_item(1.0), make_item(7.0)
    };
    pq.add(items, /*do_clone*/ false, /*replace*/ true);

    REQUIRE(pq.size() == 3);
    CHECK(pq.best()->value  == 1.0);
    CHECK(pq.worst()->value == 7.0);
}

TEST_CASE("GFixedSizePriorityQueueT: bulk add (replace=false) merges with existing items",
          "[common][priority-queue]") {
    TestPQ pq(5);
    pq.add(make_item(10.0), false);

    std::vector<std::shared_ptr<TestItem>> const items{
        make_item(4.0), make_item(20.0)
    };
    pq.add(items, false, /*replace*/ false);

    // 10, 4, 20 → best=4, worst=20 (LOWERISBETTER)
    REQUIRE(pq.size() == 3);
    CHECK(pq.best()->value  == 4.0);
    CHECK(pq.worst()->value == 20.0);
}

TEST_CASE("GFixedSizePriorityQueueT: bulk add skips null shared_ptrs",
          "[common][priority-queue]") {
    TestPQ pq(5);
    std::vector<std::shared_ptr<TestItem>> items;
    items.push_back(make_item(3.0));
    items.push_back(nullptr);
    items.push_back(make_item(1.0));

    pq.add(items, false, true);
    CHECK(pq.size() == 2);
}

// ---------------------------------------------------------------------------
// add(..., do_clone=true): each inserted shared_ptr should be a *clone*, not
// the original pointee. The original pointer and the stored pointer must not
// alias.

TEST_CASE("GFixedSizePriorityQueueT: do_clone=true inserts a copy, not the original pointer",
          "[common][priority-queue]") {
    TestPQ pq(5);
    auto src = make_item(2.5);
    pq.add(src, /*do_clone*/ true);

    REQUIRE(pq.size() == 1);
    auto stored = pq.best();
    CHECK(stored.get() != src.get());      // distinct shared_ptrs
    CHECK(stored->value == src->value);    // same logical value
}

// ---------------------------------------------------------------------------
// Duplicate removal: adding the same shared_ptr twice (do_clone=false) must
// not produce duplicates.

TEST_CASE("GFixedSizePriorityQueueT: same shared_ptr is not duplicated when do_clone=false",
          "[common][priority-queue]") {
    TestPQ pq(5);
    auto src = make_item(4.0);
    pq.add(src, false);
    pq.add(src, false);
    CHECK(pq.size() == 1);
}

// ---------------------------------------------------------------------------
// Unlimited size: maxSize_=0

TEST_CASE("GFixedSizePriorityQueueT: maxSize=0 means unlimited",
          "[common][priority-queue]") {
    TestPQ pq(0);
    for(int i = 0; i < 50; ++i) {
        pq.add(make_item(static_cast<double>(i)), false);
    }
    CHECK(pq.size() == 50);
    CHECK(pq.best()->value  == 0.0);
    CHECK(pq.worst()->value == 49.0);
}

// ---------------------------------------------------------------------------
// setMaxSize trims the queue when the new size is smaller.

TEST_CASE("GFixedSizePriorityQueueT: setMaxSize shrinks the queue to the new bound",
          "[common][priority-queue]") {
    TestPQ pq(0);
    for(int i = 0; i < 10; ++i) {
        pq.add(make_item(static_cast<double>(i)), false);
    }
    REQUIRE(pq.size() == 10);

    pq.setMaxSize(4);
    CHECK(pq.size() == 4);
    CHECK(pq.getMaxSize() == 4);

    // The trimmed survivors must be the four best (lowest) values.
    CHECK(pq.best()->value  == 0.0);
    CHECK(pq.worst()->value == 3.0);
}

// ---------------------------------------------------------------------------
// clear, toVector

TEST_CASE("GFixedSizePriorityQueueT: clear drops every item",
          "[common][priority-queue]") {
    TestPQ pq(3);
    pq.add(make_item(1.0), false);
    pq.add(make_item(2.0), false);
    pq.clear();
    CHECK(pq.empty());
}

TEST_CASE("GFixedSizePriorityQueueT: toVector returns items in priority order",
          "[common][priority-queue]") {
    TestPQ pq(5);
    pq.add(make_item(7.0), false);
    pq.add(make_item(1.0), false);
    pq.add(make_item(3.0), false);

    auto v = pq.toVector();
    REQUIRE(v.size() == 3);
    CHECK(v[0]->value == 1.0);     // best first under LOWERISBETTER
    CHECK(v[2]->value == 7.0);
}

// ---------------------------------------------------------------------------
// Copy / move semantics

TEST_CASE("GFixedSizePriorityQueueT: copy ctor produces an independent queue",
          "[common][priority-queue]") {
    TestPQ src(5);
    src.add(make_item(2.0), false);
    src.add(make_item(8.0), false);

    TestPQ cp(src);
    CHECK(cp.size() == 2);
    CHECK(cp.best()->value  == 2.0);
    CHECK(cp.worst()->value == 8.0);

    // Mutating the copy must not affect the source.
    cp.clear();
    CHECK(cp.empty());
    CHECK(src.size() == 2);
}

TEST_CASE("GFixedSizePriorityQueueT: move ctor resets the source to default state",
          "[common][priority-queue]") {
    TestPQ src(7, sortOrder::HIGHERISBETTER);
    src.add(make_item(1.0), false);
    src.add(make_item(9.0), false);

    TestPQ const dst(std::move(src));
    CHECK(dst.size() == 2);
    CHECK(dst.getMaxSize() == 7);
    CHECK(dst.getSortOrder() == sortOrder::HIGHERISBETTER);

    // Moved-from object reset to defaults.
    CHECK(src.empty());
    CHECK(src.getMaxSize()   == GFSPQ_DEF_MAX_SIZE);
    CHECK(src.getSortOrder() == GFSPQ_DEF_SORT_ORDER);
}

// ---------------------------------------------------------------------------
// Sort-order setter / getter

TEST_CASE("GFixedSizePriorityQueueT: setSortOrder + getSortOrder round-trip",
          "[common][priority-queue]") {
    TestPQ pq(5);
    CHECK(pq.getSortOrder() == sortOrder::LOWERISBETTER);
    pq.setSortOrder(sortOrder::HIGHERISBETTER);
    CHECK(pq.getSortOrder() == sortOrder::HIGHERISBETTER);
}
