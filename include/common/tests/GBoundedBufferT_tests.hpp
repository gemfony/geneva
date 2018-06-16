/**
 * @file GBoundedBufferT_tests.hpp
 *
 * Tests of the GBoundedBufferT class
 */

// Standard headers go here

// Boost headers go here
#include <boost/test/unit_test.hpp>

// Geneva headers go here
#include "common/GBoundedBufferT.hpp"

namespace Gem {
namespace Common {
namespace Tests {

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
		 }

		 //----------------------------------------------------------------------

		 { // Check boundaries after construction
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
		 }

		 //----------------------------------------------------------------------

		 { // Test adding work items to the queue
		 	 //------------------------------------------
			 // First to an unbounded queue with copy_only_struct

			 GBoundedBufferT<copy_only_struct, 0> gbt_co_unbounded;

			 bool push_succeeded = false;
			 for(std::size_t i=0; i<2*DEFAULTBUFFERSIZE; i++) {
				 	copy_only_struct c(i);
					BOOST_CHECK_NO_THROW(push_succeeded = gbt_co_unbounded.try_push_copy(c));
					BOOST_CHECK(push_succeeded);
					BOOST_CHECK(gbt_co_unbounded.size() == i+1);

					push_succeeded = false;
			 }

			 //------------------------------------------
			 // Next with an unbounded queue with move_only_struct

			 GBoundedBufferT<move_only_struct, 0> gbt_mo_unbounded;

			 push_succeeded = false;
			 for(std::size_t i=0; i<2*DEFAULTBUFFERSIZE; i++) {
				 move_only_struct m(i);
				 BOOST_CHECK_NO_THROW(push_succeeded = gbt_mo_unbounded.try_push_move(std::move(m)));
				 BOOST_CHECK(push_succeeded);
				 BOOST_CHECK(gbt_mo_unbounded.size() == i+1);

				 push_succeeded = false;
			 }

			 //------------------------------------------
		 }

		 //----------------------------------------------------------------------
		 //----------------------------------------------------------------------
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
