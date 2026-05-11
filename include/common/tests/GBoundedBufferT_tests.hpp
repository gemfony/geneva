/**
 * @file GBoundedBufferT_tests.hpp
 *
 * Tests of the GBoundedBufferT class
 */

// Standard headers go here
#include <algorithm>
#include <vector>

// Catch2 headers go here
#include <catch2/catch_test_macros.hpp>

// Geneva headers go here
#include "common/GBoundedBufferT.hpp"

namespace Gem {
namespace Common {
namespace Tests {

/**
 * TODO:
 * - Add copy_move_struct to test whether the correct actions are taken
 *   o Must know whether it was copied or moved, store "history"
 * - Add tests for std::shared_ptr and std::unique_otr with copy- and move, where applicable
 * - Add fuzz-testing with 10000 items submitted/retreived concurrently using different functions
 */

/******************************************************************************/
/**
 * A simple struct meant to test copying of work items.
 */
struct copy_only_struct {
public:
    copy_only_struct() = delete;
    copy_only_struct(std::size_t secret)
      : secret_(secret) { /* nothing */
    }
    copy_only_struct(const copy_only_struct &cp) = default;
    copy_only_struct(copy_only_struct &&) = delete;

    copy_only_struct &operator=(copy_only_struct &cp) = default;
    copy_only_struct &operator=(copy_only_struct &&cp) = delete;

    std::size_t getSecret() const {
        return secret_;
    }

private:
    std::size_t secret_ = 0;
};

/******************************************************************************/
/**
 * A simple struct meant to test moving of work items.
 */
struct move_only_struct {
public:
    move_only_struct() = delete;

    move_only_struct(std::size_t secret)
      : secret_(secret) { /* nothing */
    }
    move_only_struct(const move_only_struct &cp) = delete;
    move_only_struct(move_only_struct &&cp) {
        secret_ = cp.secret_;
        cp.secret_ = 0;
    }

    move_only_struct &operator=(move_only_struct &cp) = delete;
    move_only_struct &operator=(move_only_struct &&cp) {
        secret_ = cp.secret_;
        cp.secret_ = 0;

        return *this;
    }

    std::size_t getSecret() const {
        return secret_;
    }

private:
    std::size_t secret_ = 0;
};

/******************************************************************************/
/**
 * A simple struct meant to make sure the correct functions were called
 */
struct copy_move_struct {
public:
    copy_move_struct() = delete;

    copy_move_struct(std::size_t secret)
      : secret_(secret) { /* nothing */
    }
    copy_move_struct(const copy_move_struct &cp)
      : secret_(cp.secret_)
      , copy_move_history_(cp.copy_move_history_) {
        copy_move_history_.push_back(M_COPIED);
    }
    copy_move_struct(copy_move_struct &&cp) {
        secret_ = cp.secret_;
        cp.secret_ = 0;
        copy_move_history_ = std::move(cp.copy_move_history_);
        copy_move_history_.push_back(M_MOVED);
    }

    copy_move_struct &operator=(copy_move_struct &cp) {
        secret_ = cp.secret_;
        copy_move_history_ = cp.copy_move_history_;
        copy_move_history_.push_back(M_COPIED);

        return *this;
    }
    copy_move_struct &operator=(copy_move_struct &&cp) {
        secret_ = cp.secret_;
        cp.secret_ = 0;
        copy_move_history_ = std::move(cp.copy_move_history_);
        copy_move_history_.push_back(M_MOVED);

        return *this;
    }

    bool struct_was_copied() {
        if(std::find(copy_move_history_.begin(), copy_move_history_.end(), M_COPIED) !=
           copy_move_history_.end()) {
            return true;
        }
        return false;
    }

    bool struct_was_moved() {
        if(std::find(copy_move_history_.begin(), copy_move_history_.end(), M_MOVED) !=
           copy_move_history_.end()) {
            return true;
        }
        return false;
    }

    bool struct_was_copied_and_moved() { // This should not happen in the tests
        return (struct_was_copied() && struct_was_moved());
    }

    bool struct_was_copied_or_moved() {
        return !copy_move_history_.empty();
    }

    std::size_t getSecret() const {
        return secret_;
    }

private:
    const std::uint32_t M_COPIED = 0, M_MOVED = 1;

    std::size_t secret_ = 0;
    std::vector<std::uint32_t> copy_move_history_;
};

/******************************************************************************/
/**
 * Unit tests for the GBoundedBufferT class
 */
class GBoundedBufferT_tests {
public:
    /*************************************************************************/
    /**
	  * Test of features that are expected to work
	  */
    void no_failure_expected() {
        //----------------------------------------------------------------------

        { // Check construction with different sizes and value types
            CHECK_NOTHROW((GBoundedBufferT<copy_only_struct>()));    // DEFAULTBUFFERSIZE
            CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 0>())); // unbounded
            CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 10>()));
            CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 20>()));
            CHECK_NOTHROW((GBoundedBufferT<copy_only_struct, 30>()));

            CHECK_NOTHROW((GBoundedBufferT<move_only_struct>()));    // DEFAULTBUFFERSIZE
            CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 0>())); // unbounded
            CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 10>()));
            CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 20>()));
            CHECK_NOTHROW((GBoundedBufferT<move_only_struct, 30>()));

            CHECK_NOTHROW((GBoundedBufferT<copy_move_struct>()));    // DEFAULTBUFFERSIZE
            CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 0>())); // unbounded
            CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 10>()));
            CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 20>()));
            CHECK_NOTHROW((GBoundedBufferT<copy_move_struct, 30>()));
        }

        //----------------------------------------------------------------------

        {   // Check boundaries after construction
            //-------------------------------------------------------------------
            // copy_only_struct

            GBoundedBufferT<copy_only_struct> gbt1; // DEFAULTBUFFERSIZE
            CHECK(gbt1.getCapacity() == DEFAULTBUFFERSIZE);
            CHECK(gbt1.isBounded());
            CHECK(gbt1.empty());
            CHECK(gbt1.size() == 0);
            CHECK(!gbt1.isNotEmpty());

            GBoundedBufferT<copy_only_struct, 0> gbt2; // unbounded
            CHECK(gbt2.getCapacity() == 0);
            CHECK(!gbt2.isBounded());
            CHECK(gbt2.empty());
            CHECK(gbt2.size() == 0);
            CHECK(!gbt2.isNotEmpty());

            GBoundedBufferT<copy_only_struct, 10> gbt3;
            CHECK(gbt3.getCapacity() == 10);
            CHECK(gbt3.isBounded());
            CHECK(gbt3.empty());
            CHECK(gbt3.size() == 0);
            CHECK(!gbt3.isNotEmpty());

            GBoundedBufferT<copy_only_struct, 20> gbt4;
            CHECK(gbt4.getCapacity() == 20);
            CHECK(gbt4.isBounded());
            CHECK(gbt4.empty());
            CHECK(gbt4.size() == 0);
            CHECK(!gbt4.isNotEmpty());

            GBoundedBufferT<copy_only_struct, 30> gbt5;
            CHECK(gbt5.getCapacity() == 30);
            CHECK(gbt5.isBounded());
            CHECK(gbt5.empty());
            CHECK(gbt5.size() == 0);
            CHECK(!gbt5.isNotEmpty());

            //-------------------------------------------------------------------
            // move_only_struct

            GBoundedBufferT<move_only_struct> gbt6; // DEFAULTBUFFERSIZE
            CHECK(gbt6.getCapacity() == DEFAULTBUFFERSIZE);
            CHECK(gbt6.isBounded());
            CHECK(gbt6.empty());
            CHECK(gbt6.size() == 0);
            CHECK(!gbt6.isNotEmpty());

            GBoundedBufferT<move_only_struct, 0> gbt7; // unbounded
            CHECK(gbt7.getCapacity() == 0);
            CHECK(!gbt7.isBounded());
            CHECK(gbt7.empty());
            CHECK(gbt7.size() == 0);
            CHECK(!gbt7.isNotEmpty());

            GBoundedBufferT<move_only_struct, 10> gbt8;
            CHECK(gbt8.getCapacity() == 10);
            CHECK(gbt8.isBounded());
            CHECK(gbt8.empty());
            CHECK(gbt8.size() == 0);
            CHECK(!gbt8.isNotEmpty());

            GBoundedBufferT<move_only_struct, 20> gbt9;
            CHECK(gbt9.getCapacity() == 20);
            CHECK(gbt9.isBounded());
            CHECK(gbt9.empty());
            CHECK(gbt9.size() == 0);
            CHECK(!gbt9.isNotEmpty());

            GBoundedBufferT<move_only_struct, 30> gbt10;
            CHECK(gbt10.getCapacity() == 30);
            CHECK(gbt10.isBounded());
            CHECK(gbt10.empty());
            CHECK(gbt10.size() == 0);
            CHECK(!gbt10.isNotEmpty());

            //-------------------------------------------------------------------
            // copy_move_struct

            GBoundedBufferT<copy_move_struct> gpbt11; // DEFAULTBUFFERSIZE
            CHECK(gpbt11.getCapacity() == DEFAULTBUFFERSIZE);
            CHECK(gpbt11.isBounded());
            CHECK(gpbt11.empty());
            CHECK(gpbt11.size() == 0);
            CHECK(!gpbt11.isNotEmpty());

            GBoundedBufferT<copy_move_struct, 0> gpbt12; // unbounded
            CHECK(gpbt12.getCapacity() == 0);
            CHECK(!gpbt12.isBounded());
            CHECK(gpbt12.empty());
            CHECK(gpbt12.size() == 0);
            CHECK(!gpbt12.isNotEmpty());

            GBoundedBufferT<copy_move_struct, 10> gpbt13;
            CHECK(gpbt13.getCapacity() == 10);
            CHECK(gpbt13.isBounded());
            CHECK(gpbt13.empty());
            CHECK(gpbt13.size() == 0);
            CHECK(!gpbt13.isNotEmpty());

            GBoundedBufferT<copy_move_struct, 20> gpbt14;
            CHECK(gpbt14.getCapacity() == 20);
            CHECK(gpbt14.isBounded());
            CHECK(gpbt14.empty());
            CHECK(gpbt14.size() == 0);
            CHECK(!gpbt14.isNotEmpty());

            GBoundedBufferT<copy_move_struct, 30> gpbt15;
            CHECK(gpbt15.getCapacity() == 30);
            CHECK(gpbt15.isBounded());
            CHECK(gpbt15.empty());
            CHECK(gpbt15.size() == 0);
            CHECK(!gpbt15.isNotEmpty());

            //-------------------------------------------------------------------
        }

        //----------------------------------------------------------------------

        { // Test adding work items to the queue with try_push_* and removing them subsequently
            {
                //------------------------------------------
                // First to an unbounded queue with copy_only_struct

                GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;

                bool push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
                    copy_only_struct c(i);
                    CHECK(c.getSecret() == i); // Copy should not alter this value
                    CHECK_NOTHROW(push_succeeded = gbt_co_unbounded.try_push_copy(c));
                    CHECK(!gbt_co_unbounded.empty());
                    CHECK(push_succeeded);
                    CHECK(gbt_co_unbounded.size() == i + 1);
                    CHECK(c.getSecret() == i);

                    push_succeeded = false;
                }

                bool pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // Copy should not alter this value; i++) { // Remove items
                    copy_only_struct c(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_co_unbounded.try_pop_copy(c));
                    CHECK(pop_succeeded);
                    CHECK(gbt_co_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(c.getSecret() == i);

                    pop_succeeded = false;
                }

                CHECK(gbt_co_unbounded.size() == 0);
                CHECK(gbt_co_unbounded.empty());

                //------------------------------------------
                // Next with an unbounded queue with move_only_struct

                GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;

                push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
                    move_only_struct m(i);
                    CHECK(m.getSecret() == i);
                    CHECK_NOTHROW(push_succeeded = gbt_mo_unbounded.try_push_move(std::move(m)));
                    CHECK(m.getSecret() == 0); // Should have been cleared after move
                    CHECK(!gbt_mo_unbounded.empty());
                    CHECK(push_succeeded);
                    CHECK(gbt_mo_unbounded.size() == i + 1);

                    push_succeeded = false;
                }

                pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Remove items
                    move_only_struct m(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_mo_unbounded.try_pop_move(m));
                    CHECK(pop_succeeded);
                    CHECK(gbt_mo_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(m.getSecret() == i);

                    pop_succeeded = false;
                }

                CHECK(gbt_mo_unbounded.size() == 0);
                CHECK(gbt_mo_unbounded.empty());

                //------------------------------------------
                // Now with copy_move_struct, while only copying

                GBoundedBufferT<copy_move_struct, 0> gbt_copy_only_cms_unbounded;

                push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
                    copy_move_struct c(i);
                    CHECK(c.getSecret() == i); // Copy should not alter this value
                    CHECK_NOTHROW(push_succeeded = gbt_copy_only_cms_unbounded.try_push_copy(c));
                    CHECK(!gbt_copy_only_cms_unbounded.empty());
                    CHECK(push_succeeded);
                    CHECK(gbt_copy_only_cms_unbounded.size() == i + 1);
                    CHECK(c.getSecret() == i);

                    push_succeeded = false;
                }

                pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // Copy should not alter this value; i++) { // Remove items
                    copy_move_struct c(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_copy_only_cms_unbounded.try_pop_copy(c));
                    CHECK(pop_succeeded);
                    CHECK(gbt_copy_only_cms_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(c.getSecret() == i);

                    // Check that the item ws only copied, never moved, but that one of the two has happened
                    CHECK(c.struct_was_copied());
                    CHECK(!c.struct_was_moved());
                    CHECK(c.struct_was_copied_or_moved());

                    pop_succeeded = false;
                }

                CHECK(gbt_copy_only_cms_unbounded.size() == 0);
                CHECK(gbt_copy_only_cms_unbounded.empty());

                //------------------------------------------
                // Now with copy_move_struct, while only moving

                GBoundedBufferT<copy_move_struct, 0> gbt_move_only_cms_unbounded;

                push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
                    copy_move_struct m(i);
                    CHECK(m.getSecret() == i);
                    CHECK_NOTHROW(
                        push_succeeded = gbt_move_only_cms_unbounded.try_push_move(std::move(m))
                    );
                    CHECK(m.getSecret() == 0); // Should have been cleared after move
                    CHECK(!gbt_move_only_cms_unbounded.empty());
                    CHECK(push_succeeded);
                    CHECK(gbt_move_only_cms_unbounded.size() == i + 1);

                    push_succeeded = false;
                }

                pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Remove items
                    copy_move_struct m(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_move_only_cms_unbounded.try_pop_move(m));
                    CHECK(pop_succeeded);
                    CHECK(gbt_move_only_cms_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(m.getSecret() == i);

                    // Check that the item ws only moved, never copied, but that one of the two has happened
                    CHECK(!m.struct_was_copied());
                    CHECK(m.struct_was_moved());
                    CHECK(m.struct_was_copied_or_moved());

                    pop_succeeded = false;
                }

                CHECK(gbt_move_only_cms_unbounded.size() == 0);
                CHECK(gbt_move_only_cms_unbounded.empty());

                //------------------------------------------
            }

            {
                //------------------------------------------
                // Now to a bounded queue with copy_only_struct

                GBoundedBufferT<copy_only_struct> gbt_co_bounded; // DEFAULTBUFFERSIZE
                CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);

                bool push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // More than the capacity of the queue
                    copy_only_struct c(i);
                    CHECK_NOTHROW(push_succeeded = gbt_co_bounded.try_push_copy(c));
                    CHECK(!gbt_co_bounded.empty());
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(push_succeeded);
                        CHECK(gbt_co_bounded.size() == i + 1);
                    }
                    else {
                        CHECK(!push_succeeded);
                        CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE);
                    }
                    CHECK(c.getSecret() == i); // No changes by copying, or if the item was ignored

                    push_succeeded = false;
                }

                bool pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Remove items
                    copy_only_struct c(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_co_bounded.try_pop_copy(c));
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(pop_succeeded);
                        CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                        CHECK(c.getSecret() == i);
                    }
                    else { // We try to remove more items than were in the queue
                        CHECK(!pop_succeeded);
                        CHECK(gbt_co_bounded.size() == 0);
                        CHECK(gbt_co_bounded.empty());
                        CHECK(
                            c.getSecret() == 3 * DEFAULTBUFFERSIZE
                        ); // No item was popped, so original value remains
                    }

                    pop_succeeded = false;
                }

                //------------------------------------------
                // Next with a bounded queue with move_only_struct

                GBoundedBufferT<move_only_struct> gbt_mo_bounded; // DEFAULTBUFFERSIZE
                CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);

                push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // More than the capacity of the queue
                    move_only_struct m(i);
                    CHECK_NOTHROW(push_succeeded = gbt_mo_bounded.try_push_move(std::move(m)));
                    CHECK(!gbt_mo_bounded.empty());
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(push_succeeded);
                        CHECK(m.getSecret() == 0); // Should have been cleared after move
                        CHECK(gbt_mo_bounded.size() == i + 1);
                    }
                    else {
                        CHECK(!push_succeeded);
                        CHECK(
                            m.getSecret() == i
                        ); // Should not have been altered by move if item was ignored
                        CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE);
                    }

                    push_succeeded = false;
                }

                pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // Remove items (more than are stored in the queue)
                    move_only_struct m(3 * DEFAULTBUFFERSIZE); // This value should never be found
                    CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_mo_bounded.try_pop_move(m));
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(pop_succeeded);
                        CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                        CHECK(m.getSecret() == i);
                    }
                    else {
                        CHECK(!pop_succeeded);
                        CHECK(gbt_mo_bounded.size() == 0);
                        CHECK(
                            m.getSecret() == 3 * DEFAULTBUFFERSIZE
                        ); // Should not be altered, as no items were popped
                    }

                    pop_succeeded = false;
                }

                CHECK(gbt_mo_bounded.size() == 0);
                CHECK(gbt_mo_bounded.empty());

                //------------------------------------------
            }
        }

        //----------------------------------------------------------------------

        { // Test adding work items to the queue with push_and_block_* and removing them subsequently
            {
                //------------------------------------------
                // First to an unbounded queue with copy_only_struct

                GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;

                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
                    copy_only_struct c(i);
                    CHECK(c.getSecret() == i); // Copy should not alter this value
                    CHECK_NOTHROW(gbt_co_unbounded.push_and_block_copy(c));
                    CHECK(!gbt_co_unbounded.empty());
                    CHECK(gbt_co_unbounded.size() == i + 1);
                    CHECK(c.getSecret() == i);
                }

                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Remove items
                    copy_only_struct c(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(gbt_co_unbounded.pop_and_block_copy(c));
                    CHECK(gbt_co_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(c.getSecret() == i);
                }

                CHECK(gbt_co_unbounded.size() == 0);
                CHECK(gbt_co_unbounded.empty());

                //------------------------------------------
                // Next with an unbounded queue with move_only_struct

                GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;

                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
                    move_only_struct m(i);
                    CHECK(m.getSecret() == i);
                    CHECK_NOTHROW(gbt_mo_unbounded.push_and_block_move(std::move(m)));
                    CHECK(m.getSecret() == 0); // Should have been cleared after move
                    CHECK(!gbt_mo_unbounded.empty());
                    CHECK(gbt_mo_unbounded.size() == i + 1);
                }

                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Remove items
                    move_only_struct m(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(gbt_mo_unbounded.pop_and_block_move(m));
                    CHECK(gbt_mo_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(m.getSecret() == i);
                }

                CHECK(gbt_mo_unbounded.size() == 0);
                CHECK(gbt_mo_unbounded.empty());

                //------------------------------------------
            }

            {
                //------------------------------------------
                // Now to a bounded queue with copy_only_struct

                GBoundedBufferT<copy_only_struct> gbt_co_bounded; // DEFAULTBUFFERSIZE
                CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);

                for(std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
                    copy_only_struct c(i);
                    CHECK_NOTHROW(gbt_co_bounded.push_and_block_copy(c));
                    CHECK(!gbt_co_bounded.empty());
                    CHECK(gbt_co_bounded.size() == i + 1);
                    CHECK(c.getSecret() == i); // No changes by copying, or if the item was ignored
                }

                for(std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) { // Remove items
                    copy_only_struct c(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(gbt_co_bounded.pop_and_block_copy(c));
                    CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                    CHECK(c.getSecret() == i);
                }

                //------------------------------------------
                // Next with a bounded queue with move_only_struct

                GBoundedBufferT<move_only_struct> gbt_mo_bounded; // DEFAULTBUFFERSIZE
                CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);

                for(std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
                    move_only_struct m(i);
                    CHECK_NOTHROW(gbt_mo_bounded.push_and_block_move(std::move(m)));
                    CHECK(!gbt_mo_bounded.empty());
                    CHECK(m.getSecret() == 0); // Should have been cleared after move
                    CHECK(gbt_mo_bounded.size() == i + 1);
                }

                for(std::size_t i = 0; i < DEFAULTBUFFERSIZE;
                    i++) { // Remove items (more than are stored in the queue)
                    move_only_struct m(3 * DEFAULTBUFFERSIZE); // This value should never be found
                    CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(gbt_mo_bounded.pop_and_block_move(m));
                    CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                    CHECK(m.getSecret() == i);
                }

                CHECK(gbt_mo_bounded.size() == 0);
                CHECK(gbt_mo_bounded.empty());

                //------------------------------------------
            }
        }

        //----------------------------------------------------------------------

        { // Test adding work items to the queue with push_and_wait_* and removing them subsequently
            std::chrono::duration<double> timeout(std::chrono::microseconds(1));

            {
                //------------------------------------------
                // First to an unbounded queue with copy_only_struct

                GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;

                bool push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
                    copy_only_struct c(i);
                    CHECK(c.getSecret() == i); // Copy should not alter this value
                    CHECK_NOTHROW(push_succeeded = gbt_co_unbounded.push_and_wait_copy(c, timeout));
                    CHECK(!gbt_co_unbounded.empty());
                    CHECK(push_succeeded);
                    CHECK(gbt_co_unbounded.size() == i + 1);
                    CHECK(c.getSecret() == i);

                    push_succeeded = false;
                }

                bool pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // Copy should not alter this value; i++) { // Remove items
                    copy_only_struct c(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_co_unbounded.pop_and_wait_copy(c, timeout));
                    CHECK(pop_succeeded);
                    CHECK(gbt_co_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(c.getSecret() == i);

                    pop_succeeded = false;
                }

                CHECK(gbt_co_unbounded.size() == 0);
                CHECK(gbt_co_unbounded.empty());

                //------------------------------------------
                // Next with an unbounded queue with move_only_struct

                GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;

                push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
                    move_only_struct m(i);
                    CHECK(m.getSecret() == i);
                    CHECK_NOTHROW(
                        push_succeeded = gbt_mo_unbounded.push_and_wait_move(std::move(m), timeout)
                    );
                    CHECK(m.getSecret() == 0); // Should have been cleared after move
                    CHECK(!gbt_mo_unbounded.empty());
                    CHECK(push_succeeded);
                    CHECK(gbt_mo_unbounded.size() == i + 1);

                    push_succeeded = false;
                }

                pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Remove items
                    move_only_struct m(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_mo_unbounded.pop_and_wait_move(m, timeout));
                    CHECK(pop_succeeded);
                    CHECK(gbt_mo_unbounded.size() == 2 * DEFAULTBUFFERSIZE - i - 1);
                    CHECK(m.getSecret() == i);

                    pop_succeeded = false;
                }

                CHECK(gbt_mo_unbounded.size() == 0);
                CHECK(gbt_mo_unbounded.empty());

                //------------------------------------------
            }

            {
                //------------------------------------------
                // Now to a bounded queue with copy_only_struct

                GBoundedBufferT<copy_only_struct> gbt_co_bounded; // DEFAULTBUFFERSIZE
                CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);

                bool push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // More than the capacity of the queue
                    copy_only_struct c(i);
                    CHECK_NOTHROW(push_succeeded = gbt_co_bounded.push_and_wait_copy(c, timeout));
                    CHECK(!gbt_co_bounded.empty());
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(push_succeeded);
                        CHECK(gbt_co_bounded.size() == i + 1);
                    }
                    else {
                        CHECK(!push_succeeded);
                        CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE);
                    }
                    CHECK(c.getSecret() == i); // No changes by copying, or if the item was ignored

                    push_succeeded = false;
                }

                bool pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Remove items
                    copy_only_struct c(3 * DEFAULTBUFFERSIZE); // This value should never be reached
                    CHECK(c.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_co_bounded.pop_and_wait_copy(c, timeout));
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(pop_succeeded);
                        CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                        CHECK(c.getSecret() == i);
                    }
                    else { // We try to remove more items than were in the queue
                        CHECK(!pop_succeeded);
                        CHECK(gbt_co_bounded.size() == 0);
                        CHECK(gbt_co_bounded.empty());
                        CHECK(
                            c.getSecret() == 3 * DEFAULTBUFFERSIZE
                        ); // No item was popped, so original value remains
                    }

                    pop_succeeded = false;
                }

                //------------------------------------------
                // Next with a bounded queue with move_only_struct

                GBoundedBufferT<move_only_struct> gbt_mo_bounded; // DEFAULTBUFFERSIZE
                CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);

                push_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // More than the capacity of the queue
                    move_only_struct m(i);
                    CHECK_NOTHROW(
                        push_succeeded = gbt_mo_bounded.push_and_wait_move(std::move(m), timeout)
                    );
                    CHECK(!gbt_mo_bounded.empty());
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(push_succeeded);
                        CHECK(m.getSecret() == 0); // Should have been cleared after move
                        CHECK(gbt_mo_bounded.size() == i + 1);
                    }
                    else {
                        CHECK(!push_succeeded);
                        CHECK(
                            m.getSecret() == i
                        ); // Should not have been altered by move if item was ignored
                        CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE);
                    }

                    push_succeeded = false;
                }

                pop_succeeded = false;
                for(std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE;
                    i++) { // Remove items (more than are stored in the queue)
                    move_only_struct m(3 * DEFAULTBUFFERSIZE); // This value should never be found
                    CHECK(m.getSecret() == 3 * DEFAULTBUFFERSIZE);
                    CHECK_NOTHROW(pop_succeeded = gbt_mo_bounded.pop_and_wait_move(m, timeout));
                    if(i < DEFAULTBUFFERSIZE) {
                        CHECK(pop_succeeded);
                        CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
                        CHECK(m.getSecret() == i);
                    }
                    else {
                        CHECK(!pop_succeeded);
                        CHECK(gbt_mo_bounded.size() == 0);
                        CHECK(
                            m.getSecret() == 3 * DEFAULTBUFFERSIZE
                        ); // Should not be altered, as no items were popped
                    }

                    pop_succeeded = false;
                }

                CHECK(gbt_mo_bounded.size() == 0);
                CHECK(gbt_mo_bounded.empty());

                //------------------------------------------
            }
        }

        //----------------------------------------------------------------------
    }

    /*************************************************************************/
    /**
	  * Test features that are expected to fail
	  */
    void failures_expected() {
        { /* nothing */
        }
    }
};

/******************************************************************************/

} /* namespace Tests */
} /* namespace Common */
} /* namespace Gem */
