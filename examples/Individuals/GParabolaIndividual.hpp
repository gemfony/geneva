/**
 * @file
 */

/* GParabolaIndividual.cpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/function.hpp>

// GenEvA header files go here
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GEvaluator.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBoostThreadPopulation.hpp"

/************************************************************************************************/
/**
 * Set up an evaluation function. Note that we are over-cautious here. If you are
 * dealing with your own objects, you might want to deploy faster alternatives
 */
double parabola(const GParameterSet& gps){
	// Does gps have any data at all ?
	if(gps.empty()){
		std::cout << "In eval: Error! Supplied GParameterSet object" << std::endl
			      << "does not contain any data" << std::endl;
		std::terminate();
	}

	// Extract data - we know there is at least one shared_ptr<GParameterBase> registered.
	// gps is essentially a std::vector<boost::shared_ptr<GParameterBase> > . get() is
	// the shared_ptr way of retrieving the stored object.
	const GParameterBase *data_base = gps.at(0).get();

	// We know there should be a GDOubleCollection present - extract it.
	const GDoubleCollection *gdc_load = dynamic_cast<const GDoubleCollection*>(data_base);

	// Check that the conversion worked. dynamic_cast emits an empty pointer,
	// if this was not the case.
	if(!gdc_load){
		std::cout << "In eval: Conversion error!" << std::endl;
		std::terminate();
	}

	// Great - now we can do the actual calculations. We do this the fancy way ...
	double result = 0;
	std::vector<double>::const_iterator cit;
	for(cit=gdc_load->begin(); cit!=gdc_load->end(); ++cit){
		result += std::pow(*cit, 2); // Sum up the squares
	}

	return result;
}
