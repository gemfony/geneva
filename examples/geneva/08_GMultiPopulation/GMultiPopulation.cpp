/**
 * @file GBrokerOverhead.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */


// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include "geneva/Go2.hpp"
#include "geneva/GMultiPopulationEAT.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

// Declares a function to parse the command line
#include "GArgumentParser.hpp"

using namespace Gem::Geneva;
using namespace Gem::Courtier;
using namespace Gem::Hap;

/************************************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
  std::string configFile;		  
  bool serverMode;
  std::string ip;
  unsigned short port;
  boost::uint16_t nProducerThreads;
  boost::uint16_t nEvaluationThreads;
  std::size_t populationSizeSuper;
  std::size_t nParentsSuper;
  boost::uint32_t maxIterationsSuper;
  long maxMinutesSuper;
  boost::uint32_t reportIterationSuper;
  duplicationScheme rSchemeSuper;
  sortingModeMP smodeSuper;
  std::size_t populationSizeSub;
  std::size_t nParentsSub;
  boost::uint32_t maxIterationsSub;
  long maxMinutesSub;
  boost::uint32_t reportIterationSub;
  duplicationScheme rSchemeSub;
  sortingMode smodeSub;
  std::size_t parDim;
  double minVar;
  double maxVar;
  bool returnRegardless;
  boost::uint32_t nProcessingUnits;
  solverFunction df;
  boost::uint32_t adaptionThreshold;
  double sigma;
  double sigmaSigma;
  double minSigma;
  double maxSigma;
  double adProb;
  Gem::Common::serializationMode serMode;

  if(!parseCommandLine(
        argc, argv,
        configFile
    )
     ||
     !parseConfigFile(configFile,
		      nProducerThreads,
		      nEvaluationThreads,
		      populationSizeSuper,
		      nParentsSuper,
		      maxIterationsSuper,
		      maxMinutesSuper,
		      reportIterationSuper,
		      rSchemeSuper,
		      smodeSuper,
		      populationSizeSub,
		      nParentsSub,
		      maxIterationsSub,
		      maxMinutesSub,
		      reportIterationSub,
		      rSchemeSub,
		      smodeSub,
			   returnRegardless,
			   nProcessingUnits,
		      adProb,
		      adaptionThreshold,
			   sigma,
			   sigmaSigma,
			   minSigma,
			   maxSigma,
		      parDim,
		      minVar,
		      maxVar,
		      df))
    { exit(1); }

  // Random numbers are our most valuable good. Set the number of threads
  GRANDOMFACTORY->setNProducerThreads(nProducerThreads);

  //***************************************************************************
  // Create a factory for GFunctionIndividual objects and perform
  // any necessary initial work.
  GFunctionIndividualFactory gfi("./GFunctionIndividual.json");

  // Create the first set of parent populations.
  std::vector<boost::shared_ptr<GBaseEA> > parentPopulations;
  for(std::size_t psuper = 0; psuper<nParentsSuper; psuper++) {
	  // This smart pointer holds a parent population. Note that it can be useful
	  // not to parallelize on the level of sub-populations. Using a GBrokerEA here
	  // will likely lead to failure.
	  boost::shared_ptr<GSerialEA> sub_pop_ptr
		  = boost::shared_ptr<GSerialEA>(new GSerialEA());

	  // Create the first set of parent individuals. Initialization of parameters is done randomly.
	  for(std::size_t psub = 0 ; psub<nParentsSub; psub++) {
		  boost::shared_ptr<GParameterSet> functionIndividual_ptr = gfi();

		  // Set up a GDoubleCollection with dimension values, each initialized
		  // with a random number in the range [min,max[
		  boost::shared_ptr<GDoubleCollection> gdc_ptr(new GDoubleCollection(parDim,minVar,maxVar));

		  // Set up and register an adaptor for the collection, so it
		  // knows how to be adapted.
		  boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma,sigmaSigma,minSigma,maxSigma));
		  gdga_ptr->setAdaptionThreshold(adaptionThreshold);
		  gdga_ptr->setAdaptionProbability(adProb);
		  gdc_ptr->addAdaptor(gdga_ptr);

		  // Make the parameter collection known to this individual
		  functionIndividual_ptr->push_back(gdc_ptr);

		  sub_pop_ptr->push_back(functionIndividual_ptr);
	  }

	  // Specify some general population settings
	  sub_pop_ptr->setDefaultPopulationSize(populationSizeSub,nParentsSub);
	  sub_pop_ptr->setMaxIteration(maxIterationsSub);
	  sub_pop_ptr->setMaxTime(boost::posix_time::minutes(maxMinutesSub));
	  sub_pop_ptr->setReportIteration(reportIterationSub);
	  sub_pop_ptr->setRecombinationMethod(rSchemeSub);
	  sub_pop_ptr->setSortingScheme(smodeSub);

	  // Add the sub population to the vector
	  parentPopulations.push_back(sub_pop_ptr);
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // This EA population can hold derivatives of GOptimizationAlgorithmT<GParameterSet>
  GMultiPopulationEAT<GParameterSet> gmp;

  // Now we have suitable populations and can fill them with data

  // Add individuals to the population
  for(std::size_t psuper = 0 ; psuper<nParentsSuper; psuper++) {
	  gmp.push_back(parentPopulations[psuper]);
  }
 
  // Specify some general population settings
  gmp.setDefaultPopulationSize(populationSizeSuper,nParentsSuper);
  gmp.setMaxIteration(maxIterationsSuper);
  gmp.setMaxTime(boost::posix_time::minutes(maxMinutesSuper));
  gmp.setReportIteration(reportIterationSuper);
  gmp.setRecombinationMethod(rSchemeSuper);
  gmp.setSortingScheme(smodeSuper);
  gmp.setEmitTerminationReason(true);
  
  // Do the actual optimization
  gmp.optimize();

  //--------------------------------------------------------------------------------------------
  // Terminate Geneva
  return 0;
}
