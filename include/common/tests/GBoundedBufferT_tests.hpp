/**
 * @file GBoundedBufferT_tests.hpp
 *
 * Tests of the GBoundedBufferT class
 */

// Standard headers go here
#include <vector>
#include <algorithm>

// Boost headers go here
#include <boost/test/unit_test.hpp>

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
	 copy_only_struct(std::size_t secret) : m_secret(secret) { /* nothing */ }
	 copy_only_struct(const copy_only_struct& cp) = default;
	 copy_only_struct(copy_only_struct&&) = delete;

	 copy_only_struct& operator=(copy_only_struct& cp) = default;
	 copy_only_struct& operator=(copy_only_struct&& cp) = delete;

	 std::size_t getSecret() const {
		 return m_secret;
	 }

private:
	 std::size_t m_secret = 0;
};

/******************************************************************************/
/**
 * A simple struct meant to test moving of work items.
 */
struct move_only_struct {
public:
	 move_only_struct() = delete;

	 move_only_struct(std::size_t secret) : m_secret(secret) { /* nothing */ }
	 move_only_struct(const move_only_struct& cp) = delete;
	 move_only_struct(move_only_struct&& cp) {
		 m_secret = cp.m_secret;
		 cp.m_secret = 0;
	 }

	 move_only_struct& operator=(move_only_struct& cp) = delete;
	 move_only_struct& operator=(move_only_struct&& cp) {
		 m_secret = cp.m_secret;
		 cp.m_secret = 0;
	 }

	 std::size_t getSecret() const {
		 return m_secret;
	 }

private:
	 std::size_t m_secret = 0;
};

/******************************************************************************/
/**
 * A simple struct meant to make sure the correct functions were called
 */
struct copy_move_struct {
public:
	 copy_move_struct() = delete;

	 copy_move_struct(std::size_t secret) : m_secret(secret) { /* nothing */ }
	 copy_move_struct(const copy_move_struct& cp)
		 : m_secret(cp.m_secret)
		 , m_copy_move_history(cp.m_copy_move_history)
	 {
		 m_copy_move_history.push_back(M_COPIED);
	 }
	 copy_move_struct(copy_move_struct&& cp) {
		 m_secret = cp.m_secret;
		 cp.m_secret = 0;
		 m_copy_move_history = std::move(cp.m_copy_move_history);
		 m_copy_move_history.push_back(M_MOVED);
	 }

	 copy_move_struct& operator=(copy_move_struct& cp) {
	 	 m_secret = cp.m_secret;
	 	 m_copy_move_history = cp.m_copy_move_history;
		 m_copy_move_history.push_back(M_COPIED);
	 }
	 copy_move_struct& operator=(copy_move_struct&& cp) {
		 m_secret = cp.m_secret;
		 cp.m_secret = 0;
		 m_copy_move_history = std::move(cp.m_copy_move_history);
		 m_copy_move_history.push_back(M_MOVED);
	 }

	 bool struct_was_copied() {
		 if(std::find(m_copy_move_history.begin(), m_copy_move_history.end(), M_COPIED) != m_copy_move_history.end()) {
		 	return true;
		 }
		 return false;
	 }

	 bool struct_was_moved() {
		 if(std::find(m_copy_move_history.begin(), m_copy_move_history.end(), M_MOVED) != m_copy_move_history.end()) {
			 return true;
		 }
		 return false;
	 }

	 bool struct_was_copied_and_moved() { // This should not happen in the tests
	 	return (struct_was_copied() && struct_was_moved());
	 }

	 bool struct_was_copied_or_moved() {
	 	return !m_copy_move_history.empty();
	 }

	 std::size_t getSecret() const {
		 return m_secret;
	 }

private:
	 const std::uint32_t M_COPIED=0, M_MOVED=1;

	 std::size_t m_secret = 0;
	 std::vector<std::uint32_t> m_copy_move_history;
};

/******************************************************************************/
/**
 * Unit tests for the GBoundedBufferT class
 */
class GBoundedBufferT_tests
{
public:
	 /*************************************************************************/
	 /**
	  * Test of features that are expected to work
	  */
	 void no_failure_expected() {
	 	 //----------------------------------------------------------------------

		 { // Check construction with different sizes and value types
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_only_struct>())); // DEFAULTBUFFERSIZE
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_only_struct, 0>())); // unbounded
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_only_struct, 10>()));
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_only_struct, 20>()));
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_only_struct, 30>()));

			 BOOST_CHECK_NO_THROW((GBoundedBufferT<move_only_struct>())); // DEFAULTBUFFERSIZE
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<move_only_struct, 0>())); // unbounded
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<move_only_struct, 10>()));
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<move_only_struct, 20>()));
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<move_only_struct, 30>()));

			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_move_struct>())); // DEFAULTBUFFERSIZE
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_move_struct, 0>())); // unbounded
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_move_struct, 10>()));
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_move_struct, 20>()));
			 BOOST_CHECK_NO_THROW((GBoundedBufferT<copy_move_struct, 30>()));
		 }

		 //----------------------------------------------------------------------

		 { // Check boundaries after construction
		 	 //-------------------------------------------------------------------
		 	 // copy_only_struct

			 GBoundedBufferT<copy_only_struct> gbt1; // DEFAULTBUFFERSIZE
			 BOOST_CHECK(gbt1.getCapacity() == DEFAULTBUFFERSIZE);
			 BOOST_CHECK(gbt1.isBounded());
			 BOOST_CHECK(gbt1.empty());
			 BOOST_CHECK(gbt1.size() == 0);
			 BOOST_CHECK(!gbt1.isNotEmpty());

			 GBoundedBufferT<copy_only_struct, 0> gbt2; // unbounded
			 BOOST_CHECK(gbt2.getCapacity() == 0);
			 BOOST_CHECK(!gbt2.isBounded());
			 BOOST_CHECK(gbt2.empty());
			 BOOST_CHECK(gbt2.size() == 0);
			 BOOST_CHECK(!gbt2.isNotEmpty());

			 GBoundedBufferT<copy_only_struct, 10> gbt3;
			 BOOST_CHECK(gbt3.getCapacity() == 10);
			 BOOST_CHECK(gbt3.isBounded());
			 BOOST_CHECK(gbt3.empty());
			 BOOST_CHECK(gbt3.size() == 0);
			 BOOST_CHECK(!gbt3.isNotEmpty());

			 GBoundedBufferT<copy_only_struct, 20> gbt4;
			 BOOST_CHECK(gbt4.getCapacity() == 20);
			 BOOST_CHECK(gbt4.isBounded());
			 BOOST_CHECK(gbt4.empty());
			 BOOST_CHECK(gbt4.size() == 0);
			 BOOST_CHECK(!gbt4.isNotEmpty());

			 GBoundedBufferT<copy_only_struct, 30> gbt5;
			 BOOST_CHECK(gbt5.getCapacity() == 30);
			 BOOST_CHECK(gbt5.isBounded());
			 BOOST_CHECK(gbt5.empty());
			 BOOST_CHECK(gbt5.size() == 0);
			 BOOST_CHECK(!gbt5.isNotEmpty());

			 //-------------------------------------------------------------------
		 	 // move_only_struct

			 GBoundedBufferT<move_only_struct> gbt6; // DEFAULTBUFFERSIZE
			 BOOST_CHECK(gbt6.getCapacity() == DEFAULTBUFFERSIZE);
			 BOOST_CHECK(gbt6.isBounded());
			 BOOST_CHECK(gbt6.empty());
			 BOOST_CHECK(gbt6.size() == 0);
			 BOOST_CHECK(!gbt6.isNotEmpty());

			 GBoundedBufferT<move_only_struct, 0> gbt7; // unbounded
			 BOOST_CHECK(gbt7.getCapacity() == 0);
			 BOOST_CHECK(!gbt7.isBounded());
			 BOOST_CHECK(gbt7.empty());
			 BOOST_CHECK(gbt7.size() == 0);
			 BOOST_CHECK(!gbt7.isNotEmpty());

			 GBoundedBufferT<move_only_struct, 10> gbt8;
			 BOOST_CHECK(gbt8.getCapacity() == 10);
			 BOOST_CHECK(gbt8.isBounded());
			 BOOST_CHECK(gbt8.empty());
			 BOOST_CHECK(gbt8.size() == 0);
			 BOOST_CHECK(!gbt8.isNotEmpty());

			 GBoundedBufferT<move_only_struct, 20> gbt9;
			 BOOST_CHECK(gbt9.getCapacity() == 20);
			 BOOST_CHECK(gbt9.isBounded());
			 BOOST_CHECK(gbt9.empty());
			 BOOST_CHECK(gbt9.size() == 0);
			 BOOST_CHECK(!gbt9.isNotEmpty());

			 GBoundedBufferT<move_only_struct, 30> gbt10;
			 BOOST_CHECK(gbt10.getCapacity() == 30);
			 BOOST_CHECK(gbt10.isBounded());
			 BOOST_CHECK(gbt10.empty());
			 BOOST_CHECK(gbt10.size() == 0);
			 BOOST_CHECK(!gbt10.isNotEmpty());

			 //-------------------------------------------------------------------
			 // copy_move_struct

			 GBoundedBufferT<copy_move_struct> gpbt11; // DEFAULTBUFFERSIZE
			 BOOST_CHECK(gpbt11.getCapacity() == DEFAULTBUFFERSIZE);
			 BOOST_CHECK(gpbt11.isBounded());
			 BOOST_CHECK(gpbt11.empty());
			 BOOST_CHECK(gpbt11.size() == 0);
			 BOOST_CHECK(!gpbt11.isNotEmpty());

			 GBoundedBufferT<copy_move_struct, 0> gpbt12; // unbounded
			 BOOST_CHECK(gpbt12.getCapacity() == 0);
			 BOOST_CHECK(!gpbt12.isBounded());
			 BOOST_CHECK(gpbt12.empty());
			 BOOST_CHECK(gpbt12.size() == 0);
			 BOOST_CHECK(!gpbt12.isNotEmpty());

			 GBoundedBufferT<copy_move_struct, 10> gpbt13;
			 BOOST_CHECK(gpbt13.getCapacity() == 10);
			 BOOST_CHECK(gpbt13.isBounded());
			 BOOST_CHECK(gpbt13.empty());
			 BOOST_CHECK(gpbt13.size() == 0);
			 BOOST_CHECK(!gpbt13.isNotEmpty());

			 GBoundedBufferT<copy_move_struct, 20> gpbt14;
			 BOOST_CHECK(gpbt14.getCapacity() == 20);
			 BOOST_CHECK(gpbt14.isBounded());
			 BOOST_CHECK(gpbt14.empty());
			 BOOST_CHECK(gpbt14.size() == 0);
			 BOOST_CHECK(!gpbt14.isNotEmpty());

			 GBoundedBufferT<copy_move_struct, 30> gpbt15;
			 BOOST_CHECK(gpbt15.getCapacity() == 30);
			 BOOST_CHECK(gpbt15.isBounded());
			 BOOST_CHECK(gpbt15.empty());
			 BOOST_CHECK(gpbt15.size() == 0);
			 BOOST_CHECK(!gpbt15.isNotEmpty());

			 //-------------------------------------------------------------------
		 }

		 //----------------------------------------------------------------------

		 { // Test adding work items to the queue with try_push_* and removing them subsequently
			 {
				 //------------------------------------------
				 // First to an unbounded queue with copy_only_struct

				 GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;

				 bool push_succeeded = false;
				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
					 copy_only_struct c(i);
					 BOOST_CHECK(c.getSecret() == i); // Copy should not alter this value
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_co_unbounded.try_push_copy(c));
					 BOOST_CHECK(!gbt_co_unbounded.empty());
					 BOOST_CHECK(push_succeeded);
					 BOOST_CHECK(gbt_co_unbounded.size() == i + 1);
					 BOOST_CHECK(c.getSecret() == i);

					 push_succeeded = false;
				 }

				 bool pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Copy should not alter this value; i++) { // Remove items
					 copy_only_struct c(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_co_unbounded.try_pop_copy(c));
				 	 BOOST_CHECK(pop_succeeded);
				 	 BOOST_CHECK(gbt_co_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
				 	 BOOST_CHECK(c.getSecret() == i);

				 	 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_co_unbounded.size() == 0);
				 BOOST_CHECK(gbt_co_unbounded.empty());

				 //------------------------------------------
				 // Next with an unbounded queue with move_only_struct

				 GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;

				 push_succeeded = false;
				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
					 move_only_struct m(i);
					 BOOST_CHECK(m.getSecret() == i);
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_mo_unbounded.try_push_move(std::move(m)));
					 BOOST_CHECK(m.getSecret() == 0); // Should have been cleared after move
					 BOOST_CHECK(!gbt_mo_unbounded.empty());
					 BOOST_CHECK(push_succeeded);
					 BOOST_CHECK(gbt_mo_unbounded.size() == i + 1);

					 push_succeeded = false;
				 }


				 pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items
					 move_only_struct m(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_mo_unbounded.try_pop_move(m));
					 BOOST_CHECK(pop_succeeded);
					 BOOST_CHECK(gbt_mo_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(m.getSecret() == i);

					 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_mo_unbounded.size() == 0);
				 BOOST_CHECK(gbt_mo_unbounded.empty());

				 //------------------------------------------
				 // Now with copy_move_struct, while only copying

				 GBoundedBufferT<copy_move_struct, 0> gbt_copy_only_cms_unbounded;

				 push_succeeded = false;
				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
					 copy_move_struct c(i);
					 BOOST_CHECK(c.getSecret() == i); // Copy should not alter this value
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_copy_only_cms_unbounded.try_push_copy(c));
					 BOOST_CHECK(!gbt_copy_only_cms_unbounded.empty());
					 BOOST_CHECK(push_succeeded);
					 BOOST_CHECK(gbt_copy_only_cms_unbounded.size() == i + 1);
					 BOOST_CHECK(c.getSecret() == i);

					 push_succeeded = false;
				 }

				 pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Copy should not alter this value; i++) { // Remove items
					 copy_move_struct c(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_copy_only_cms_unbounded.try_pop_copy(c));
					 BOOST_CHECK(pop_succeeded);
					 BOOST_CHECK(gbt_copy_only_cms_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(c.getSecret() == i);

					 // Check that the item ws only copied, never moved, but that one of the two has happened
					 BOOST_CHECK(c.struct_was_copied());
					 BOOST_CHECK(!c.struct_was_moved());
					 BOOST_CHECK(c.struct_was_copied_or_moved());

					 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_copy_only_cms_unbounded.size() == 0);
				 BOOST_CHECK(gbt_copy_only_cms_unbounded.empty());

				 //------------------------------------------
				 // Now with copy_move_struct, while only moving

				 GBoundedBufferT<copy_move_struct, 0> gbt_move_only_cms_unbounded;

				 push_succeeded = false;
				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
					 copy_move_struct m(i);
					 BOOST_CHECK(m.getSecret() == i);
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_move_only_cms_unbounded.try_push_move(std::move(m)));
					 BOOST_CHECK(m.getSecret() == 0); // Should have been cleared after move
					 BOOST_CHECK(!gbt_move_only_cms_unbounded.empty());
					 BOOST_CHECK(push_succeeded);
					 BOOST_CHECK(gbt_move_only_cms_unbounded.size() == i + 1);

					 push_succeeded = false;
				 }


				 pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items
					 copy_move_struct m(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_move_only_cms_unbounded.try_pop_move(m));
					 BOOST_CHECK(pop_succeeded);
					 BOOST_CHECK(gbt_move_only_cms_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(m.getSecret() == i);

					 // Check that the item ws only moved, never copied, but that one of the two has happened
					 BOOST_CHECK(!m.struct_was_copied());
					 BOOST_CHECK(m.struct_was_moved());
					 BOOST_CHECK(m.struct_was_copied_or_moved());

					 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_move_only_cms_unbounded.size() == 0);
				 BOOST_CHECK(gbt_move_only_cms_unbounded.empty());


				 //------------------------------------------
			 }

			 {
				 //------------------------------------------
				 // Now to a bounded queue with copy_only_struct

				 GBoundedBufferT<copy_only_struct> gbt_co_bounded; // DEFAULTBUFFERSIZE
				 BOOST_CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);

				 bool push_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // More than the capacity of the queue
					 copy_only_struct c(i);
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_co_bounded.try_push_copy(c));
					 BOOST_CHECK(!gbt_co_bounded.empty());
					 if(i<DEFAULTBUFFERSIZE) {
						 BOOST_CHECK(push_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == i + 1);
					 } else {
						 BOOST_CHECK(!push_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE);
					 }
					 BOOST_CHECK(c.getSecret() == i); // No changes by copying, or if the item was ignored

					 push_succeeded = false;
				 }

				 bool pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items
					 copy_only_struct c(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_co_bounded.try_pop_copy(c));
					 if(i<DEFAULTBUFFERSIZE) {
						 BOOST_CHECK(pop_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
						 BOOST_CHECK(c.getSecret() == i);
					 } else { // We try to remove more items than were in the queue
						 BOOST_CHECK(!pop_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == 0);
						 BOOST_CHECK(gbt_co_bounded.empty());
						 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE); // No item was popped, so original value remains
					 }

					 pop_succeeded = false;
				 }

				 //------------------------------------------
				 // Next with a bounded queue with move_only_struct

				 GBoundedBufferT<move_only_struct> gbt_mo_bounded; // DEFAULTBUFFERSIZE
				 BOOST_CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);

				 push_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // More than the capacity of the queue
					 move_only_struct m(i);
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_mo_bounded.try_push_move(std::move(m)));
					 BOOST_CHECK(!gbt_mo_bounded.empty());
					 if(i<DEFAULTBUFFERSIZE) {
						 BOOST_CHECK(push_succeeded);
						 BOOST_CHECK(m.getSecret() == 0); // Should have been cleared after move
						 BOOST_CHECK(gbt_mo_bounded.size() == i + 1);
					 } else {
						 BOOST_CHECK(!push_succeeded);
						 BOOST_CHECK(m.getSecret() == i); // Should not have been altered by move if item was ignored
						 BOOST_CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE);
					 }

					 push_succeeded = false;
				 }

				 pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items (more than are stored in the queue)
					 move_only_struct m(3*DEFAULTBUFFERSIZE); // This value should never be found
					 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_mo_bounded.try_pop_move(m));
					 if(i<DEFAULTBUFFERSIZE) {
					 	BOOST_CHECK(pop_succeeded);
					 	BOOST_CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
					 	BOOST_CHECK(m.getSecret() == i);
					 } else {
						 BOOST_CHECK(!pop_succeeded);
						 BOOST_CHECK(gbt_mo_bounded.size() == 0);
						 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE); // Should not be altered, as no items were popped
					 }

					 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_mo_bounded.size() == 0);
				 BOOST_CHECK(gbt_mo_bounded.empty());

				 //------------------------------------------
			 }
		 }

		 //----------------------------------------------------------------------

		 { // Test adding work items to the queue with push_and_block_* and removing them subsequently
			 {
				 //------------------------------------------
				 // First to an unbounded queue with copy_only_struct

				 GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;

				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
					 copy_only_struct c(i);
					 BOOST_CHECK(c.getSecret() == i); // Copy should not alter this value
					 BOOST_CHECK_NO_THROW(gbt_co_unbounded.push_and_block_copy(c));
					 BOOST_CHECK(!gbt_co_unbounded.empty());
					 BOOST_CHECK(gbt_co_unbounded.size() == i + 1);
					 BOOST_CHECK(c.getSecret() == i);
				 }

				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items
					 copy_only_struct c(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(gbt_co_unbounded.pop_and_block_copy(c));
					 BOOST_CHECK(gbt_co_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(c.getSecret() == i);
				 }

				 BOOST_CHECK(gbt_co_unbounded.size() == 0);
				 BOOST_CHECK(gbt_co_unbounded.empty());

				 //------------------------------------------
				 // Next with an unbounded queue with move_only_struct

				 GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;

				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
					 move_only_struct m(i);
					 BOOST_CHECK(m.getSecret() == i);
					 BOOST_CHECK_NO_THROW(gbt_mo_unbounded.push_and_block_move(std::move(m)));
					 BOOST_CHECK(m.getSecret() == 0); // Should have been cleared after move
					 BOOST_CHECK(!gbt_mo_unbounded.empty());
					 BOOST_CHECK(gbt_mo_unbounded.size() == i + 1);
				 }

				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items
					 move_only_struct m(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(gbt_mo_unbounded.pop_and_block_move(m));
					 BOOST_CHECK(gbt_mo_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(m.getSecret() == i);
				 }

				 BOOST_CHECK(gbt_mo_unbounded.size() == 0);
				 BOOST_CHECK(gbt_mo_unbounded.empty());

				 //------------------------------------------
			 }

			 {
				 //------------------------------------------
				 // Now to a bounded queue with copy_only_struct

				 GBoundedBufferT<copy_only_struct> gbt_co_bounded; // DEFAULTBUFFERSIZE
				 BOOST_CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);

				 for (std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
					 copy_only_struct c(i);
					 BOOST_CHECK_NO_THROW(gbt_co_bounded.push_and_block_copy(c));
					 BOOST_CHECK(!gbt_co_bounded.empty());
					 BOOST_CHECK(gbt_co_bounded.size() == i + 1);
					 BOOST_CHECK(c.getSecret() == i); // No changes by copying, or if the item was ignored
				 }

				 for (std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) { // Remove items
					 copy_only_struct c(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(gbt_co_bounded.pop_and_block_copy(c));
					 BOOST_CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(c.getSecret() == i);
				 }

				 //------------------------------------------
				 // Next with a bounded queue with move_only_struct

				 GBoundedBufferT<move_only_struct> gbt_mo_bounded; // DEFAULTBUFFERSIZE
				 BOOST_CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);

				 for (std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) {
					 move_only_struct m(i);
					 BOOST_CHECK_NO_THROW(gbt_mo_bounded.push_and_block_move(std::move(m)));
					 BOOST_CHECK(!gbt_mo_bounded.empty());
					 BOOST_CHECK(m.getSecret() == 0); // Should have been cleared after move
					 BOOST_CHECK(gbt_mo_bounded.size() == i + 1);
				 }

				 for (std::size_t i = 0; i < DEFAULTBUFFERSIZE; i++) { // Remove items (more than are stored in the queue)
					 move_only_struct m(3*DEFAULTBUFFERSIZE); // This value should never be found
					 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(gbt_mo_bounded.pop_and_block_move(m));
					 BOOST_CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(m.getSecret() == i);
				 }

				 BOOST_CHECK(gbt_mo_bounded.size() == 0);
				 BOOST_CHECK(gbt_mo_bounded.empty());

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
				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) { // Add items
					 copy_only_struct c(i);
					 BOOST_CHECK(c.getSecret() == i); // Copy should not alter this value
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_co_unbounded.push_and_wait_copy(c, timeout));
					 BOOST_CHECK(!gbt_co_unbounded.empty());
					 BOOST_CHECK(push_succeeded);
					 BOOST_CHECK(gbt_co_unbounded.size() == i + 1);
					 BOOST_CHECK(c.getSecret() == i);

					 push_succeeded = false;
				 }

				 bool pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Copy should not alter this value; i++) { // Remove items
					 copy_only_struct c(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_co_unbounded.pop_and_wait_copy(c, timeout));
					 BOOST_CHECK(pop_succeeded);
					 BOOST_CHECK(gbt_co_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(c.getSecret() == i);

					 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_co_unbounded.size() == 0);
				 BOOST_CHECK(gbt_co_unbounded.empty());

				 //------------------------------------------
				 // Next with an unbounded queue with move_only_struct

				 GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;

				 push_succeeded = false;
				 for (std::size_t i = 0; i < 2 * DEFAULTBUFFERSIZE; i++) {
					 move_only_struct m(i);
					 BOOST_CHECK(m.getSecret() == i);
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_mo_unbounded.push_and_wait_move(std::move(m), timeout));
					 BOOST_CHECK(m.getSecret() == 0); // Should have been cleared after move
					 BOOST_CHECK(!gbt_mo_unbounded.empty());
					 BOOST_CHECK(push_succeeded);
					 BOOST_CHECK(gbt_mo_unbounded.size() == i + 1);

					 push_succeeded = false;
				 }


				 pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items
					 move_only_struct m(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_mo_unbounded.pop_and_wait_move(m, timeout));
					 BOOST_CHECK(pop_succeeded);
					 BOOST_CHECK(gbt_mo_unbounded.size() == 2*DEFAULTBUFFERSIZE - i - 1);
					 BOOST_CHECK(m.getSecret() == i);

					 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_mo_unbounded.size() == 0);
				 BOOST_CHECK(gbt_mo_unbounded.empty());

				 //------------------------------------------
			 }

			 {
				 //------------------------------------------
				 // Now to a bounded queue with copy_only_struct

				 GBoundedBufferT<copy_only_struct> gbt_co_bounded; // DEFAULTBUFFERSIZE
				 BOOST_CHECK(gbt_co_bounded.getCapacity() == DEFAULTBUFFERSIZE);

				 bool push_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // More than the capacity of the queue
					 copy_only_struct c(i);
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_co_bounded.push_and_wait_copy(c, timeout));
					 BOOST_CHECK(!gbt_co_bounded.empty());
					 if(i<DEFAULTBUFFERSIZE) {
						 BOOST_CHECK(push_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == i + 1);
					 } else {
						 BOOST_CHECK(!push_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE);
					 }
					 BOOST_CHECK(c.getSecret() == i); // No changes by copying, or if the item was ignored

					 push_succeeded = false;
				 }

				 bool pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items
					 copy_only_struct c(3*DEFAULTBUFFERSIZE); // This value should never be reached
					 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_co_bounded.pop_and_wait_copy(c, timeout));
					 if(i<DEFAULTBUFFERSIZE) {
						 BOOST_CHECK(pop_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
						 BOOST_CHECK(c.getSecret() == i);
					 } else { // We try to remove more items than were in the queue
						 BOOST_CHECK(!pop_succeeded);
						 BOOST_CHECK(gbt_co_bounded.size() == 0);
						 BOOST_CHECK(gbt_co_bounded.empty());
						 BOOST_CHECK(c.getSecret() == 3*DEFAULTBUFFERSIZE); // No item was popped, so original value remains
					 }

					 pop_succeeded = false;
				 }

				 //------------------------------------------
				 // Next with a bounded queue with move_only_struct

				 GBoundedBufferT<move_only_struct> gbt_mo_bounded; // DEFAULTBUFFERSIZE
				 BOOST_CHECK(gbt_mo_bounded.getCapacity() == DEFAULTBUFFERSIZE);

				 push_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // More than the capacity of the queue
					 move_only_struct m(i);
					 BOOST_CHECK_NO_THROW(push_succeeded = gbt_mo_bounded.push_and_wait_move(std::move(m), timeout));
					 BOOST_CHECK(!gbt_mo_bounded.empty());
					 if(i<DEFAULTBUFFERSIZE) {
						 BOOST_CHECK(push_succeeded);
						 BOOST_CHECK(m.getSecret() == 0); // Should have been cleared after move
						 BOOST_CHECK(gbt_mo_bounded.size() == i + 1);
					 } else {
						 BOOST_CHECK(!push_succeeded);
						 BOOST_CHECK(m.getSecret() == i); // Should not have been altered by move if item was ignored
						 BOOST_CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE);
					 }

					 push_succeeded = false;
				 }

				 pop_succeeded = false;
				 for (std::size_t i = 0; i < 2*DEFAULTBUFFERSIZE; i++) { // Remove items (more than are stored in the queue)
					 move_only_struct m(3*DEFAULTBUFFERSIZE); // This value should never be found
					 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE);
					 BOOST_CHECK_NO_THROW(pop_succeeded = gbt_mo_bounded.pop_and_wait_move(m, timeout));
					 if(i<DEFAULTBUFFERSIZE) {
						 BOOST_CHECK(pop_succeeded);
						 BOOST_CHECK(gbt_mo_bounded.size() == DEFAULTBUFFERSIZE - i - 1);
						 BOOST_CHECK(m.getSecret() == i);
					 } else {
						 BOOST_CHECK(!pop_succeeded);
						 BOOST_CHECK(gbt_mo_bounded.size() == 0);
						 BOOST_CHECK(m.getSecret() == 3*DEFAULTBUFFERSIZE); // Should not be altered, as no items were popped
					 }

					 pop_succeeded = false;
				 }

				 BOOST_CHECK(gbt_mo_bounded.size() == 0);
				 BOOST_CHECK(gbt_mo_bounded.empty());

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
		 { /* nothing */ }
	 }
};

/******************************************************************************/

} /* namespace Tests */
} /* namespace Common */
} /* namespace Gem */
