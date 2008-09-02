/**
 * @file GBitFlipAdaptor_test.cpp
 *
 * This test checks all public member functions of the GBitFlipAdaptor adaptor
 * class. In addition, it attempts to check parent classes, in particular the
 * GObject class. You should run this test both in DEBUG and in RELEASE mode,
 * as some functions work differently in this case.
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

// Standard header files go here

// Boost header files go here

#define BOOST_TEST_MODULE GObject_test
#include <boost/test/unit_test.hpp>


// Geneva header files go here
#include "GObject.hpp"
#include "GBitFlipAdaptor.hpp"

using namespace Gem;
using namespace Gem::GenEvA;

const std::string ADAPTORNAME="GBitFlipAdaptor";
const std::string ADAPTORNAME2="GBitFlipAdaptor2";
const std::string ADAPTORNAME3="GBitFlipAdaptor3";

// This test checks as much as possible of the functionality
// provided by the parent class GObject, plus some base functionality
// of the GBitFlipAdaptor class, such as the "named" and the copy
// constructors.
BOOST_AUTO_TEST_CASE( gobject_test )
{
	GBitFlipAdaptor *gbfa=new GBitFlipAdaptor(ADAPTORNAME); // Construction by name
	GBitFlipAdaptor *gbfa2=new GBitFlipAdaptor(*gbfa); // Copy construction

	// Getting and setting the name
	BOOST_CHECK(gbfa->name() == ADAPTORNAME);
	BOOST_CHECK(gbfa->name() == gbfa2->name());

	gbfa2->setName(ADAPTORNAME2);
	BOOST_CHECK(gbfa->name() == ADAPTORNAME);
	BOOST_CHECK(gbfa->name() != gbfa2->name());
	BOOST_CHECK(gbfa2->name() == ADAPTORNAME2);

	// Getting and setting the serialization mode
	BOOST_CHECK(gbfa->getSerializationMode() == TEXTSERIALIZATION); // The default mode
	gbfa->setSerializationMode(TEXTSERIALIZATION);
	BOOST_CHECK(gbfa->getSerializationMode() == TEXTSERIALIZATION);
	gbfa->setSerializationMode(XMLSERIALIZATION);
	BOOST_CHECK(gbfa->getSerializationMode() == XMLSERIALIZATION);
	gbfa->setSerializationMode(BINARYSERIALIZATION);
	BOOST_CHECK(gbfa->getSerializationMode() == BINARYSERIALIZATION);
	BOOST_CHECK(gbfa->getSerializationMode() != gbfa2->getSerializationMode());
	gbfa->setSerializationMode(DEFAULTSERIALIZATION);
	BOOST_CHECK(gbfa->getSerializationMode() == DEFAULTSERIALIZATION);
	BOOST_CHECK(gbfa->getSerializationMode() != gbfa2->getSerializationMode());

	// Assigning the object
	*gbfa2 = *gbfa;
	BOOST_CHECK(gbfa->name() == ADAPTORNAME);
	BOOST_CHECK(gbfa->name() == gbfa2->name());
	BOOST_CHECK(gbfa->getSerializationMode() == gbfa2->getSerializationMode());

	// Changing the aspects again
	gbfa->setSerializationMode(TEXTSERIALIZATION);
	gbfa->setName(ADAPTORNAME2);
	BOOST_CHECK(gbfa->name() != gbfa2->name());
	BOOST_CHECK(gbfa->getSerializationMode() != gbfa2->getSerializationMode());

	// Cloning should create an independent object
	GBitFlipAdaptor *gbfa3 = dynamic_cast<GBitFlipAdaptor *>(gbfa2->clone());
	if(!gbfa3) throw;
	BOOST_CHECK(gbfa2->name() == gbfa3->name());
	BOOST_CHECK(gbfa2->getSerializationMode() == gbfa3->getSerializationMode());
	gbfa3->setSerializationMode(BINARYSERIALIZATION);
	gbfa3->setName(ADAPTORNAME3);
	BOOST_CHECK(gbfa2->name() != gbfa3->name());
	BOOST_CHECK(gbfa2->getSerializationMode() != gbfa3->getSerializationMode());

	gbfa2->load(gbfa3);
	BOOST_CHECK(gbfa2->name() == gbfa3->name());
	BOOST_CHECK(gbfa2->getSerializationMode() == gbfa3->getSerializationMode());

	delete gbfa;
	delete gbfa2;
	delete gbfa3;
}


// This test checks (de-serialization of the object in different modes. This is simultaneously
// a test for the toString/fromString functions of the GObject class.

// EOF
