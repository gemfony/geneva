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

#include "geneva/GTestIndividual1.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Tests::GTestIndividual1) // NOLINT

namespace Gem::Tests {

/******************************************************************************/
/**
 * The default constructor.
 */
GTestIndividual1::GTestIndividual1() {
    // Fill with some data
    std::shared_ptr<Gem::Geneva::GDoubleCollection> gdc_ptr(
        new Gem::Geneva::GDoubleCollection(100, -10., 10.)
    );
    std::shared_ptr<Gem::Geneva::GDoubleGaussAdaptor> gdga1(
        new Gem::Geneva::GDoubleGaussAdaptor(0.025, 0.1, 0., 1.)
    );

    // Prevent changes to adProb_
    gdga1->setAdaptAdProb(0.);

    gdc_ptr->addAdaptor(gdga1);
    gdc_ptr->randomInit(Gem::Geneva::activityMode::ACTIVEONLY, gr_);
    this->push_back(gdc_ptr);
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GTestIndividual1::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GTestIndividual1 reference independent of this object and convert the pointer
    const GTestIndividual1 *p_load =
        Gem::Common::g_convert_and_compare<GObject, GTestIndividual1>(cp, this);

    GToken token("GTestIndividual1", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterSet>(*this, *p_load, token);

    // ... no local data

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Loads the data of another GTestIndividual1, camouflaged as a GObject.
 *
 * @param cp A copy of another GTestIndividual1, camouflaged as a GObject
 */
void GTestIndividual1::load_(const GObject *cp) {
    using namespace Gem::Common;
    using namespace Gem::Geneva;

    // Check that we are dealing with a GTestIndividual1 reference independent of this object and convert the pointer
    const GTestIndividual1 *p_load =
        Gem::Common::g_convert_and_compare<GObject, GTestIndividual1>(cp, this);

    // Load our parent's data
    GParameterSet::load_(cp);

    // No local data
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
Gem::Geneva::GObject *GTestIndividual1::clone_() const {
    return new GTestIndividual1(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @param id The id of the target function (ignored here)
 * @return The value of this object
 */
double GTestIndividual1::fitnessCalculation() {
    double result = 0.;

    // Extract the first Gem::Geneva::GDoubleCollection object. In a realistic scenario, you might want
    // to add error checks here upon first invocation.
    std::shared_ptr<Gem::Geneva::GDoubleCollection> v_c = at<Gem::Geneva::GDoubleCollection>(0);

    // Calculate the value of the parabola
    for(std::size_t i = 0; i < v_c->size(); i++) {
        result += v_c->at(i) * v_c->at(i);
    }

    return result;
}

// Note: The following code is designed to mainly test parent classes

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 * only.
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GTestIndividual1::modify_GUnitTests_() {
#ifdef GEM_TESTING

    bool result = false;

    // Call the parent classes' functions
    if(Gem::Geneva::GParameterSet::modify_GUnitTests_()) {
        result = true;
    }

    // Change the parameter settings
    this->adapt();
    result = true;

    return result;
#else
    return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Adds a number of GDoubleObject objects to the individual
 *
 * @param nItems The number of items to be added
 */
void GTestIndividual1::addGDoubleObjects_(const std::size_t &n_items) {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Clear the collection, so we can start fresh
    CHECK_NOTHROW(this->clear());

    // Add GDoubleObject items with adaptors to p_test1
    for(std::size_t i = 0; i < n_items; i++) {
        // Create a suitable adaptor
        std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr;

        CHECK_NOTHROW(
            gdga_ptr = std::make_shared<GDoubleGaussAdaptor>(0.025, 0.1, 0., 1., 1.0)
        );
        CHECK_NOTHROW(
            gdga_ptr->setAdaptionThreshold(0)
        ); // Make sure the adaptor's internal parameters don't change through the adaption
        CHECK_NOTHROW(gdga_ptr->setAdaptionMode(adaptionMode::ALWAYS)); // Always adapt

        // Create a suitable GDoubleObject object
        std::shared_ptr<GDoubleObject> gdo_ptr;

        CHECK_NOTHROW(
            gdo_ptr = std::make_shared<GDoubleObject>(-100., 100.)
        ); // Initialization in the range -100, 100

        // Add the adaptor
        CHECK_NOTHROW(gdo_ptr->addAdaptor(gdga_ptr));

        // Randomly initialize the GDoubleObject object, so it is unique
        CHECK_NOTHROW(gdo_ptr->randomInit(Gem::Geneva::activityMode::ACTIVEONLY, gr_));

        // Add the object to the collection
        CHECK_NOTHROW(this->push_back(gdo_ptr));
    }
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GTestIndividual1::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // A few settings
    const std::size_t n_items = 100;

    // Call the parent classes' functions
    Gem::Geneva::GParameterSet::specificTestsNoFailureExpected_GUnitTests_();

    //------------------------------------------------------------------------------

    { // Tests whether calls to adapt() result in changes of the object
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test_old =
            this->clone<Gem::Tests::GTestIndividual1>();

        std::size_t n_tests = 1000;

        for(std::size_t i = 0; i < n_tests; i++) {
            CHECK_NOTHROW(p_test->adapt());
            CHECK(*p_test != *p_test_old);
            CHECK_NOTHROW(p_test_old->GObject::load(p_test));
        }
    }

    //------------------------------------------------------------------------------

    { // Tests customAdaptions, dirty_flag and the effects of the fitness function. Also test setting of server-mode flag
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure this individual is not dirty
        if(p_test->is_due_for_processing()) {
            CHECK_NOTHROW(p_test->process());
            CHECK(p_test->is_processed());
        }

        std::size_t n_tests = 1000;

        double current_fitness = 0.;
        double old_fitness = current_fitness;
        bool dirty_flag = false;

        for(std::size_t i = 0; i < n_tests; i++) {
            // Change the parameters without instantly triggering fitness calculation
            CHECK_NOTHROW(p_test->customAdaptions());
            // The dirty flag should not have been set yet (done in adapt() )
            INFO("Processing status = " << p_test->getProcessingStatusAsStr() << ", i = " << i);
            CHECK((p_test->is_processed() || p_test->is_ignored()));
            // Set the flag manually
            CHECK_NOTHROW(p_test->mark_as_due_for_processing());
            // Check that the dirty flag has indeed been set
            CHECK(p_test->is_due_for_processing());
            CHECK(p_test->getProcessingStatus() == Gem::Courtier::processingStatus::DO_PROCESS);

            // Trigger value calculation
            CHECK_NOTHROW(p_test->process());
            CHECK(p_test->is_processed());
            CHECK(p_test->getProcessingStatus() == Gem::Courtier::processingStatus::PROCESSED);
            CHECK_NOTHROW(current_fitness = p_test->transformed_fitness(0));

            // Check that the evaluation has changed
            if(i > 0) {
                INFO(
                    "\n"
                    << "current_fitness = " << current_fitness << "\n"
                    << "old_fitness = " << old_fitness << "\n"
                    << "iteration = " << i << "\n"
                );
                CHECK(// Check that the fitness has changed
						current_fitness != old_fitness);
            }
            old_fitness = current_fitness;
        }
    }

    //------------------------------------------------------------------------------

    { // Tests whether modification of one clone influences another clone
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test1 =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure the individual is clean
        if(p_test1->is_due_for_processing()) {
            CHECK_NOTHROW(p_test1->process());
            CHECK(p_test1->is_processed());
        }

        // Create a clone of p_test1
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test2 =
            p_test1->clone<Gem::Tests::GTestIndividual1>();
        // Check that the clone is identical to p_test1;
        CHECK_NOTHROW(*p_test2 == *p_test1);

        // Modify p_test2
        std::size_t n_adaptions = 0;
        CHECK_NOTHROW(n_adaptions = p_test2->adapt());
        // Make sure adaptions were indeed performed
        CHECK(n_adaptions > 0);
        // Check that it is dirty
        CHECK(p_test2->is_due_for_processing());
        // Check that p_test1 is not dirty
        CHECK(not p_test1->is_due_for_processing());
        // Check that the two individuals differ
        CHECK(*p_test1 != *p_test2);
    }

    //------------------------------------------------------------------------------

    { // Check the effects of the process function in EA mode, using the "evaluate" call
        double current_fitness = 0.;
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure the individual is clean
        CHECK_NOTHROW(p_test->is_processed() || p_test->is_ignored());

        // Set the dirty flag
        CHECK_NOTHROW(p_test->mark_as_due_for_processing());

        // Setting the dirty flag should result in DO_PROCESS being set
        CHECK(Gem::Courtier::processingStatus::DO_PROCESS == p_test->getProcessingStatus());

        // Check that the dirty flag has indeed been set
        CHECK(p_test->is_due_for_processing());

        // Tell the individual about its personality and duty
        CHECK_NOTHROW(p_test->setPersonality(
            std::make_shared<oa::GEvolutionaryAlgorithm_PersonalityTraits>()
        ));

        // Calling the process() function with the "evaluate" call should clear the dirty flag
        CHECK_NOTHROW(p_test->process());

        // The dirty flag should have been cleared
        CHECK(p_test->is_processed());
    }

    //------------------------------------------------------------------------------

    { // Check the process() function
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure the individual is clean
        CHECK_NOTHROW(p_test->is_ignored() || p_test->is_processed());

        // Set the dirty flag
        CHECK_NOTHROW(p_test->mark_as_due_for_processing());

        // Check that the dirty flag has indeed been set
        CHECK(p_test->is_due_for_processing());

        // Tell the individual about its personality
        CHECK_NOTHROW(p_test->setPersonality(
            std::make_shared<oa::GEvolutionaryAlgorithm_PersonalityTraits>()
        ));

        // Calling the process() function with the "evaluate" call should clear the dirty flag
        CHECK_NOTHROW(p_test->process());

        // The dirty flag should have been cleared
        CHECK(p_test->is_processed());
    }

    //------------------------------------------------------------------------------

    { // Check the effects of the process function in SWARM mode, using the "evaluate" call
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure the individual is clean
        CHECK_NOTHROW(p_test->is_ignored() || p_test->is_processed());

        // Set the dirty flag
        CHECK_NOTHROW(p_test->mark_as_due_for_processing());

        // Check that the dirty flag has indeed been set
        CHECK(p_test->is_due_for_processing());

        // Tell the individual about its personality and duty
        CHECK_NOTHROW(p_test->setPersonality(
            std::make_shared<oa::GSwarmAlgorithm_PersonalityTraits>()
        ));

        // Calling the process() function with the "evaluate" call should clear the dirty flag
        CHECK_NOTHROW(p_test->process());

        // The dirty flag should have been cleared
        CHECK(p_test->is_processed());
    }

    //------------------------------------------------------------------------------

    { // Test of Gem::Common::GPtrVectorT<T,GObject>::swap(...)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test1 =
            this->clone<Gem::Tests::GTestIndividual1>();
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test2 =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Check that both individuals are the same
        CHECK(*p_test1 == *p_test2);

        // Adapt p_test2, so that both individuals are different
        CHECK_NOTHROW(p_test2->adapt());

        // Make sure both individuals are clean and evaluated
        double fitness1_old = 0., fitness2_old = 0;
        CHECK_NOTHROW(p_test1->mark_as_due_for_processing());
        CHECK_NOTHROW(p_test2->mark_as_due_for_processing());
        CHECK(p_test1->is_due_for_processing());
        CHECK(p_test2->is_due_for_processing());
        CHECK_NOTHROW(p_test1->process());
        CHECK_NOTHROW(p_test2->process());
        CHECK(p_test1->is_processed());
        CHECK(p_test2->is_processed());
        CHECK_NOTHROW(fitness1_old = p_test1->transformed_fitness(0));
        CHECK_NOTHROW(fitness2_old = p_test2->transformed_fitness(0));

        // Make sure the individuals are different
        CHECK(*p_test1 != *p_test2);

        // Make sure their fitness differs
        CHECK(p_test1->raw_fitness(0) != p_test2->raw_fitness(0));

        // Swap their data vectors
        CHECK_NOTHROW(p_test1->swap(*p_test2));

        // They should now both have the dirty flag set
        CHECK(p_test1->is_due_for_processing());
        CHECK(p_test2->is_due_for_processing());

        // Make sure both individuals are clean and evaluated
        double fitness1_new = 0., fitness2_new = 0;
        CHECK_NOTHROW(p_test1->process());
        CHECK_NOTHROW(p_test2->process());
        CHECK(p_test1->is_processed());
        CHECK(p_test2->is_processed());
        CHECK_NOTHROW(fitness1_new = p_test1->transformed_fitness(0));
        CHECK_NOTHROW(fitness2_new = p_test2->transformed_fitness(0));

        // The fitness values of both individuals should effectively have been exchanged
        // Note that rounding errors might prevent fitness1_new to be == fitness2_old
        // and vice versa
        CHECK(fabs(fitness1_new - fitness2_old) < pow(10, -8));
        CHECK(fabs(fitness2_new - fitness1_old) < pow(10, -8));
    }

    //------------------------------------------------------------------------------

    { // Check of the GParameterSet::customAdaptions() function
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test1 =
            this->clone<Gem::Tests::GTestIndividual1>();
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test2 =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Check that both individuals are the same
        CHECK(*p_test1 == *p_test2);

        // Make sure both individuals are clean and evaluated
        double fitness1_old = 0., fitness2_old = 0;
        CHECK_NOTHROW(p_test1->mark_as_due_for_processing());
        CHECK_NOTHROW(p_test2->mark_as_due_for_processing());
        CHECK(p_test1->is_due_for_processing());
        CHECK(p_test2->is_due_for_processing());
        CHECK_NOTHROW(p_test1->process());
        CHECK_NOTHROW(p_test2->process());
        CHECK(p_test1->is_processed());
        CHECK(p_test2->is_processed());
        CHECK_NOTHROW(fitness1_old = p_test1->transformed_fitness(0));
        CHECK_NOTHROW(fitness2_old = p_test2->transformed_fitness(0));

        // Extract and clone the first individual's GDoubleCollection object for later comparisons
        std::shared_ptr<Gem::Geneva::GDoubleCollection> gdc_ptr_old =
            p_test1->at(static_cast<std::size_t>(0))->clone<Gem::Geneva::GDoubleCollection>();

        // Adapt and evaluate the first individual
        CHECK_NOTHROW(p_test1->customAdaptions());
        // We need to manually mark the individual as dirty
        CHECK_NOTHROW(p_test1->mark_as_due_for_processing());

        // The fitness of individual1 should have changed. Re-evaluate and check
        double fitness1_new = 0.;
        CHECK_NOTHROW(p_test1->process());
        CHECK_NOTHROW(fitness1_new = p_test1->transformed_fitness(0));
        CHECK(fitness1_new != fitness1_old);

        // The individuals should now differ
        CHECK(*p_test1 != *p_test2);

        // Extract and clone the first individual's GDoubleCollection object for comparison
        std::shared_ptr<Gem::Geneva::GDoubleCollection> gdc_ptr_new =
            p_test1->at(0)->clone<Gem::Geneva::GDoubleCollection>();

        // Check that both GDoubleCollection objects differ
        CHECK(*gdc_ptr_old != *gdc_ptr_new);
    }

    //------------------------------------------------------------------------------

    { // Test resize_clone, resize_noclone, finding and counting of items (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Initialize with a fixed value
        CHECK_NOTHROW(p_test->fixedValueInit<double>(42., activityMode::ALLPARAMETERS));

        // Check the current size
        CHECK(p_test->size() == n_items);

        // Create a copy of the first parameter item
        std::shared_ptr<GDoubleObject> search_ptr;
        Gem::Tests::GTestIndividual1::const_iterator find_cit;
        CHECK_NOTHROW(search_ptr = p_test->at(0)->clone<GDoubleObject>());

        // Find the first item that complies to a GDoubleObject, initialized with the number 42
        CHECK_NOTHROW(find_cit = p_test->find(search_ptr));
        CHECK(find_cit == p_test->begin());

        // Resize, so that only one item remains, cross-check
        CHECK_NOTHROW(p_test->resize_clone(1, search_ptr));
        CHECK(p_test->size() == 1);

        // Use resize_clone to resize to the original size
        CHECK_NOTHROW(p_test->resize_clone(n_items, search_ptr));

        // Count the number of items identical to search_ptr (should be nItems)
        CHECK(p_test->count(search_ptr) == n_items);

        // Resize again to 1, using resize_noclone
        CHECK_NOTHROW(p_test->resize_noclone(1, search_ptr));
        CHECK(p_test->size() == 1);

        // Resize back to the original size
        CHECK_NOTHROW(p_test->resize_noclone(n_items, search_ptr));
        CHECK(p_test->size() == n_items);

        // Check that the pointer of the last item is identical to the one used in search_ptr
        CHECK((p_test->back()).get() == search_ptr.get());
    }

    //------------------------------------------------------------------------------

    { // Test insert_clone, insert_noclone (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Check the current size
        CHECK(p_test->size() == n_items);

        // Create a copy of the first parameter item
        std::shared_ptr<GDoubleObject> insert_ptr;
        CHECK_NOTHROW(insert_ptr = p_test->at(0)->clone<GDoubleObject>());

        // Assign a fixed value to insert_ptr
        CHECK_NOTHROW(*insert_ptr = 1.);
        CHECK(insert_ptr->value() == 1.);

        // Insert one item and check the resulting size and value of the first item
        CHECK_NOTHROW(p_test->insert_clone(p_test->begin(), insert_ptr));
        CHECK(p_test->size() == n_items + 1);
        CHECK(p_test->at<GDoubleObject>(0)->value() == 1.);

        // Find the first item which is identical to insert_ptr -- should be at the beginning
        Gem::Tests::GTestIndividual1::const_iterator find_cit;
        CHECK_NOTHROW(find_cit = p_test->find(insert_ptr));
        CHECK(find_cit == p_test->begin());

        // Insert another (nItems) - 1 items and count the number of items identical to insert_ptr
        CHECK_NOTHROW(p_test->insert_clone(p_test->begin(), n_items - 1, insert_ptr));
        CHECK(p_test->size() == 2 * n_items);
        CHECK(static_cast<std::size_t>(p_test->count(insert_ptr)) >= n_items);

        // Check that there is no item with the same physical address as insert_ptr
        for(std::size_t i = 0; i < p_test->size(); i++) {
            CHECK((p_test->at(i)).get() != insert_ptr.get());
        }

        // Insert one more item at the end, using insert_noclone
        CHECK_NOTHROW(p_test->insert_noclone(p_test->end(), insert_ptr));
        CHECK(p_test->size() == 2 * n_items + 1);

        // There should now be exactly one item with the same address as insert_ptr (i.e. the same object)
        std::size_t n_identical = 0;
        for(std::size_t i = 0; i < p_test->size(); i++) {
            if((p_test->at(i)).get() == insert_ptr.get()) {
                n_identical++;
            }
        }
        CHECK(n_identical == 1);

        // Remove the item again and check the size
        CHECK_NOTHROW(p_test->pop_back());
        CHECK(p_test->size() == 2 * n_items);

        // Check that there is no item left with the same address
        for(std::size_t i = 0; i < p_test->size(); i++) {
            CHECK((p_test->at(i)).get() != insert_ptr.get());
        }

        // Insert another nItems items at the beginning, using insert_noclone; cross-check the size
        CHECK_NOTHROW(p_test->insert_noclone(p_test->begin(), n_items, insert_ptr));
        CHECK(p_test->size() == 3 * n_items);

        // There should again be exactly one item with the same address as insert_ptr (i.e. the same object)
        n_identical = 0;
        for(std::size_t i = 0; i < p_test->size(); i++) {
            if((p_test->at(i)).get() == insert_ptr.get()) {
                n_identical++;
            }
        }
        CHECK(n_identical == 1);

        // The identical item should be at the very beginning of the collection
        CHECK((p_test->at<GDoubleObject>(0)).get() == insert_ptr.get());

        // count == 0 must be a no-op for both insert_clone and insert_noclone
        // (regression: insert_noclone previously computed `count - 1` on an
        //  unsigned counter, causing infinite-loop / OOM when count == 0).
        const std::size_t size_before_zero = p_test->size();
        CHECK_NOTHROW(p_test->insert_clone(p_test->begin(), std::size_t(0), insert_ptr));
        CHECK(p_test->size() == size_before_zero);
        CHECK_NOTHROW(p_test->insert_noclone(p_test->begin(), std::size_t(0), insert_ptr));
        CHECK(p_test->size() == size_before_zero);
    }

    //------------------------------------------------------------------------------

    { // Test push_back_clone and push_back_noclone (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Check the current size
        CHECK(p_test->size() == n_items);

        // Create a copy of the first parameter item
        std::shared_ptr<GDoubleObject> pushback_ptr;
        CHECK_NOTHROW(pushback_ptr = p_test->at(0)->clone<GDoubleObject>());

        // Assign a fixed value to pushback_ptr
        CHECK_NOTHROW(*pushback_ptr = 1.);
        CHECK(pushback_ptr->value() == 1.);

        // Push back the cloned item to the collection; cross-check the size and the pointers
        CHECK_NOTHROW(p_test->push_back_clone(pushback_ptr));
        CHECK(p_test->size() == n_items + 1);
        CHECK((p_test->back()).get() != pushback_ptr.get());

        // Push back the un-cloned item to the collection; cross-check the size and the pointers
        CHECK_NOTHROW(p_test->push_back_noclone(pushback_ptr));
        CHECK(p_test->size() == n_items + 2);
        CHECK((p_test->back()).get() == pushback_ptr.get());
    }

    //------------------------------------------------------------------------------

    { // Test retrieval of a data copy (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Check the current size
        CHECK(p_test->size() == n_items);

        std::vector<std::shared_ptr<GParameterBase>> data_copy;
        CHECK_NOTHROW(p_test->getDataCopy(data_copy));

        // Check the size and content
        CHECK((data_copy.size() == p_test->size() && not p_test->empty()));
        for(std::size_t i = 0; i < p_test->size(); i++) {
            CHECK((p_test->at(i)).get() != data_copy.at(i).get());
        }
    }

    //------------------------------------------------------------------------------

    { // Check setting and retrieval of the current personality status and whether the personalities themselves can be accessed
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();
        std::shared_ptr<GPersonalityTraits> p_pt;

        // Reset the personality type
        CHECK_NOTHROW(p_test->resetPersonality());
        INFO(
            "\n"
            << "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
            << "expected PERSONALITY_NONE\n"
        );
        CHECK(p_test->getPersonality() == "PERSONALITY_NONE");

        // Set the personality type to EA
        CHECK_NOTHROW(p_test->setPersonality(
            std::make_shared<oa::GEvolutionaryAlgorithm_PersonalityTraits>()
        ));
        INFO(
            "\n"
            << "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
            << "expected EA\n"
        );
        CHECK(p_test->getPersonality() == "GEvolutionaryAlgorithm_PersonalityTraits");

        // Try to retrieve a GEvolutionaryAlgorithm_PersonalityTraits object and check that the smart pointer actually points somewhere
        std::shared_ptr<oa::GEvolutionaryAlgorithm_PersonalityTraits> p_pt_ea;
        CHECK_NOTHROW(
            p_pt_ea = p_test->getPersonalityTraits<oa::GEvolutionaryAlgorithm_PersonalityTraits>()
        );
        CHECK(p_pt_ea);
        p_pt_ea.reset();

        // Retrieve a base pointer to the EA object and check that it points somewhere
        CHECK_NOTHROW(p_pt = p_test->getPersonalityTraits());
        CHECK(p_pt);
        p_pt.reset();

        // Set the personality type to GD
        CHECK_NOTHROW(p_test->setPersonality(
            std::make_shared<oa::GGradientDescent_PersonalityTraits>()
        ));
        INFO(
            "\n"
            << "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
            << "expected GGradientDescent_PersonalityTraits\n"
        );
        CHECK(p_test->getPersonality() == "GGradientDescent_PersonalityTraits");

        // Try to retrieve a GGradientDescent_PersonalityTraits object and check that the smart pointer actually points somewhere
        std::shared_ptr<oa::GGradientDescent_PersonalityTraits> p_pt_gd;
        CHECK_NOTHROW(p_pt_gd = p_test->getPersonalityTraits<oa::GGradientDescent_PersonalityTraits>());
        CHECK(p_pt_gd);
        p_pt_gd.reset();

        // Retrieve a base pointer to the GD object and check that it points somewhere
        CHECK_NOTHROW(p_pt = p_test->getPersonalityTraits());
        CHECK(p_pt);
        p_pt.reset();

        // Set the personality type to SWARM
        CHECK_NOTHROW(p_test->setPersonality(
            std::make_shared<oa::GSwarmAlgorithm_PersonalityTraits>()
        ));
        INFO(
            "\n"
            << "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
            << "expected GSwarmAlgorithm_PersonalityTraits\n"
        );
        CHECK(p_test->getPersonality() == "GSwarmAlgorithm_PersonalityTraits");

        // Try to retrieve a GSwarmAlgorithm_PersonalityTraits object and check that the smart pointer actually points somewhere
        std::shared_ptr<oa::GSwarmAlgorithm_PersonalityTraits> p_pt_swarm;
        CHECK_NOTHROW(
            p_pt_swarm = p_test->getPersonalityTraits<oa::GSwarmAlgorithm_PersonalityTraits>()
        );
        CHECK(p_pt_swarm);
        p_pt_swarm.reset();

        // Retrieve a base pointer to the SWARM object and check that it points somewhere
        CHECK_NOTHROW(p_pt = p_test->getPersonalityTraits());
        CHECK(p_pt);
        p_pt.reset();

        // Set the personality type to PERSONALITY_NONE
        CHECK_NOTHROW(p_test->resetPersonality());
        INFO(
            "\n"
            << "p_test->getPersonality() = " << p_test->getPersonality() << "\n"
            << "expected PERSONALITY_NONE\n"
        );
        CHECK(p_test->getPersonality() == "PERSONALITY_NONE");
    }

    // --------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "Gem::Tests::GTestIndividual1::specificTestsNoFailureExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GTestIndividual1::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // A few settings
    const std::size_t n_items = 100;

    // Call the parent classes' functions
    Gem::Geneva::GParameterSet::specificTestsFailuresExpected_GUnitTests_();

    //------------------------------------------------------------------------------

#ifdef DEBUG
    { // Tests that evaluating a dirty individual in "server mode" throws
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        CHECK_NOTHROW(p_test->mark_as_due_for_processing());
        CHECK_THROWS_AS(p_test->transformed_fitness(0), geneva_exception);
    }
#endif /* DEBUG */

    //------------------------------------------------------------------------------

    { // Test that trying to count an empty smart pointer throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to count the number of occurrences of an empty smart pointer. Should throw
        CHECK_THROWS_AS((p_test->count(std::shared_ptr<GDoubleObject>())), geneva_exception);
    }

    //------------------------------------------------------------------------------

    { // Test that trying to find an empty smart pointer throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to find an empty smart pointer. Should throw
        CHECK_THROWS_AS((p_test->find(std::shared_ptr<GDoubleObject>())), geneva_exception);
    }

    //------------------------------------------------------------------------------

    { // Test that trying to insert an empty smart pointer with insert_noclone(pos, item) throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to insert an empty smart pointers. Should throw
        CHECK_THROWS_AS(
            p_test->insert_noclone(p_test->begin(), std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

    { // Test that trying to insert an empty smart pointer with insert_noclone(pos, amount, item) throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to insert a number of empty smart pointers. Should throw
        CHECK_THROWS_AS(
            p_test->insert_noclone(p_test->begin(), 10, std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

    { // Test that trying to insert an empty smart pointer with insert_clone(pos, item) throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to insert a number of empty smart pointers. Should throw
        CHECK_THROWS_AS(
            p_test->insert_clone(p_test->begin(), std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

    { // Test that trying to insert an empty smart pointer with insert_clone(pos, amount, item) throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to insert a number of empty smart pointers. Should throw
        CHECK_THROWS_AS(
            p_test->insert_clone(p_test->begin(), 10, std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

    { // Test that trying to add an empty smart pointer with push_back_clone(item) throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to attach an empty smart pointer Should throw
        CHECK_THROWS_AS(
            p_test->push_back_clone(std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

    { // Test that trying to add an empty smart pointer with push_back_noclone(item) throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Add a few data items
        CHECK_NOTHROW(p_test->addGDoubleObjects_(n_items));

        // Try to attach an empty smart pointer Should throw
        CHECK_THROWS_AS(
            p_test->push_back_noclone(std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

    { // Test that trying to resize an empty collection with resize(amount) throws (Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure p_test is empty
        CHECK_NOTHROW(p_test->clear());
        CHECK(p_test->empty());

        // Try to resize an empty collection
        CHECK_THROWS_AS((p_test->resize(10)), geneva_exception);
    }

    //------------------------------------------------------------------------------

    { // Test that trying to resize an empty collection with resize_noclone(amount, item) throws if item is an empty smart pointer(Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure p_test is empty
        CHECK_NOTHROW(p_test->clear());
        CHECK(p_test->empty());

        // Try to resize an empty collection
        CHECK_THROWS_AS(
            p_test->resize_noclone(10, std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

    { // Test that trying to resize an empty collection with resize_clone(amount, item) throws if item is an empty smart pointer(Test of GPtrVectorT<GParameterBase> functionality)
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure p_test is empty
        CHECK_NOTHROW(p_test->clear());
        CHECK(p_test->empty());

        // Try to resize an empty collection
        CHECK_THROWS_AS(
            p_test->resize_clone(10, std::shared_ptr<GDoubleObject>()),
            geneva_exception
        );
    }

    //------------------------------------------------------------------------------

#ifdef DEBUG
    { // Test that retrieval of an EA personality traits object from an uninitialized pointer throws in DEBUG mode
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure the personality type is set to PERSONALITY_NONE
        CHECK_NOTHROW(p_test->resetPersonality());

        // Trying to retrieve an EA personality object should throw
        std::shared_ptr<oa::GEvolutionaryAlgorithm_PersonalityTraits> p_pt_ea;
        CHECK_THROWS_AS(
            (p_pt_ea = p_test->getPersonalityTraits<oa::GEvolutionaryAlgorithm_PersonalityTraits>()),
            geneva_exception
        );
    }
#endif /* DEBUG */

    //------------------------------------------------------------------------------

#ifdef DEBUG
    { // Test that retrieval of an EA personality traits object from an individual with SWARM personality throws
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure the personality type is set to SWARM
        CHECK_NOTHROW(p_test->setPersonality(
            std::make_shared<oa::GSwarmAlgorithm_PersonalityTraits>()
        ));

        // Trying to retrieve an EA personality object should throw
        CHECK_THROWS_AS(
            (p_test->getPersonalityTraits<oa::GEvolutionaryAlgorithm_PersonalityTraits>()),
            geneva_exception
        );
    }
#endif /* DEBUG */

    //------------------------------------------------------------------------------

#ifdef DEBUG
    { // Test that retrieval of a personality traits base object from an individual without personality throws
        std::shared_ptr<Gem::Tests::GTestIndividual1> p_test =
            this->clone<Gem::Tests::GTestIndividual1>();

        // Make sure the personality type is set to PERSONALITY_NONE
        CHECK_NOTHROW(p_test->resetPersonality());

        // Trying to retrieve an EA personality object should throw
        std::shared_ptr<GPersonalityTraits> p_pt;
        CHECK_THROWS_AS((p_pt = p_test->getPersonalityTraits()), geneva_exception);
    }
#endif /* DEBUG */

    //------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    Gem::Common::condnotset(
        "GTestIndividual1::specificTestsFailuresExpected_GUnitTests",
        "GEM_TESTING"
    );
#endif                  /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Gem::Tests */
