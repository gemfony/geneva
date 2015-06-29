/**
 * @file GSerializationOverhead.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

// Boost header files go here
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/lexical_cast.hpp"

// Geneva header files go here
#include "common/GPlotDesigner.hpp"
#include "geneva-individuals/GTestIndividual2.hpp"

// The number of consecutive measurements
const std::size_t NMEASUREMENTS=100;

// The maximum object size
const std::size_t MAXOBJECTSIZE = 100;

// The step size
const std::size_t STEPSIZE = 10;

// The default serialization mode
const Gem::Common::serializationMode DEFAULTSERMODE = Gem::Common::SERIALIZATIONMODE_BINARY;
// const Gem::Common::serializationMode DEFAULTSERMODE = Gem::Common::SERIALIZATIONMODE_XML;

using namespace boost::posix_time;
using namespace Gem::Common;
using namespace Gem::Geneva;
using namespace Gem::Tests;

int main(int argc, char **argv) {
	std::string caption = "Times for adaption and serialization (" + boost::lexical_cast<std::string>(NMEASUREMENTS) + " measurements each; serialization in " + serializationModeToString(DEFAULTSERMODE) + ")";
	GPlotDesigner gpd(caption, 2, NPERFOBJECTTYPES);

	std::shared_ptr<GGraph2D> gdo_adapt_ptr(new GGraph2D());
	gdo_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gdo_adapt_ptr->setPlotLabel("GDoubleObject / Adaption");
	gdo_adapt_ptr->setXAxisLabel("Number of parameters");
	gdo_adapt_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gdo_ser_ptr(new GGraph2D());
	gdo_ser_ptr->setPlotMode(Gem::Common::CURVE);
	gdo_ser_ptr->setPlotLabel("GDoubleObject / Serialization");
	gdo_ser_ptr->setXAxisLabel("Number of parameters");
	gdo_ser_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gcdo_adapt_ptr(new GGraph2D());
	gcdo_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gcdo_adapt_ptr->setPlotLabel("GConstrainedDoubleObject / Adaption");
	gcdo_adapt_ptr->setXAxisLabel("Number of parameters");
	gcdo_adapt_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gcdo_ser_ptr(new GGraph2D());
	gcdo_ser_ptr->setPlotMode(Gem::Common::CURVE);
	gcdo_ser_ptr->setPlotLabel("GConstrainedDoubleObject / Serialization");
	gcdo_ser_ptr->setXAxisLabel("Number of parameters");
	gcdo_ser_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gcdoc_adapt_ptr(new GGraph2D());
	gcdoc_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gcdoc_adapt_ptr->setPlotLabel("GConstrainedDoubleObjectCollection / Adaption");
	gcdoc_adapt_ptr->setXAxisLabel("Number of parameters");
	gcdoc_adapt_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gcdoc_ser_ptr(new GGraph2D());
	gcdoc_ser_ptr->setPlotMode(Gem::Common::CURVE);
	gcdoc_ser_ptr->setPlotLabel("GConstrainedDoubleObjectCollection / Serialization");
	gcdoc_ser_ptr->setXAxisLabel("Number of parameters");
	gcdoc_ser_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gdc_adapt_ptr(new GGraph2D());
	gdc_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gdc_adapt_ptr->setPlotLabel("GDoubleCollection / Adaption");
	gdc_adapt_ptr->setXAxisLabel("Number of parameters");
	gdc_adapt_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gdc_ser_ptr(new GGraph2D());
	gdc_ser_ptr->setPlotMode(Gem::Common::CURVE);
	gdc_ser_ptr->setPlotLabel("GDoubleCollection / Serialization");
	gdc_ser_ptr->setXAxisLabel("Number of parameters");
	gdc_ser_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gcdc_adapt_ptr(new GGraph2D());
	gcdc_adapt_ptr->setPlotMode(Gem::Common::CURVE);
	gcdc_adapt_ptr->setPlotLabel("GConstrainedDoubleCollection / Adaption");
	gcdc_adapt_ptr->setXAxisLabel("Number of parameters");
	gcdc_adapt_ptr->setYAxisLabel("Time (s)");

	std::shared_ptr<GGraph2D> gcdc_ser_ptr(new GGraph2D());
	gcdc_ser_ptr->setPlotMode(Gem::Common::CURVE);
	gcdc_ser_ptr->setPlotLabel("GConstrainedDoubleCollection / Serialization");
	gcdc_ser_ptr->setXAxisLabel("Number of parameters");
	gcdc_ser_ptr->setYAxisLabel("Time (s)");

	for(std::size_t s=1; s<=MAXOBJECTSIZE; s+=(s<10?1:STEPSIZE)) {
		std::cout << "Starting measurement for object size " << s << std::endl;

		for(std::size_t o=0; o<NPERFOBJECTTYPES; o++) {
			// Create a GTestIndividual2 object of the desired size
			std::shared_ptr<GTestIndividual2> gti_ptr(new GTestIndividual2(s,PERFOBJECTTYPE(o)));

			// First test the time needed for NMEASUREMENTS
			// consecutive adaptions
			ptime pre_adapt = microsec_clock::local_time();
			for(std::size_t i=1; i<=NMEASUREMENTS; i++) {
				gti_ptr->adapt();
			}
			ptime post_adapt = microsec_clock::local_time();

			// Now measure the time needed for NMEASUREMENTS
			// consecutive (de-)serializations in the fastest mode (binary)
			for(std::size_t i=1; i<=NMEASUREMENTS; i++) {
				gti_ptr->GObject::fromString(gti_ptr->GObject::toString(DEFAULTSERMODE), DEFAULTSERMODE);
			}
			ptime post_serialization = microsec_clock::local_time();

			time_duration adaptionTime = post_adapt - pre_adapt;
			time_duration serializationTime = post_serialization - post_adapt;

			double adaptionTimeD = double(adaptionTime.total_microseconds())/1000000.;
			double serializationTimeD = double(serializationTime.total_microseconds())/1000000.;

			switch(o) {
				case 0:
					gdo_adapt_ptr->add(std::tuple<double,double>((double)s, adaptionTimeD));
					gdo_ser_ptr->add(std::tuple<double,double>((double)s, serializationTimeD));
					break;

				case 1:
					gcdo_adapt_ptr->add(std::tuple<double,double>((double)s, adaptionTimeD));
					gcdo_ser_ptr->add(std::tuple<double,double>((double)s, serializationTimeD));
					break;

				case 2:
					gcdoc_adapt_ptr->add(std::tuple<double,double>((double)s, adaptionTimeD));
					gcdoc_ser_ptr->add(std::tuple<double,double>((double)s, serializationTimeD));
					break;

				case 3:
					gdc_adapt_ptr->add(std::tuple<double,double>((double)s, adaptionTimeD));
					gdc_ser_ptr->add(std::tuple<double,double>((double)s, serializationTimeD));
					break;

				case 4:
					gcdc_adapt_ptr->add(std::tuple<double,double>((double)s, adaptionTimeD));
					gcdc_ser_ptr->add(std::tuple<double,double>((double)s, serializationTimeD));
					break;

				default:
					glogger
					<< "Error in main(): Incorrect object type requested: " << o << std::endl
					<< GEXCEPTION;
					break;
			}
		}
	}

	gpd.registerPlotter(gdo_adapt_ptr);
	gpd.registerPlotter(gdo_ser_ptr);
	gpd.registerPlotter(gcdo_adapt_ptr);
	gpd.registerPlotter(gcdo_ser_ptr);
	gpd.registerPlotter(gcdoc_adapt_ptr);
	gpd.registerPlotter(gcdoc_ser_ptr);
	gpd.registerPlotter(gdc_adapt_ptr);
	gpd.registerPlotter(gdc_ser_ptr);
	gpd.registerPlotter(gcdc_adapt_ptr);
	gpd.registerPlotter(gcdc_ser_ptr);

	// Emit the result file
	gpd.writeToFile("result.C");
}
