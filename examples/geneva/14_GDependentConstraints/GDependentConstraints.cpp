/**
 * @file GDependentConstraints.cpp
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

// Boost header files go here
#include <boost/ref.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

using namespace Gem::Geneva;
namespace po = boost::program_options;

int main(int argc, char **argv) {
	//---------------------------------------------------------------------------
	// We want to add additional command line options

	bool printValid = false;
	bool useRawFitness = false;
	std::string monitorSpec = "empty";
	bool observeBoundaries = false;

	// Assemble command line options
	boost::program_options::options_description user_options;
	user_options.add_options() (
		"validOnly"
		, po::value<bool>(&printValid)->implicit_value(true)->default_value(false) // This allows you say both --validOnly and --validOnly=true
		, "Enforces output of valid solutions only"
	)(
		"useRawFitness"
		, po::value<bool>(&useRawFitness)->implicit_value(true)->default_value(false) // This allows you say both --useRawFitness and --useRawFitness=true
		, "Plot untransformed fitness value, even if a transformation takes place for the purpose of optimization"
	)(
		"monitorSpec"
		, po::value<std::string>(&monitorSpec)->default_value(std::string("empty"))
		, "Allows you to specify variables to be monitored like this: \"d(var0 -10 10)\""
	)(
		"observeBoundaries"
		, po::value<bool>(&observeBoundaries)->implicit_value(true)->default_value(false) // This allows you say both --observeBoundaries and --observeBoundaries=true
		, "Only plot inside of specified boundaries (no effect, when monitorSpec hasn't been set)"
	);

	Go2 go(argc, argv, "./config/Go2.json", user_options);

	//---------------------------------------------------------------------------
	// Client mode
	if(go.clientMode()) {
		return go.clientRun();
	} // Execution will end here in client mode

	//---------------------------------------------------------------------------
	// Create a factory for GFunctionIndividual objects and perform
	// any necessary initial work.
	std::shared_ptr<GFunctionIndividualFactory>
		gfi_ptr(new GFunctionIndividualFactory("./config/GFunctionIndividual.json"));

	// We want the GFunctionIndividual objects to always use GConstrainedDoubleObject objects
	// so that parameter types have defined names
	gfi_ptr->setPT(Gem::Geneva::parameterType::USEGCONSTRAINEDDOUBLEOBJECT);

	//---------------------------------------------------------------------------
	// Register a progress plotter with the global optimization algorithm factory
	if(monitorSpec != "empty") {
		std::shared_ptr<GProgressPlotterT<GParameterSet, double>> progplot_ptr(new GProgressPlotterT<GParameterSet, double>());

		progplot_ptr->setProfileSpec(monitorSpec);
		progplot_ptr->setObserveBoundaries(observeBoundaries);
		progplot_ptr->setMonitorValidOnly(printValid); // Only record valid parameters, when printValid is set to true
		progplot_ptr->setUseRawEvaluation(useRawFitness); // Use untransformed evaluation values for logging

		go.registerPluggableOM(progplot_ptr);
	}

	//---------------------------------------------------------------------------
	// Add a number of start values to the go object. We also add some constraint definitions here.
	for(std::size_t i=0; i<10; i++) {
		std::shared_ptr<GFunctionIndividual> p = gfi_ptr->get<GFunctionIndividual>();

		// Create the constraint objects
		std::shared_ptr<GDoubleSumConstraint>           doublesum_constraint_ptr(new GDoubleSumConstraint(1.));
		std::shared_ptr<GSphereConstraint>              sphere_constraint_ptr(new GSphereConstraint(3.));
		std::shared_ptr<GParameterSetFormulaConstraint> formula_constraint(new GParameterSetFormulaConstraint("fabs(sin({{var0}})/max(fabs({{var1}}), 0.000001))")); // sin(x) < y
		std::shared_ptr<GDoubleSumGapConstraint>        gap_constraint(new GDoubleSumGapConstraint(1.,0.05)); // The sum of all variables must be 1 +/- 0.05

		// Create a check combiner and add the constraint objects to it
		std::shared_ptr<GCheckCombinerT<GOptimizableEntity>> combiner_ptr(new GCheckCombinerT<GOptimizableEntity>());
		combiner_ptr->setCombinerPolicy(Gem::Geneva::validityCheckCombinerPolicy::MULTIPLYINVALID);

		combiner_ptr->addCheck(doublesum_constraint_ptr);
		combiner_ptr->addCheck(sphere_constraint_ptr);
		combiner_ptr->addCheck(formula_constraint);
		// combiner_ptr->addCheck(gap_constraint);

		// Register the combiner with the individual (note: we could also have registered
		// one of the "single" constraints here (see below for commented-out examples)
		p->registerConstraint(combiner_ptr);

		// p->registerConstraint(doublesum_constraint_ptr);
		// p->registerConstraint(sphere_constraint_ptr);
		// p->registerConstraint(formula_constraint);
		// p->registerConstraint(gap_constraint);

		go.push_back(p);
	}

	//---------------------------------------------------------------------------

	// Perform the actual optimization
	std::shared_ptr<GFunctionIndividual> p = go.optimize<GFunctionIndividual>();

	// Here you can do something with the best individual ("p") found.
	// We simply print its content here, by means of an operator<< implemented
	// in the GFunctionIndividual code.
	std::cout
	<< "Best result found:" << std::endl
	<< p << std::endl;
}
