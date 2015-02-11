/**
 * @file GOptimizationBenchmarkConfig.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <vector>

// Boost header files go here
#include <boost/cstdint.hpp>

#ifndef GOPTIMIZATIONBENCHMARKCONFIG_HPP_
#define GOPTIMIZATIONBENCHMARKCONFIG_HPP_

// Geneva header files go here

#include "common/GParserBuilder.hpp"

namespace Gem {
namespace Tests {

const std::size_t DEFNOPTBENCHTESTS=10;

/*********************************************************************************/
/**
 * A class that parses configuration options for the GOptimizationBenchmark test
 */
class G_API GOptimizationBenchmarkConfig {
public:
	/*****************************************************************************/
	/**
	 * The default constructor
	 *
	 * @param configFile The name of a configuration file
	 * @param resultFile The name of a file to which results should be written
	 */
	explicit GOptimizationBenchmarkConfig(const std::string& configFile)
		: nTests_(DEFNOPTBENCHTESTS)
		, parDim_(0)
		, resultFile_("result.C")
	{
		using namespace Gem::Common;

		gpb_.registerFileParameter(
			"nTests"
			, nTests_
			, nTests_
			, VAR_IS_ESSENTIAL
			, "The number of tests to be performed for each dimension"
		);

		// Set up a vector of default values
		std::vector<boost::uint32_t> def_pardim;

		def_pardim.push_back(2);
		def_pardim.push_back(4);
		def_pardim.push_back(8);
		def_pardim.push_back(16);
		def_pardim.push_back(32);
		def_pardim.push_back(64);
		def_pardim.push_back(128);
		/* def_pardim.push_back(256);
		def_pardim.push_back(512);
		def_pardim.push_back(1024);
		def_pardim.push_back(2048);
		def_pardim.push_back(4096); */

		gpb_.registerFileParameter(
			"dimension"
			, parDim_
			, def_pardim
			, VAR_IS_ESSENTIAL
			, "Dimensions of the parameter space to be tested"
		);

		gpb_.registerFileParameter(
			"resultFile"
			, resultFile_
			, resultFile_
			, VAR_IS_ESSENTIAL
			, "The name of a file to which results of the;benchmark should be written"
		);

		// Read in the configuration file
		gpb_.parseConfigFile(configFile);
	}

	/*****************************************************************************/
	/**
	 * Retrieval of the name of the result file
	 *
	 * @return The name of the result file
	 */
	const std::string& getResultFileName() const {
		return resultFile_;
	}

	/*****************************************************************************/
	/**
	 * Retrieval of the vector holding the test dimensions
	 *
	 * @return A vector holding the parameter space dimensions to be tested
	 */
	const std::vector<boost::uint32_t>& getParDim() const {
		return parDim_;
	}

	/*****************************************************************************/
	/**
	 * Retrieve the number of tests to be performed for each dimension
	 *
	 * @return The number of tests to be performed for each dimension
	 */
	std::size_t getNTests() const {
		return nTests_;
	}

private:
	/*****************************************************************************/

	GOptimizationBenchmarkConfig(); ///< Default constructor: Intentionally private and undefined

	Gem::Common::GParserBuilder gpb_; ///< Handles the actual parsing

	std::size_t nTests_; ///< The number of tests to be performed for each dimension
	std::vector<boost::uint32_t> parDim_;
	std::string resultFile_; ///< The name of a file to which results should be written
};

/*********************************************************************************/

} /* namespace Tests */
} /* namespace Gem */

#endif /* GOPTIMIZATIONBENCHMARKCONFIG_HPP_ */
