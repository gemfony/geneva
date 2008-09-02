/**
 * @file GBitFlipAdaptor_test.cpp
 *
 * This test checks all public member functions of the GBitFlipAdaptor adaptor
 * class. In addition, it attempts to check parent classes, in particular the
 * GObject class.
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

BOOST_AUTO_TEST_CASE( constructors_test )
{
	GBitFlipAdaptor *gbfa=new GBitFlipAdaptor(ADAPTORNAME);
	GObject *gobject = gbfa;
	BOOST_CHECK( gobject->name() == ADAPTORNAME );
	delete gbfa;
}

// EOF
