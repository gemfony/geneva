/**
 * @file GParameterObjectUsagePatterns.cpp
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

// Standard header files go here
#include <iostream>
#include <tuple>

// Boost header files go here
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Geneva header files go here
#include "geneva/individuals/GFunctionIndividual.hpp"
#include "geneva/par/GBooleanCollection.hpp"
#include "geneva/par/GBooleanObject.hpp"
#include "geneva/par/GBooleanObjectCollection.hpp"
#include "geneva/par/GConstrainedDoubleCollection.hpp"
#include "geneva/par/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/par/GConstrainedInt32Object.hpp"
#include "geneva/par/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/par/GDoubleBiGaussAdaptor.hpp"
#include "geneva/par/GDoubleCollection.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "geneva/par/GDoubleObject.hpp"
#include "geneva/par/GDoubleObjectCollection.hpp"
#include "geneva/par/GInt32Collection.hpp"
#include "geneva/par/GInt32FlipAdaptor.hpp"
#include "geneva/par/GInt32GaussAdaptor.hpp"
#include "geneva/par/GInt32Object.hpp"
#include "geneva/par/GInt32ObjectCollection.hpp"
#include "geneva/Go2.hpp" // Includes all of the parameter object types

using namespace Gem::Geneva;

/******************************************************************************/
/**
 * This example wants to demonstrate the basic usage of parameter objects
 */
int main(int argc, char **argv) {
    //===========================================================================
    // Parameter Sets

    { // Conversion of parameter object data to boost::property_tree
        // Create a factory for GFunctionIndividual objects and perform
        // any necessary initial work.
        std::shared_ptr<gind::GFunctionIndividualFactory> gfi_ptr(
            new gind::GFunctionIndividualFactory("./config/GFunctionIndividual.json")
        );

        // Note: This object already contains a parameter object, in
        // addition to those added below.
        std::shared_ptr<gpar::GParameterSet> gfi_test = gfi_ptr->get();

        gfi_test->push_back(
            std::shared_ptr<gpar::GConstrainedDoubleObject>(new gpar::GConstrainedDoubleObject(-7, 17))
        );
        gfi_test->push_back(
            std::shared_ptr<gpar::GConstrainedDoubleObject>(new gpar::GConstrainedDoubleObject(-5, 5))
        );

        // Add some more data
        gfi_test->push_back(std::shared_ptr<gpar::GBooleanObject>(new gpar::GBooleanObject()));
        gfi_test->push_back(std::shared_ptr<gpar::GDoubleObject>(new gpar::GDoubleObject()));
        gfi_test->push_back(
            std::shared_ptr<gpar::GConstrainedDoubleObject>(new gpar::GConstrainedDoubleObject())
        );
        gfi_test->push_back(std::shared_ptr<gpar::GInt32Object>(new gpar::GInt32Object()));
        gfi_test->push_back(
            std::shared_ptr<gpar::GConstrainedInt32Object>(new gpar::GConstrainedInt32Object())
        );

        std::shared_ptr<gpar::GParameterObjectCollection> gpoc_ptr(new gpar::GParameterObjectCollection());
        gpoc_ptr->push_back(std::shared_ptr<gpar::GDoubleObject>(new gpar::GDoubleObject()));
        gpoc_ptr->push_back(std::shared_ptr<gpar::GDoubleObject>(new gpar::GDoubleObject()));
        gpoc_ptr->push_back(std::shared_ptr<gpar::GDoubleObject>(new gpar::GDoubleObject()));
        gpoc_ptr->push_back(
            std::shared_ptr<gpar::GConstrainedDoubleCollection>(
                new gpar::GConstrainedDoubleCollection(5, -10., 10.)
            )
        );

        gfi_test->push_back(gpoc_ptr);

        // Make sure the individual is "clean", i.e. the processed flag is set
        gfi_test->set_processing_status(Gem::Courtier::processingStatus::DO_PROCESS);
        gfi_test->process();

        boost::property_tree::ptree ptr;
        gfi_test->toPropertyTree(ptr);

#if BOOST_VERSION > 105500
        boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#else
        boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#endif /* BOOST_VERSION */
        boost::property_tree::write_xml("result.xml", ptr, std::locale(), settings);

        // Now run this program and see the file "result.xml" for the output
    }

    //===========================================================================
    // Parameter Types

    { // Usage patterns for the GDoubleObject class
        std::cout << "GDoubleObject:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GDoubleObject o1;         // Default construction
        gpar::GDoubleObject o2(o1);     // Copy construction
        gpar::GDoubleObject o3(2.);     // Initialization by value
        gpar::GDoubleObject o4(0., 2.); // Random initialization in a given range
        std::shared_ptr<gpar::GDoubleObject> p(
            new gpar::GDoubleObject(0., 2.)
        ); // Construction and access frequently happens through smart pointers

        //-----------------------------------------------------
        // Assignment, value setting and retrieval
        o1 = 1.; // Assigning and setting a value
        o2.setValue(2.);
        o4 = o1;                              // Assignment to another object
        std::cout << o4.value() << '\n'; // Value retrieval

        //-----------------------------------------------------
        // Boundaries
        std::cout << o4.getLowerInitBoundary() << '\n'; // Retrieval of lower init boundary
        std::cout << o4.getUpperInitBoundary() << '\n'; // Retrieval of upper init boundary

        //-----------------------------------------------------
        // Assignment of an adaptor
        double sigma = 0.1;                   // "step width" of gauss mutation
        double sigmaSigma = 0.8;              // adaption of sigma
        double minSigma = 0.;
        double maxSigma = 0.5; // allowed value range of sigma
        double adProb =
            0.05; // 5% probability for the adaption of this object when adaptor is called
        std::shared_ptr<gpar::GDoubleGaussAdaptor> gdga_ptr(
            new gpar::GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma)
        );
        gdga_ptr->setAdaptionProbability(adProb);
        p->addAdaptor(gdga_ptr);
    }

    { // Usage patterns for the GConstrainedDoubleObject class
        std::cout << "GConstrainedDoubleObject:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GConstrainedDoubleObject o1;             // Default construction
        gpar::GConstrainedDoubleObject o2(o1);         // Copy construction
        gpar::GConstrainedDoubleObject o3(2.);         // Initialization by value
        gpar::GConstrainedDoubleObject o4(0., 2.);     // Initialization of value boundaries
        gpar::GConstrainedDoubleObject o5(1., 0., 2.); // Initialization with value and value boundaries
        std::shared_ptr<gpar::GConstrainedDoubleObject> p(
            new gpar::GConstrainedDoubleObject(0., 2.)
        ); // Construction and access frequently happens through smart pointers

        //-----------------------------------------------------
        // Assignment, value setting and retrieval
        o1 = 1.; // Assigning a value
        o2.setValue(1.5);
        o5 = o1;                                                   // Assignment of another object
        std::cout << o4.value() << " " << o5.value() << '\n'; // Value retrieval

        //-----------------------------------------------------
        // Boundaries
        std::cout << o4.getLowerBoundary() << '\n'; // Retrieval of lower value boundary
        std::cout << o4.getUpperBoundary() << '\n'; // Retrieval of upper value boundary

        //-----------------------------------------------------
        // Assignment of an adaptor (same as for GDoubleObject)
        double sigma = 0.1;                   // "step width" of gauss mutation
        double sigmaSigma = 0.8;              // adaption of sigma
        double minSigma = 0.;
        double maxSigma = 0.5; // allowed value range of sigma
        double adProb =
            0.05; // 5% probability for the adaption of this object when adaptor is called
        std::shared_ptr<gpar::GDoubleGaussAdaptor> gdga_ptr(
            new gpar::GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma)
        );
        gdga_ptr->setAdaptionProbability(adProb);
        p->addAdaptor(gdga_ptr);
    }

    { // Usage patterns for the GDoubleObjectCollection class
        std::cout << "GDoubleObjectCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GDoubleObjectCollection c1;     // Default constructor
        gpar::GDoubleObjectCollection c2(c1); // Copy construction
        std::shared_ptr<gpar::GDoubleObjectCollection> p_c3(
            new gpar::GDoubleObjectCollection(c1)
        ); // Copy construction inside of smart pointer
        // Note: Copy construction will create deep copies
        // of all objects stored in c1

        //-----------------------------------------------------
        // Filling with objects
        for(std::size_t i = 0; i < 10; i++) {
            // Create a smart pointer wrapping a GDoubleObject
            std::shared_ptr<gpar::GDoubleObject> p(new gpar::GDoubleObject());
            // Configure GDoubleObject as required. E.g., add adaptors
            // ...
            // Add to the collection
            c1.push_back(p);
        }

        // Note: No adaptor is added to the collection itself, only
        // to the objects contained in it.

        //-----------------------------------------------------
        // Assignment through load . Note: This will create
        // deep copies of all objects stored in c1
        c2.load(c1);
        p_c3->load(c1);
        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < 10; i++) {
            std::cout << p_c3->at(i)->value() << '\n';
            std::cout << c1[i]->value() << '\n';
        }

        // Note: The iterator points to a smart pointer, so in order to
        // call a function on the parameter objects we first need to
        // dereference the iterator, then the smart pointer
        gpar::GDoubleObjectCollection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << (*it)->value() << '\n';
        }
    }

    { // Usage patterns for the GConstrainedDoubleObjectCollection class
        std::cout << "GConstrainedDoubleObjectCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GConstrainedDoubleObjectCollection c1;     // Default constructor
        gpar::GConstrainedDoubleObjectCollection c2(c1); // Copy construction
        std::shared_ptr<gpar::GConstrainedDoubleObjectCollection> p_c3(
            new gpar::GConstrainedDoubleObjectCollection(c1)
        ); // Copy construction inside of smart pointer
        // Note: Copy construction will create deep copies
        // of all objects stored in c1

        //-----------------------------------------------------
        // Filling with objects
        for(std::size_t i = 0; i < 10; i++) {
            // Create a smart pointer wrapping a GDoubleObject
            std::shared_ptr<gpar::GConstrainedDoubleObject> p(new gpar::GConstrainedDoubleObject(-10., 10.));
            // Configure GConstrainedDoubleObject as required. E.g., add adaptors
            // ...
            // Add to the collection
            c1.push_back(p);
        }

        // Note: No adaptor is added to the collection itself, only
        // to the objects contained in it.

        //-----------------------------------------------------
        // Assignment through load(). Note: This will create
        // deep copies of all objects stored in c1
        c2.load(c1);
        p_c3->load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < 10; i++) {
            std::cout << p_c3->at(i)->value() << '\n';
            std::cout << c1[i]->value() << '\n';
        }

        // Note: The iterator points to a smart pointer, so in order to
        // call a function on the parameter objects we first need to
        // dereference the iterator, then the smart pointer
        gpar::GConstrainedDoubleObjectCollection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << (*it)->value() << '\n';
        }
    }

    { // Usage patterns for the GDoubleCollection class
        std::cout << "GDoubleCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GDoubleCollection c1;     // Default construction
        gpar::GDoubleCollection c2(c1); // Copy construction
        // Copy construction inside of smart pointer
        std::shared_ptr<gpar::GDoubleCollection> p_c3(new gpar::GDoubleCollection(c1));
        // 100 double values, randomly initialized in the range [-3.,3[
        gpar::GDoubleCollection c4(100, -3., 3.);

        //-----------------------------------------------------
        // Filling with objects
        for(double d = 0.; d < 100.; d += 1.) {
            c1.push_back(d);
        }

        //-----------------------------------------------------
        // Adding an adaptor
        double sigma = 0.1;                   // "step width" of gauss mutation
        double sigmaSigma = 0.8;              // adaption of sigma
        double minSigma = 0.;
        double maxSigma = 0.5; // allowed value range of sigma
        // 5% probability for the adaption of this object when adaptor is called
        double adProb = 0.05;
        std::shared_ptr<gpar::GDoubleGaussAdaptor> gdga_ptr(
            new gpar::GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma)
        );
        gdga_ptr->setAdaptionProbability(adProb);
        c1.addAdaptor(gdga_ptr);

        //-----------------------------------------------------
        // Assignment through load() . Note: This will also create
        // deep copies of the adaptor
        c2.load(c1);
        p_c3->load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < c1.size(); i++) {
            std::cout << c1[i] << '\n';
            std::cout << c1.at(i) << '\n';
        }
        gpar::GDoubleCollection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << *it << '\n';
        }
        //-----------------------------------------------------
    }

    { // Usage patterns for the GConstrainedDoubleCollection class
        std::cout << "GConstrainedDoubleCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        // Initialization with 100 variables and constraint [-10, 10[
        gpar::GConstrainedDoubleCollection c1(100, -10, 200.);
        gpar::GConstrainedDoubleCollection c2(c1); // Copy construction

        // Note -- we do not currently fill in additional data items. This
        // class is not yet at its final stage.

        //-----------------------------------------------------
        // Adding an adaptor
        double sigma = 0.1;                   // "step width" of gauss mutation
        double sigmaSigma = 0.8;              // adaption of sigma
        double minSigma = 0.;
        double maxSigma = 0.5; // allowed value range of sigma
        // 5% probability for the adaption of this object when adaptor is called
        double adProb = 0.05;
        std::shared_ptr<gpar::GDoubleGaussAdaptor> gdga_ptr(
            new gpar::GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma)
        );
        gdga_ptr->setAdaptionProbability(adProb);
        c1.addAdaptor(gdga_ptr);

        //-----------------------------------------------------
        // Assignment through load() . Note: This will also create
        // deep copies of the adaptor
        c2.load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection
        // Note: We currently recommend not to use the subscript and at()
        // operators or iterators
        for(std::size_t i = 0; i < c1.size(); i++) {
            c1.setValue(i, static_cast<double>(i));
            std::cout << c1.value(i) << '\n';
        }
        //-----------------------------------------------------
    }

    { // Usage patterns for the GInt32Object class
        std::cout << "GInt32Object:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GInt32Object o1;       // Default construction
        gpar::GInt32Object o2(o1);   // Copy construction
        gpar::GInt32Object o3(2);    // Initialization by value
        gpar::GInt32Object o4(0, 2); // Random initialization in a given range
        std::shared_ptr<gpar::GInt32Object> p_o5(
            new gpar::GInt32Object(0, 2)
        ); // Construction and access frequently happens through smart pointers

        //-----------------------------------------------------
        // Assignment, value setting and retrieval
        o1 = 1; // Assigning and setting a value
        o2.setValue(2);
        o4 = o1;                              // Assignment of another object
        std::cout << o4.value() << '\n'; // Value retrieval

        //-----------------------------------------------------
        // Boundaries
        std::cout << o4.getLowerInitBoundary() << '\n'; // Retrieval of lower init boundary
        std::cout << o4.getUpperInitBoundary() << '\n'; // Retrieval of upper init boundary

        //-----------------------------------------------------
        // Assignment of an adaptor
        std::shared_ptr<gpar::GInt32FlipAdaptor> ifa_ptr(new gpar::GInt32FlipAdaptor());
        ifa_ptr->setAdaptionProbability(0.05); // 5% probability
        p_o5->addAdaptor(ifa_ptr);
    }

    { // Usage patterns for the GConstrainedInt32Object class
        std::cout << "GConstrainedInt32Object:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GConstrainedInt32Object o1;        // Default construction
        gpar::GConstrainedInt32Object o2(o1);    // Copy construction
        gpar::GConstrainedInt32Object o3(2);     // Initialization by value
        gpar::GConstrainedInt32Object o4(0, 10); // Initialization of allowed initialization range
        gpar::GConstrainedInt32Object o5(
            1,
            0,
            10
        ); // Initialization with value and allowed initialization range
        std::shared_ptr<gpar::GConstrainedInt32Object> p_o6(
            new gpar::GConstrainedInt32Object(0, 2)
        ); // Construction and access frequently happens through smart pointers

        //-----------------------------------------------------
        // Assignment, value setting and retrieval
        o1 = 1; // Assigning and setting a value
        o2.setValue(2);
        o4 = o1;                              // Assignment of another object
        std::cout << o4.value() << '\n'; // Value retrieval

        //-----------------------------------------------------
        // Boundaries
        std::cout << o4.getLowerBoundary() << '\n'; // Retrieval of lower init boundary
        std::cout << o4.getUpperBoundary() << '\n'; // Retrieval of upper init boundary

        //-----------------------------------------------------
        // Assignment of an adaptor
        std::shared_ptr<gpar::GInt32FlipAdaptor> ifa_ptr(new gpar::GInt32FlipAdaptor());
        ifa_ptr->setAdaptionProbability(0.05); // 5% probability
        p_o6->addAdaptor(ifa_ptr);
    }

    { // Usage patterns for the GInt32ObjectCollection class
        std::cout << "GInt32ObjectCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GInt32ObjectCollection c1;     // Default constructor
        gpar::GInt32ObjectCollection c2(c1); // Copy construction
        // Copy construction inside of smart pointer
        std::shared_ptr<gpar::GInt32ObjectCollection> p_c3(new gpar::GInt32ObjectCollection(c1));
        // Note: Copy construction will create deep copies
        // of all objects stored in c1

        //-----------------------------------------------------
        // Filling with objects
        for(std::size_t i = 0; i < 10; i++) {
            // Create a smart pointer wrapping a GInt32Object
            std::shared_ptr<gpar::GInt32Object> p(new gpar::GInt32Object());
            // Configure GInt32Object as required. E.g., add adaptors
            // ...
            // Add to the collection
            c1.push_back(p);
        }

        // Note: No adaptor is added to the collection itself, only
        // to the objects contained in it.

        //-----------------------------------------------------
        // Assignment through load() . Note: This will create
        // deep copies of all objects stored in c1
        c2.load(c1);
        p_c3->load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < 10; i++) {
            std::cout << p_c3->at(i)->value() << '\n';
            std::cout << c1[i]->value() << '\n';
        }

        // Note: The iterator points to a smart pointer, so in order to
        // call a function on the parameter objects we first need to
        // dereference the iterator, then the smart pointer
        gpar::GInt32ObjectCollection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << (*it)->value() << '\n';
        }
    }

    { // Usage patterns for the GConstrainedInt32ObjectCollection class
        std::cout << "GConstrainedInt32ObjectCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GConstrainedInt32ObjectCollection c1;     // Default constructor
        gpar::GConstrainedInt32ObjectCollection c2(c1); // Copy construction
        // Copy construction inside of smart pointer
        std::shared_ptr<gpar::GConstrainedInt32ObjectCollection> p_c3(
            new gpar::GConstrainedInt32ObjectCollection(c1)
        );
        // Note: Copy construction will create deep copies
        // of all objects stored in c1

        //-----------------------------------------------------
        // Filling with objects
        for(std::size_t i = 0; i < 10; i++) {
            // Create a smart pointer wrapping a GConstrainedInt32Object
            std::shared_ptr<gpar::GConstrainedInt32Object> p(new gpar::GConstrainedInt32Object(-10, 10));
            // Configure GConstrainedInt32Object as required. E.g., add adaptors
            // ...
            // Add to the collection
            c1.push_back(p);
        }

        // Note: No adaptor is added to the collection itself, only
        // to the objects contained in it.

        //-----------------------------------------------------
        // Assignment through load() . Note: This will create
        // deep copies of all objects stored in c1
        c2.load(c1);
        p_c3->load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < 10; i++) {
            std::cout << p_c3->at(i)->value() << '\n';
            std::cout << c1[i]->value() << '\n';
        }

        // Note: The iterator points to a smart pointer, so in order to
        // call a function on the parameter objects we first need to
        // dereference the iterator, then the smart pointer
        gpar::GConstrainedInt32ObjectCollection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << (*it)->value() << '\n';
        }
    }

    { // Usage patterns for the GInt32Collection class
        std::cout << "GInt32Collection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GInt32Collection c1;     // Default construction
        gpar::GInt32Collection c2(c1); // Copy construction
        // Copy construction inside of smart pointer
        std::shared_ptr<gpar::GInt32Collection> p_c3(new gpar::GInt32Collection(c1));
        // 100 std::int32_t values, with an initialization range of [-3,3]
        gpar::GInt32Collection c4(100, -3, 3);

        //-----------------------------------------------------
        // Filling with data
        for(std::int32_t i = 0; i < 100; i++) {
            c1.push_back(i);
        }

        //-----------------------------------------------------
        // Adding an adaptor
        std::shared_ptr<gpar::GInt32FlipAdaptor> ifa_ptr(new gpar::GInt32FlipAdaptor());
        ifa_ptr->setAdaptionProbability(0.05); // 5% probability
        c1.addAdaptor(ifa_ptr);

        //-----------------------------------------------------
        // Assignment through load() . Note: This will also create
        // deep copies of the adaptor
        c2.load(c1);
        p_c3->load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < c1.size(); i++) {
            std::cout << c1[i] << '\n';
            std::cout << c1.at(i) << '\n';
        }
        gpar::GInt32Collection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << *it << '\n';
        }
        //-----------------------------------------------------
    }

    { // Usage patterns for the GBooleanObject class
        std::cout << "GBooleanObject:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GBooleanObject o1;       // Default construction
        gpar::GBooleanObject o2(o1);   // Copy construction
        gpar::GBooleanObject o3(true); // Initialization by value
        // Construction and access frequently happens through smart pointers
        std::shared_ptr<gpar::GBooleanObject> p(new gpar::GBooleanObject(true));

        //-----------------------------------------------------
        // Assignment, value setting and retrieval
        o1 = false; // Assigning and setting a value
        o2.setValue(false);
        o3 = o1; // Assignment of another object
        // Value retrieval and value emission
        std::cout << (o3.value() ? true : false) << '\n';

        //-----------------------------------------------------
        // Assignment of an adaptor
        std::shared_ptr<gpar::GBooleanAdaptor> bad_ptr(new gpar::GBooleanAdaptor());
        bad_ptr->setAdaptionProbability(0.05); // 5% adaption probability
        p->addAdaptor(bad_ptr);
    }

    { // Usage patterns for the GBooleanObjectCollection class
        std::cout << "GBooleanObjectCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GBooleanObjectCollection c1;     // Default constructor
        gpar::GBooleanObjectCollection c2(c1); // Copy construction
        std::shared_ptr<gpar::GBooleanObjectCollection> p_c3(
            new gpar::GBooleanObjectCollection(c1)
        ); // Copy construction inside of smart pointer
        // Note: Copy construction will create deep copies
        // of all objects stored in c1

        //-----------------------------------------------------
        // Filling with objects
        for(std::size_t i = 0; i < 10; i++) {
            // Create a smart pointer wrapping a GBooleanObject
            std::shared_ptr<gpar::GBooleanObject> p(new gpar::GBooleanObject());
            // Configure GBooleanObject as required. E.g., add adaptors
            // ...
            // Add to the collection
            c1.push_back(p);
        }

        // Note: No adaptor is added to the collection itself, only
        // to the objects contained in it.

        //-----------------------------------------------------
        // Assignment through load() . Note: This will create
        // deep copies of all objects stored in c1
        c2.load(c1);
        p_c3->load(c1);
        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < 10; i++) {
            std::cout << p_c3->at(i)->value() << '\n';
            std::cout << c1[i]->value() << '\n';
        }

        // Note: The iterator points to a smart pointer, so in order to
        // call a function on the parameter objects we first need to
        // dereference the iterator, then the smart pointer
        gpar::GBooleanObjectCollection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << (*it)->value() << '\n';
        }
    }

    { // Usage patterns for the GBooleanCollection class
        std::cout << "GBooleanCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GBooleanCollection c1;      // Default construction
        gpar::GBooleanCollection c2(c1);  // Copy construction
        gpar::GBooleanCollection c3(100); // Initialization with 100 random booleans
        // Initialization with 100 random booleans, of which 25% have a true value
        gpar::GBooleanCollection c4(100, 0.25);
        // Copy construction inside of smart pointer
        std::shared_ptr<gpar::GBooleanCollection> p_c5(new gpar::GBooleanCollection(c1));

        //-----------------------------------------------------
        // Filling with data
        for(std::size_t i = 0; i < 100; i++) {
            c1.push_back(i % 2 == 0 ? true : false);
        }

        //-----------------------------------------------------
        // Adding an adaptor
        std::shared_ptr<gpar::GBooleanAdaptor> bad_ptr(new gpar::GBooleanAdaptor());
        bad_ptr->setAdaptionProbability(0.05); // 5% adaption probability
        p_c5->addAdaptor(bad_ptr);

        //-----------------------------------------------------
        // Assignment through load() . Note: This will also create
        // deep copies of the adaptor
        c2.load(c1);
        p_c5->load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection
        for(std::size_t i = 0; i < c1.size(); i++) {
            std::cout << (c1[i] ? "true" : "false") << '\n';
            std::cout << (c1.at(i) ? "true" : "false") << '\n';
        }
        gpar::GBooleanCollection::iterator it;
        for(it = c1.begin(); it != c1.end(); ++it) {
            std::cout << (*it ? "true" : "false") << '\n';
        }
        //-----------------------------------------------------
    }

    { // Usage patterns for the GParameterObjectCollection class
        std::cout << "GParameterObjectCollection:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GParameterObjectCollection c1;     // Default constructor
        gpar::GParameterObjectCollection c2(c1); // Copy construction
        std::shared_ptr<gpar::GParameterObjectCollection> p_c3(
            new gpar::GParameterObjectCollection(c1)
        ); // Copy construction inside of smart pointer
        // Note: Copy construction will create deep copies
        // of all objects stored in c1

        //-----------------------------------------------------
        // Filling with objects. Note that they may have
        // different types, but must all be derived from
        // GParameterBase

        // Create a smart pointer wrapping a GDoubleObject
        std::shared_ptr<gpar::GDoubleObject> p_d(new gpar::GDoubleObject());
        // Configure GDoubleObject as required. E.g., add adaptors
        // ...
        // Add to the collection
        c1.push_back(p_d);

        // Create a smart pointer wrapping a GInt32Object
        std::shared_ptr<gpar::GInt32Object> p_i(new gpar::GInt32Object());
        // Configure GInt32Object as required. E.g., add adaptors
        // ...
        // Add to the collection
        c1.push_back(p_i);

        // Create another GParameterObjectCollection object.
        // As it is derived from GParameterBase, we can store it
        // in GParameterObjectCollection objects and create
        // tree-like structures in this way
        std::shared_ptr<gpar::GParameterObjectCollection> p_child(new gpar::GParameterObjectCollection());
        c1.push_back(p_child);

        // Note: No adaptor is added to the collection itself, only
        // to the objects contained in it (if they support this).

        //-----------------------------------------------------
        // Assignment through load() . Note: This will create
        // deep copies of all objects stored in c1
        c2.load(c1);
        p_c3->load(c1);

        //-----------------------------------------------------
        // Access to parameter objects in the collection

        // Direct conversion, if we know the target type
        std::shared_ptr<gpar::GDoubleObject> p_d2 = c1.at<gpar::GDoubleObject>(0);

        // Filtered range view -- will return all GDoubleObject items
        // stored on this level (does not recurse into nested collections).
        for(auto p_conv : c1.filteredView<gpar::GDoubleObject>()) {
            std::cout << p_conv->value() << '\n';
        }
        //-----------------------------------------------------
    }

    //===========================================================================
    // Adaptors

    { // GDoubleGaussAdaptor
        std::cout << "GDoubleGaussAdaptor:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GDoubleGaussAdaptor a1;     // Default construction
        gpar::GDoubleGaussAdaptor a2(a1); // Copy construction

        double adProb = 0.05;         // A 5% probability that adaption actually takes place
        gpar::GDoubleGaussAdaptor a3(0.05); // Construction with adaption probability

        double sigma = 0.2;
        double sigmaSigma = 0.1;
        double minSigma = 0.;
        double maxSigma = 1.;
        gpar::GDoubleGaussAdaptor a4(
            sigma,
            sigmaSigma,
            minSigma,
            maxSigma
        ); //Construction with specific mutation parameters

        gpar::GDoubleGaussAdaptor a5(
            sigma,
            sigmaSigma,
            minSigma,
            maxSigma,
            adProb
        ); //Construction with specific mutation parameters

        std::shared_ptr<gpar::GDoubleGaussAdaptor> p_a6(
            new gpar::GDoubleGaussAdaptor()
        ); // Construction inside of a smart pointer

        //-----------------------------------------------------
        // Assignment
        a3 = a1;
        *p_a6 = a1;

        //-----------------------------------------------------
        // Setting and retrieval of specific configuration parameters
        a1.setSigmaRange(minSigma, maxSigma);
        std::tuple<double, double> t = a1.getSigmaRange();
        std::cout << std::get<0>(t) << " " << std::get<1>(t) << '\n';

        a1.setSigma(sigma);
        double sigma2 = a1.getSigma();

        a1.setSigmaAdaptionRate(sigmaSigma);
        double adaptionRate = a1.getSigmaAdaptionRate();

        a1.setAll(sigma, sigmaSigma, minSigma, maxSigma);

        //-----------------------------------------------------
        // Parameters common to all adaptors
        a1.setAdaptionProbability(adProb);
        double adProb2 = a1.getAdaptionProbability();

        std::uint32_t adaptionThreshold = 1;
        a1.setAdaptionThreshold(adaptionThreshold);
        adaptionThreshold = a1.getAdaptionThreshold();

        a1.setAdaptionMode(adaptionMode::ALWAYS); // Always adapt, irrespective of probability
        a2.setAdaptionMode(
            adaptionMode::WITHPROBABILITY
        );                                       // Adapt according to the adaption probability
        a3.setAdaptionMode(adaptionMode::NEVER); // Temporarily disable the adaptor
        adaptionMode am = a1.getAdaptionMode();
    }

    { // GDoubleBiGaussAdaptor
        std::cout << "GDoubleBiGaussAdaptor:" << '\n';

        //-----------------------------------------------------
        // Construction
        gpar::GDoubleBiGaussAdaptor a1;     // Default construction
        gpar::GDoubleBiGaussAdaptor a2(a1); // Copy construction

        double adProb = 0.05;           // A 5% probability that adaption actually takes place
        gpar::GDoubleBiGaussAdaptor a3(0.05); // Construction with adaption probability

        // Construction inside of a smart pointer
        std::shared_ptr<gpar::GDoubleBiGaussAdaptor> p_a4(new gpar::GDoubleBiGaussAdaptor());

        //-----------------------------------------------------
        // Assignment
        a3.load(a1);
        p_a4->load(a1);

        //-----------------------------------------------------
        // Setting and retrieval of specific configuration parameters

        // sigma1 and sigma2 may differ
        a1.setUseSymmetricSigmas(false);
        bool useSymmetricSigmas = a1.getUseSymmetricSigmas();

        // Set/get sigma1 and sigma2
        a1.setSigma1(0.1);
        a1.setSigma2(0.2);
        double sigma1 = a1.getSigma1();
        double sigma2 = a1.getSigma2();

        // Set/get the allowed value range of sigma1 and sigma2
        a1.setSigma1Range(0.001, 2.);
        a1.setSigma2Range(0.001, 2.);
        std::tuple<double, double> sigma1Range = a1.getSigma1Range();
        std::tuple<double, double> sigma2Range = a1.getSigma2Range();

        // Set/get the adaption rate of sigma1 and sigma2
        a1.setSigma1AdaptionRate(0.8);
        a1.setSigma2AdaptionRate(0.8);
        double sigma1AdaptionRate = a1.getSigma1AdaptionRate();
        double sigma2AdaptionRate = a1.getSigma2AdaptionRate();

        // Set all sigma1 and sigma2 parameters at once. Note: We use
        // the lower/upper boundaries extracted above.
        a1.setAllSigma1(
            sigma1,
            sigma1AdaptionRate,
            std::get<0>(sigma1Range),
            std::get<1>(sigma1Range)
        );
        a1.setAllSigma2(
            sigma2,
            sigma2AdaptionRate,
            std::get<0>(sigma2Range),
            std::get<1>(sigma2Range)
        );

        // Set/get the lower and upper boundaries of delta
        a1.setDeltaRange(0., 5.);
        std::tuple<double, double> deltaRange = a1.getDeltaRange();

        // Set the initial distance between both peaks
        // and retieve the current value
        a1.setDelta(1.5);
        double delta = a1.getDelta();

        // Set/get the adaption rate of delta
        a1.setDeltaAdaptionRate(0.8);
        double deltaAdaptionRate = a1.getDeltaAdaptionRate();

        // Set all delta parameters at once. Note: We use the
        // lower and upper boundaries that were extracted above
        a1.setAllDelta(delta, deltaAdaptionRate, std::get<0>(deltaRange), std::get<1>(deltaRange));

        //-----------------------------------------------------
        // Parameters common to all adaptors
        a1.setAdaptionProbability(adProb);
        double adProb2 = a1.getAdaptionProbability();

        std::uint32_t adaptionThreshold = 1;
        a1.setAdaptionThreshold(adaptionThreshold);
        adaptionThreshold = a1.getAdaptionThreshold();

        // Always adapt, irrespective of probability
        a1.setAdaptionMode(adaptionMode::ALWAYS);
        // Adapt according to the adaption probability
        a2.setAdaptionMode(adaptionMode::WITHPROBABILITY);
        // Temporarily disable the adaptor
        a3.setAdaptionMode(adaptionMode::NEVER);
        adaptionMode am = a1.getAdaptionMode();

        //-----------------------------------------------------
    }

    { // GInt32GaussAdaptor
        //-----------------------------------------------------
        // Construction
        gpar::GInt32GaussAdaptor a1;     // Default construction
        gpar::GInt32GaussAdaptor a2(a1); // Copy construction

        double adProb = 0.05;        // A 5% probability that adaption actually takes place
        gpar::GInt32GaussAdaptor a3(0.05); // Construction with adaption probability

        double sigma = 0.2;
        double sigmaSigma = 0.1;
        double minSigma = 0.;
        double maxSigma = 1.;
        gpar::GInt32GaussAdaptor a4(
            sigma,
            sigmaSigma,
            minSigma,
            maxSigma
        ); //Construction with specific mutation parameters

        gpar::GInt32GaussAdaptor a5(
            sigma,
            sigmaSigma,
            minSigma,
            maxSigma,
            adProb
        ); //Construction with specific mutation parameters

        std::shared_ptr<gpar::GInt32GaussAdaptor> p_a6(
            new gpar::GInt32GaussAdaptor()
        ); // Construction inside of a smart pointer

        //-----------------------------------------------------
        // Assignment
        a3 = a1;
        *p_a6 = a1;

        //-----------------------------------------------------
        // Setting and retrieval of specific configuration parameters
        a1.setSigmaRange(minSigma, maxSigma);
        std::tuple<double, double> t = a1.getSigmaRange();
        std::cout << std::get<0>(t) << " " << std::get<1>(t) << '\n';

        a1.setSigma(sigma);
        double sigma2 = a1.getSigma();

        a1.setSigmaAdaptionRate(sigmaSigma);
        double adaptionRate = a1.getSigmaAdaptionRate();

        a1.setAll(sigma, sigmaSigma, minSigma, maxSigma);

        //-----------------------------------------------------
        // Parameters common to all adaptors
        a1.setAdaptionProbability(adProb);
        double adProb2 = a1.getAdaptionProbability();

        std::uint32_t adaptionThreshold = 1;
        a1.setAdaptionThreshold(adaptionThreshold);
        adaptionThreshold = a1.getAdaptionThreshold();

        a1.setAdaptionMode(adaptionMode::ALWAYS); // Always adapt, irrespective of probability
        a2.setAdaptionMode(
            adaptionMode::WITHPROBABILITY
        );                                       // Adapt according to the adaption probability
        a3.setAdaptionMode(adaptionMode::NEVER); // Temporarily disable the adaptor
        adaptionMode am = a1.getAdaptionMode();

        //-----------------------------------------------------
    }

    { // GInt32FlipAdaptor
        //-----------------------------------------------------
        // Construction
        gpar::GInt32FlipAdaptor a1;     // Default construction
        gpar::GInt32FlipAdaptor a2(a1); // Copy construction

        double adProb = 0.05;       // A 5% probability that adaption actually takes place
        gpar::GInt32FlipAdaptor a3(0.05); // Construction with adaption probability

        std::shared_ptr<gpar::GInt32FlipAdaptor> p_a4(
            new gpar::GInt32FlipAdaptor()
        ); // Construction inside of a smart pointer

        //-----------------------------------------------------
        // Assignment
        a3 = a1;
        *p_a4 = a1;

        //-----------------------------------------------------
        // Parameters common to all adaptors
        a1.setAdaptionProbability(adProb);
        double adProb2 = a1.getAdaptionProbability();

        std::uint32_t adaptionThreshold = 1;
        a1.setAdaptionThreshold(adaptionThreshold);
        adaptionThreshold = a1.getAdaptionThreshold();

        a1.setAdaptionMode(adaptionMode::ALWAYS); // Always adapt, irrespective of probability
        a2.setAdaptionMode(
            adaptionMode::WITHPROBABILITY
        );                                       // Adapt according to the adaption probability
        a3.setAdaptionMode(adaptionMode::NEVER); // Temporarily disable the adaptor
        adaptionMode am = a1.getAdaptionMode();

        //-----------------------------------------------------
    }

    { // GBooleanAdaptor
        //-----------------------------------------------------
        // Construction
        gpar::GBooleanAdaptor a1;     // Default construction
        gpar::GBooleanAdaptor a2(a1); // Copy construction

        double adProb = 0.05;     // A 5% probability that adaption actually takes place
        gpar::GBooleanAdaptor a3(0.05); // Construction with adaption probability

        std::shared_ptr<gpar::GBooleanAdaptor> p_a4(
            new gpar::GBooleanAdaptor()
        ); // Construction inside of a smart pointer

        //-----------------------------------------------------
        // Assignment
        a3 = a1;
        *p_a4 = a1;

        //-----------------------------------------------------
        // Parameters common to all adaptors
        a1.setAdaptionProbability(adProb);
        double adProb2 = a1.getAdaptionProbability();

        std::uint32_t adaptionThreshold = 1;
        a1.setAdaptionThreshold(adaptionThreshold);
        adaptionThreshold = a1.getAdaptionThreshold();

        a1.setAdaptionMode(adaptionMode::ALWAYS); // Always adapt, irrespective of probability
        a2.setAdaptionMode(
            adaptionMode::WITHPROBABILITY
        );                                       // Adapt according to the adaption probability
        a3.setAdaptionMode(adaptionMode::NEVER); // Temporarily disable the adaptor
        adaptionMode am = a1.getAdaptionMode();

        //-----------------------------------------------------
    }

    //===========================================================================

    return 0;
}
