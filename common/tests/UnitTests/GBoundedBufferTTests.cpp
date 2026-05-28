/**
 * @file GBoundedBufferTTests.cpp
 *
 * Tests of the GBoundedBufferT class (moved from include/common/tests/).
 */

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

// Standard headers go here
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <vector>

// Catch2 headers go here
#include <catch2/catch_test_macros.hpp>

// Geneva headers go here
#include "common/GBoundedBufferT.hpp"

using Gem::Common::GBoundedBufferT;
using Gem::Common::DEFAULTBUFFERSIZE;

/******************************************************************************/

namespace {

struct copy_only_struct {
    copy_only_struct() = delete;
    explicit copy_only_struct(std::size_t secret) : secret_(secret) {}
    copy_only_struct(const copy_only_struct &) = default;
    copy_only_struct(copy_only_struct &&) = delete;
    copy_only_struct &operator=(copy_only_struct &) = default;
    copy_only_struct &operator=(copy_only_struct &&) = delete;
    std::size_t getSecret() const { return secret_; }
private:
    std::size_t secret_ = 0;
};

struct move_only_struct {
    move_only_struct() = delete;
    explicit move_only_struct(std::size_t secret) : secret_(secret) {}
    move_only_struct(const move_only_struct &) = delete;
    move_only_struct(move_only_struct &&cp) { secret_ = cp.secret_; cp.secret_ = 0; }
    move_only_struct &operator=(move_only_struct &) = delete;
    move_only_struct &operator=(move_only_struct &&cp) {
        secret_ = cp.secret_; cp.secret_ = 0; return *this;
    }
    std::size_t getSecret() const { return secret_; }
private:
    std::size_t secret_ = 0;
};

struct copy_move_struct {
    copy_move_struct() = delete;
    explicit copy_move_struct(std::size_t secret) : secret_(secret) {}
    copy_move_struct(const copy_move_struct &cp)
        : secret_(cp.secret_), copy_move_history_(cp.copy_move_history_) {
        copy_move_history_.push_back(m_copied_);
    }
    copy_move_struct(copy_move_struct &&cp) {
        secret_ = cp.secret_; cp.secret_ = 0;
        copy_move_history_ = std::move(cp.copy_move_history_);
        copy_move_history_.push_back(m_moved_);
    }
    copy_move_struct &operator=(copy_move_struct &cp) {
        secret_ = cp.secret_;
        copy_move_history_ = cp.copy_move_history_;
        copy_move_history_.push_back(m_copied_);
        return *this;
    }
    copy_move_struct &operator=(copy_move_struct &&cp) {
        secret_ = cp.secret_; cp.secret_ = 0;
        copy_move_history_ = std::move(cp.copy_move_history_);
        copy_move_history_.push_back(m_moved_);
        return *this;
    }
    bool struct_was_copied() const {
        return std::find(copy_move_history_.begin(), copy_move_history_.end(), m_copied_)
               != copy_move_history_.end();
    }
    bool struct_was_moved() const {
        return std::find(copy_move_history_.begin(), copy_move_history_.end(), m_moved_)
               != copy_move_history_.end();
    }
    bool struct_was_copied_or_moved() const { return !copy_move_history_.empty(); }
    std::size_t getSecret() const { return secret_; }
private:
    const std::uint32_t m_copied_ = 0, m_moved_ = 1;
    std::size_t secret_ = 0;
    std::vector<std::uint32_t> copy_move_history_;
};

} // namespace

/******************************************************************************/

TEST_CASE("GBoundedBuffer no_failure_expected", "[common][standard]") {

    // Check construction with different sizes and value types
    CHECK_NOTHROW((GBoundedBufferT<copy_only_struct>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 0>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 10>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 20>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 30>()));

    CHECK_NOTHROW((GBoundedBufferT<move_only_struct>()));
    CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 0>()));
    CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 10>()));
    CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 20>()));
    CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 30>()));

    CHECK_NOTHROW((GBoundedBufferT<copy_move_struct>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 0>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 10>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 20>()));
    CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 30>()));

    // Check boundaries after construction
    {
        GBoundedBufferT<copy_only_struct> gbt1;
        CHECK(gbt1.getCapacity() == DEFAULTBUFFERSIZE);
        CHECK(gbt1.isBounded());
        CHECK(gbt1.empty());
        CHECK(gbt1.size() == 0);
        CHECK(!gbt1.isNotEmpty());

        GBoundedBufferT<copy_only_struct, 0> gbt2;
        CHECK(gbt2.getCapacity() == 0);
        CHECK(!gbt2.isBounded());
        CHECK(gbt2.empty());

        GBoundedBufferT<copy_only_struct, 10> gbt3;
        CHECK(gbt3.getCapacity() == 10);
        CHECK(gbt3.isBounded());

        GBoundedBufferT<move_only_struct> gbt6;
        CHECK(gbt6.getCapacity() == DEFAULTBUFFERSIZE);
        CHECK(gbt6.isBounded());
        CHECK(gbt6.empty());

        GBoundedBufferT<move_only_struct, 0> gbt7;
        CHECK(gbt7.getCapacity() == 0);
        CHECK(!gbt7.isBounded());

        GBoundedBufferT<copy_move_struct> gpbt11;
        CHECK(gpbt11.getCapacity() == DEFAULTBUFFERSIZE);
        CHECK(gpbt11.isBounded());
        CHECK(gpbt11.empty());

        GBoundedBufferT<copy_move_struct, 0> gpbt12;
        CHECK(gpbt12.getCapacity() == 0);
        CHECK(!gpbt12.isBounded());
    }

    // try_push_* / try_pop_* — unbounded queues
    {
        GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;
        bool push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(i);
            CHECK(c.getSecret() == i);
            CHECK_NOTHROW(push_succeeded = gbt_co_unbounded.try_push_copy(c));
            CHECK(!gbt_co_unbounded.empty());
            CHECK(push_succeeded);
            CHECK(gbt_co_unbounded.size() == i + 1);
            CHECK(c.getSecret() == i);
            push_succeeded = false;
        }
        bool pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_co_unbounded.try_pop_copy(c));
            CHECK(pop_succeeded);
            CHECK(gbt_co_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
            CHECK(c.getSecret() == i);
            pop_succeeded = false;
        }
        CHECK(gbt_co_unbounded.size() == 0);
        CHECK(gbt_co_unbounded.empty());

        GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;
        push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(i);
            CHECK_NOTHROW(push_succeeded = gbt_mo_unbounded.try_push_move(std::move(m)));
            CHECK(m.getSecret() == 0);
            CHECK(push_succeeded);
            CHECK(gbt_mo_unbounded.size() == i + 1);
            push_succeeded = false;
        }
        pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_mo_unbounded.try_pop_move(m));
            CHECK(pop_succeeded);
            CHECK(gbt_mo_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
            CHECK(m.getSecret() == i);
            pop_succeeded = false;
        }
        CHECK(gbt_mo_unbounded.size() == 0);
        CHECK(gbt_mo_unbounded.empty());

        GBoundedBufferT<copy_move_struct, 0> gbt_copy_only_cms_unbounded;
        push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_move_struct c(i);
            CHECK_NOTHROW(push_succeeded = gbt_copy_only_cms_unbounded.try_push_copy(c));
            CHECK(push_succeeded);
            CHECK(gbt_copy_only_cms_unbounded.size() == i + 1);
            push_succeeded = false;
        }
        pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_move_struct c(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_copy_only_cms_unbounded.try_pop_copy(c));
            CHECK(pop_succeeded);
            CHECK(c.struct_was_copied());
            CHECK(!c.struct_was_moved());
            CHECK(c.struct_was_copied_or_moved());
            pop_succeeded = false;
        }
        CHECK(gbt_copy_only_cms_unbounded.size() == 0);

        GBoundedBufferT<copy_move_struct, 0> gbt_move_only_cms_unbounded;
        push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_move_struct m(i);
            CHECK_NOTHROW(push_succeeded = gbt_move_only_cms_unbounded.try_push_move(std::move(m)));
            CHECK(push_succeeded);
            push_succeeded = false;
        }
        pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_move_struct m(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_move_only_cms_unbounded.try_pop_move(m));
            CHECK(pop_succeeded);
            CHECK(!m.struct_was_copied());
            CHECK(m.struct_was_moved());
            CHECK(m.struct_was_copied_or_moved());
            pop_succeeded = false;
        }
        CHECK(gbt_move_only_cms_unbounded.size() == 0);
    }

    // try_push_* / try_pop_* — bounded queues
    {
        GBoundedBufferT<copy_only_struct> gbt_co_bounded;
        CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);
        bool push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(i);
            CHECK_NOTHROW(push_succeeded = gbt_co_bounded.try_push_copy(c));
            CHECK(!gbt_co_bounded.empty());
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(push_succeeded);
                CHECK(gbt_co_bounded.size() == i + 1);
            } else {
                CHECK(!push_succeeded);
                CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE);
            }
            CHECK(c.getSecret() == i);
            push_succeeded = false;
        }
        bool pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_co_bounded.try_pop_copy(c));
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(pop_succeeded);
                CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                CHECK(c.getSecret() == i);
            } else {
                CHECK(!pop_succeeded);
                CHECK(gbt_co_bounded.size() == 0);
                CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
            }
            pop_succeeded = false;
        }

        GBoundedBufferT<move_only_struct> gbt_mo_bounded;
        CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);
        push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(i);
            CHECK_NOTHROW(push_succeeded = gbt_mo_bounded.try_push_move(std::move(m)));
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(push_succeeded);
                CHECK(m.getSecret() == 0);
                CHECK(gbt_mo_bounded.size() == i + 1);
            } else {
                CHECK(!push_succeeded);
                CHECK(m.getSecret() == i);
                CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE);
            }
            push_succeeded = false;
        }
        pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_mo_bounded.try_pop_move(m));
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(pop_succeeded);
                CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                CHECK(m.getSecret() == i);
            } else {
                CHECK(!pop_succeeded);
                CHECK(gbt_mo_bounded.size() == 0);
                CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
            }
            pop_succeeded = false;
        }
        CHECK(gbt_mo_bounded.size() == 0);
        CHECK(gbt_mo_bounded.empty());
    }

    // push_and_block_* / pop_and_block_* — unbounded queues
    {
        GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(i);
            CHECK_NOTHROW(gbt_co_unbounded.push_and_block_copy(c));
            CHECK(!gbt_co_unbounded.empty());
            CHECK(gbt_co_unbounded.size() == i + 1);
            CHECK(c.getSecret() == i);
        }
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(gbt_co_unbounded.pop_and_block_copy(c));
            CHECK(gbt_co_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
            CHECK(c.getSecret() == i);
        }
        CHECK(gbt_co_unbounded.size() == 0);
        CHECK(gbt_co_unbounded.empty());

        GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(i);
            CHECK_NOTHROW(gbt_mo_unbounded.push_and_block_move(std::move(m)));
            CHECK(m.getSecret() == 0);
            CHECK(!gbt_mo_unbounded.empty());
            CHECK(gbt_mo_unbounded.size() == i + 1);
        }
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(gbt_mo_unbounded.pop_and_block_move(m));
            CHECK(gbt_mo_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
            CHECK(m.getSecret() == i);
        }
        CHECK(gbt_mo_unbounded.size() == 0);
        CHECK(gbt_mo_unbounded.empty());
    }

    // push_and_block_* / pop_and_block_* — bounded queues
    {
        GBoundedBufferT<copy_only_struct> gbt_co_bounded;
        CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);
        for(std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(i);
            CHECK_NOTHROW(gbt_co_bounded.push_and_block_copy(c));
            CHECK(!gbt_co_bounded.empty());
            CHECK(gbt_co_bounded.size() == i + 1);
            CHECK(c.getSecret() == i);
        }
        for(std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(gbt_co_bounded.pop_and_block_copy(c));
            CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
            CHECK(c.getSecret() == i);
        }

        GBoundedBufferT<move_only_struct> gbt_mo_bounded;
        CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);
        for(std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(i);
            CHECK_NOTHROW(gbt_mo_bounded.push_and_block_move(std::move(m)));
            CHECK(!gbt_mo_bounded.empty());
            CHECK(m.getSecret() == 0);
            CHECK(gbt_mo_bounded.size() == i + 1);
        }
        for(std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(gbt_mo_bounded.pop_and_block_move(m));
            CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
            CHECK(m.getSecret() == i);
        }
        CHECK(gbt_mo_bounded.size() == 0);
        CHECK(gbt_mo_bounded.empty());
    }

    // push_and_wait_* / pop_and_wait_* — unbounded queues
    {
        std::chrono::duration<double> timeout(std::chrono::microseconds(1));

        GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;
        bool push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(i);
            CHECK_NOTHROW(push_succeeded = gbt_co_unbounded.push_and_wait_copy(c, timeout));
            CHECK(push_succeeded);
            CHECK(gbt_co_unbounded.size() == i + 1);
            push_succeeded = false;
        }
        bool pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_co_unbounded.pop_and_wait_copy(c, timeout));
            CHECK(pop_succeeded);
            CHECK(gbt_co_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
            CHECK(c.getSecret() == i);
            pop_succeeded = false;
        }
        CHECK(gbt_co_unbounded.size() == 0);
        CHECK(gbt_co_unbounded.empty());

        GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;
        push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(i);
            CHECK_NOTHROW(
                push_succeeded = gbt_mo_unbounded.push_and_wait_move(std::move(m), timeout)
            );
            CHECK(m.getSecret() == 0);
            CHECK(push_succeeded);
            CHECK(gbt_mo_unbounded.size() == i + 1);
            push_succeeded = false;
        }
        pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_mo_unbounded.pop_and_wait_move(m, timeout));
            CHECK(pop_succeeded);
            CHECK(gbt_mo_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
            CHECK(m.getSecret() == i);
            pop_succeeded = false;
        }
        CHECK(gbt_mo_unbounded.size() == 0);
        CHECK(gbt_mo_unbounded.empty());
    }

    // push_and_wait_* / pop_and_wait_* — bounded queues
    {
        std::chrono::duration<double> timeout(std::chrono::microseconds(1));

        GBoundedBufferT<copy_only_struct> gbt_co_bounded;
        CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);
        bool push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(i);
            CHECK_NOTHROW(push_succeeded = gbt_co_bounded.push_and_wait_copy(c, timeout));
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(push_succeeded);
                CHECK(gbt_co_bounded.size() == i + 1);
            } else {
                CHECK(!push_succeeded);
                CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE);
            }
            CHECK(c.getSecret() == i);
            push_succeeded = false;
        }
        bool pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            copy_only_struct c(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_co_bounded.pop_and_wait_copy(c, timeout));
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(pop_succeeded);
                CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                CHECK(c.getSecret() == i);
            } else {
                CHECK(!pop_succeeded);
                CHECK(gbt_co_bounded.size() == 0);
                CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
            }
            pop_succeeded = false;
        }

        GBoundedBufferT<move_only_struct> gbt_mo_bounded;
        CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);
        push_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(i);
            CHECK_NOTHROW(
                push_succeeded = gbt_mo_bounded.push_and_wait_move(std::move(m), timeout)
            );
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(push_succeeded);
                CHECK(m.getSecret() == 0);
                CHECK(gbt_mo_bounded.size() == i + 1);
            } else {
                CHECK(!push_succeeded);
                CHECK(m.getSecret() == i);
                CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE);
            }
            push_succeeded = false;
        }
        pop_succeeded = false;
        for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
            move_only_struct m(3 * DEFAULTBUFFERSIZE);
            CHECK_NOTHROW(pop_succeeded = gbt_mo_bounded.pop_and_wait_move(m, timeout));
            if(i < DEFAULTBUFFERSIZE) {
                CHECK(pop_succeeded);
                CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                CHECK(m.getSecret() == i);
            } else {
                CHECK(!pop_succeeded);
                CHECK(gbt_mo_bounded.size() == 0);
                CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
            }
            pop_succeeded = false;
        }
        CHECK(gbt_mo_bounded.size() == 0);
        CHECK(gbt_mo_bounded.empty());
    }
}

/******************************************************************************/

TEST_CASE("GBoundedBuffer failures_expected", "[common][standard][failures-expected]") {
    // No failure cases currently defined for GBoundedBufferT
}
