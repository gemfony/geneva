/**
 * @file GLogger_test.cpp
 *
 * This test checks performs simple tests of the GLogger class
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

#define BOOST_TEST_MODULE GLogger_test
#include <boost/test/unit_test.hpp>

#include <boost/shared_ptr.hpp>

// GenEvA header files go here
#include "GLogger.hpp"

#include "GParabolaIndividual.hpp"

using namespace Gem::GLogFramework;

BOOST_AUTO_TEST_CASE( glogger_simple_invocation_no_failure_expected )
{ /* nothing */ }
