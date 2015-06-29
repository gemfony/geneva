/**
 * @file GPluggableOptimizationMonitorsT.hpp
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
#include <fstream>
#include <type_traits>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/date_time.hpp>

#ifndef GPLUGGABLEOPTIMIZATIONMONITORST_HPP_
#define GPLUGGABLEOPTIMIZATIONMONITORST_HPP_

// Geneva headers go here
#include "common/GPlotDesigner.hpp"
#include "common/GLogger.hpp"
#include "geneva/GParameterPropertyParser.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/*
 * This is a collection of simple pluggable modules suitable for emitting certain specialized
 * information from within optimization algorithms. They can be plugged into GOptimizationMonitorT<>
 * derivatives. The one requirement is that they implement a function "informationFunction"
 * according to the API of GOptimizationAlgorithmT<>::GBasePluggableOMT
 */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements the standard output common to all optimization algorithms.
 * It will usually be already registered as a pluggable optimization monitor, when
 * you instantiate a new optimization algorithm.
 */
template <typename ind_type>
class GStandardMonitorT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
			&make_nvp("GBasePluggableOMT", boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this));
	}

	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GStandardMonitorT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GStandardMonitorT(const GStandardMonitorT<ind_type>& cp)
		: GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT(cp)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GStandardMonitorT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GStandardMonitorT<ind_type>& operator=(const GStandardMonitorT<ind_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GCollectiveMonitorT object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GStandardMonitorT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GCollectiveMonitorT object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GStandardMonitorT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Aggregates the work of all registered pluggable monitors
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		switch(im) {
			case Gem::Geneva::INFOINIT: {
				glogger
				<< "Starting an optimization run with algorithm \"" << goa->getAlgorithmName() << "\"" << std::endl
				<< GLOGGING;
			}
				break;

			case Gem::Geneva::INFOPROCESSING: {
				glogger
				<< std::setprecision(5)
				<< goa->getIteration() << ": "
			   << Gem::Common::g_to_string(goa->getBestCurrentPrimaryFitness())
				<< " // best past: " << Gem::Common::g_to_string(goa->getBestKnownPrimaryFitness())
				<< std::endl
				<< GLOGGING;
			}
				break;

			case Gem::Geneva::INFOEND: {
				glogger
				<< "End of optimization reached in algorithm \"" << goa->getAlgorithmName()
				<< "\"" << std::endl
				<< GLOGGING;
			}
				break;

			default: {
				glogger
				<< "In GStandardMonitorT<ind_type>::informationFunction(): Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
			}
				break;
		}
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GStandardMonitorT<>");
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GCollectiveMonitorT<ind_type> reference independent of this object and convert the pointer
		const GStandardMonitorT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GStandardMonitorT<>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GCollectiveMonitorT<ind_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GCollectiveMonitorT<ind_type> reference independent of this object and convert the pointer
		const GStandardMonitorT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... no local data
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GStandardMonitorT<ind_type>(*this);
	}

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GStandardMonitorT<ind_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GStandardMonitorT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GStandardMonitorT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to output fitness information for a given optimization run.
 * It takes care of successive runs and marks them in the output. At the user's
 * choice, information may be output for the n best individual(s) found so far or
 * for the n best individual(s) in a given iteration.
 */
template <typename ind_type>
class GFitnessMonitorT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOMT", boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this))
		& BOOST_SERIALIZATION_NVP(xDim_)
	  	& BOOST_SERIALIZATION_NVP(yDim_)
		& BOOST_SERIALIZATION_NVP(nMonitorInds_)
  		& BOOST_SERIALIZATION_NVP(resultFile_)
	 	& BOOST_SERIALIZATION_NVP(infoInitRun_)
		& BOOST_SERIALIZATION_NVP(fitnessGraphVec_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor. Some variables may be initialized in the class body.
	 */
	GFitnessMonitorT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GFitnessMonitorT(const GFitnessMonitorT<ind_type>& cp)
		: GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT(cp)
	  	, xDim_(cp.xDim_)
	  	, yDim_(cp.yDim_)
		, nMonitorInds_(cp.nMonitorInds_)
		, resultFile_(cp.resultFile_)
		, infoInitRun_(cp.infoInitRun_)
	{
		Gem::Common::copyCloneableSmartPointerContainer(cp.fitnessGraphVec_, fitnessGraphVec_);
	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GFitnessMonitorT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GFitnessMonitorT<ind_type>& operator=(const GFitnessMonitorT<ind_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GCollectiveMonitorT object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GFitnessMonitorT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Checks for inequality with another GCollectiveMonitorT object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GFitnessMonitorT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Allows to specify a different name for the result file
	 *
	 * @param resultFile The desired name of the result file
	 */
	void setResultFileName(
		const std::string &resultFile
	) {
		resultFile_ = resultFile;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the current value of the result file name
	 *
	 * @return The current name of the result file
	 */
	std::string getResultFileName() const {
		return resultFile_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the dimensions of the canvas
	 *
	 * @param xDim The desired dimension of the canvas in x-direction
	 * @param yDim The desired dimension of the canvas in y-direction
	 */
	void setDims(const std::uint32_t &xDim, const std::uint32_t &yDim) {
		xDim_ = xDim;
		yDim_ = yDim;
	}

	/***************************************************************************/
	/**
	 * Retrieve the dimensions as a tuple
	 *
	 * @return The dimensions of the canvas as a tuple
	 */
	std::tuple<std::uint32_t, std::uint32_t> getDims() const {
		return std::tuple<std::uint32_t, std::uint32_t>(xDim_, yDim_);
	}

	/***************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in x-direction
	 *
	 * @return The dimension of the canvas in x-direction
	 */
	std::uint32_t getXDim() const {
		return xDim_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in y-direction
	 *
	 * @return The dimension of the canvas in y-direction
	 */
	std::uint32_t getYDim() const {
		return yDim_;
	}

	/***************************************************************************/
	/**
	 * Sets the number of individuals in the population that should be monitored.
	 * If nMonitorInds_ == 0, the default will be set to 3, as fitness graphs are plotted in a row,
	 * and more than 3 will not give satisfactory graphical results. You may however
	 * request more monitored individuals, but will likely have to postprocess the ROOT script.
	 * If nMonitorInds_ is set to a larger number than there are individuals in the population,
	 * the value will be reset to that amount of individuals in informationFunction.
	 *
	 * @oaram nMonitorInds The number of individuals in the population that should be monitored
	 */
	void setNMonitorIndividuals(const std::size_t &nMonitorInds) {
		// Determine a suitable number of monitored individuals, if it hasn't already
		// been set externally.
		if(nMonitorInds_ == 0) {
			nMonitorInds_ = std::size_t(3);
		}

		nMonitorInds_ = nMonitorInds;
	}

	/***************************************************************************/
	/**
	 * Retrieves the number of individuals that are being monitored
	 *
	 * @return The number of individuals in the population being monitored
	 */
	std::size_t getNMonitorIndividuals() const {
		return nMonitorInds_;
	}

	/***************************************************************************/
	/**
	 * Aggregates the work of all registered pluggable monitors
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		switch(im) {
			case Gem::Geneva::INFOINIT: {
				if(!infoInitRun_) {
					// Reset the number of monitored individuals to a suitable valie,
					// if necessary.
					if(nMonitorInds_ > goa->getPopulationSize()) {
						nMonitorInds_ = goa->getPopulationSize();
					}

					// Set up the plotters
					for(std::size_t ind = 0; ind<nMonitorInds_; ind++) {
						std::shared_ptr <Gem::Common::GGraph2D> graph(new Gem::Common::GGraph2D());
						graph->setXAxisLabel("Iteration");
						graph->setYAxisLabel("Fitness");
						graph->setPlotLabel(std::string("Individual ") + boost::lexical_cast<std::string>(ind));
						graph->setPlotMode(Gem::Common::CURVE);

						fitnessGraphVec_.push_back(graph);
					}

					// Make sure fitnessGraphVec_ is only initialized once
					infoInitRun_ = true;
				}
			} break;

			case Gem::Geneva::INFOPROCESSING: {

			} break;

			case Gem::Geneva::INFOEND: {

			} break;

			default: {
				glogger
				<< "In GFitnessMonitorT<ind_type>::informationFunction(): Received invalid infoMode " << im <<
				std::endl
				<< GEXCEPTION;
			} break;
		}
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GFitnessMonitorT<>");
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GCollectiveMonitorT<ind_type> reference independent of this object and convert the pointer
		const GFitnessMonitorT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GFitnessMonitorT<>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(xDim_, p_load->xDim_), token);
		compare_t(IDENTITY(yDim_, p_load->yDim_), token);
		compare_t(IDENTITY(nMonitorInds_, p_load->nMonitorInds_), token);
		compare_t(IDENTITY(resultFile_, p_load->resultFile_), token);
		compare_t(IDENTITY(infoInitRun_, p_load->infoInitRun_), token);
		compare_t(IDENTITY(fitnessGraphVec_, p_load->fitnessGraphVec_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GCollectiveMonitorT<ind_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GCollectiveMonitorT<ind_type> reference independent of this object and convert the pointer
		const GFitnessMonitorT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... and then our local data
		xDim_ = p_load->xDim_;
		yDim_ = p_load->yDim_;
		nMonitorInds_ = p_load->nMonitorInds_;
		resultFile_ = p_load->resultFile_;
		infoInitRun_ = p_load->infoInitRun_;

		Gem::Common::copyCloneableSmartPointerContainer(p_load->fitnessGraphVec_, fitnessGraphVec_);
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GFitnessMonitorT<ind_type>(*this);
	}

private:
	/************************************************************************/

	std::uint32_t xDim_ = DEFAULTXDIMOM; ///< The dimension of the canvas in x-direction
	std::uint32_t yDim_ = DEFAULTYDIMOM; ///< The dimension of the canvas in y-direction
	std::size_t nMonitorInds_ = 1; ///< The number of individuals that should be monitored
	std::string resultFile_ = DEFAULTROOTRESULTFILEOM; ///< The name of the file to which data is emitted

	bool infoInitRun_ = false; ///< Allows to check whether the INFOINIT section of informationFunction has already been passed at least once
	std::vector<std::shared_ptr<Gem::Common::GGraph2D>> fitnessGraphVec_; ///< Will hold progress information

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFitnessMonitorT<ind_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFitnessMonitorT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFitnessMonitorT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class accepts a number of other pluggable monitors and executes them
 * in sequence.
 */
template <typename ind_type>
class GCollectiveMonitorT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOMT",	boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this));

		// Some preparation needed if this is a load operation.
		// This is needed to work around a problem in Boost 1.58
		if (Archive::is_loading::value) {
			pluggable_monitors_.clear();
		}

		ar
		& BOOST_SERIALIZATION_NVP(pluggable_monitors_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GCollectiveMonitorT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GCollectiveMonitorT(const GCollectiveMonitorT<ind_type>& cp)
		: GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT(cp)
	{
		Gem::Common::copyCloneableSmartPointerContainer(cp.pluggable_monitors_, pluggable_monitors_);
	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GCollectiveMonitorT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GCollectiveMonitorT<ind_type>& operator=(const GCollectiveMonitorT<ind_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GCollectiveMonitorT object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GCollectiveMonitorT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GCollectiveMonitorT object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GCollectiveMonitorT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Aggregates the work of all registered pluggable monitors
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		for(auto it: pluggable_monitors_) {
			it->informationFunction(im,goa);
		}
	}

	/***************************************************************************/
	/**
	 * Allows to register a new pluggable monitor
	 */
	void registerPluggableOM(std::shared_ptr<typename Gem::Geneva::GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT> om_ptr) {
		if(om_ptr) {
			pluggable_monitors_.push_back(om_ptr);
		} else {
			glogger
			<< "In GCollectiveMonitorT<>::registerPluggableOM(): Error!" << std::endl
			<< "Got empty pointer to pluggable optimization monitor." << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * Checks if adaptors have been registered in the collective monitor
	 */
	bool hasOptimizationMonitors() const {
		return !pluggable_monitors_.empty();
	}

	/***************************************************************************/
	/**
	 * Allows to clear all registered monitors
	 */
	void resetPluggbleOM() {
		pluggable_monitors_.clear();
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GCollectiveMonitorT<>");
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GCollectiveMonitorT<ind_type> reference independent of this object and convert the pointer
		const GCollectiveMonitorT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GCollectiveMonitorT<>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(pluggable_monitors_, p_load->pluggable_monitors_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GCollectiveMonitorT<ind_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GCollectiveMonitorT<ind_type> reference independent of this object and convert the pointer
		const GCollectiveMonitorT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... and then our local data
		Gem::Common::copyCloneableSmartPointerContainer(p_load->pluggable_monitors_, pluggable_monitors_);
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GCollectiveMonitorT<ind_type>(*this);
	}


private:
	std::vector<std::shared_ptr<typename Gem::Geneva::GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>> pluggable_monitors_; ///< The collection of monitors

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GCollectiveMonitorT<ind_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GCollectiveMonitorT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GCollectiveMonitorT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to monitor a given set of variables inside of all or of the
 * best individuals of a population, creating a graphical output using ROOT. It
 * supports floating point types only. double and float values may not be mixed.
 */
template <typename ind_type, typename fp_type>
class GProgressPlotterT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
			& make_nvp("GBasePluggableOMT",	boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this))
			& BOOST_SERIALIZATION_NVP(fp_profVarVec_)
			& BOOST_SERIALIZATION_NVP(gpd_oa_)
			& BOOST_SERIALIZATION_NVP(progressPlotter2D_oa_)
			& BOOST_SERIALIZATION_NVP(progressPlotter3D_oa_)
			& BOOST_SERIALIZATION_NVP(progressPlotter4D_oa_)
			& BOOST_SERIALIZATION_NVP(fileName_)
			& BOOST_SERIALIZATION_NVP(canvasDimensions_)
		   & BOOST_SERIALIZATION_NVP(monitorBestOnly_)
			& BOOST_SERIALIZATION_NVP(monitorValidOnly_)
		   & BOOST_SERIALIZATION_NVP(observeBoundaries_)
			& BOOST_SERIALIZATION_NVP(addPrintCommand_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if fp_type really is a floating point type
	static_assert(
		std::is_floating_point<fp_type>::value
		, "fp_type should be a floating point type"
	);

public:
	/***************************************************************************/
	/**
	 * The default constructor. Some member variables may be initialized
	 * in the class body.
	 */
	GProgressPlotterT()
		: gpd_oa_("Progress information", 1, 1)
		, canvasDimensions_(std::tuple<std::uint32_t,std::uint32_t>(1024,768))
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Construction with the information whether only the best individuals
	 * should be monitored and whether only valid items should be recorded.
	 * Some member variables may be initialized in the class body.
	 */
	GProgressPlotterT(bool monitorBestOnly, bool monitorValidOnly)
		: gpd_oa_("Progress information", 1, 1)
		, canvasDimensions_(std::tuple<std::uint32_t,std::uint32_t>(1024,768))
		, monitorBestOnly_(monitorBestOnly)
		, monitorValidOnly_(monitorValidOnly)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GProgressPlotterT(const GProgressPlotterT<ind_type, fp_type>& cp)
		: GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT(cp)
		, gpd_oa_(cp.gpd_oa_)
		, fileName_(cp.fileName_)
		, canvasDimensions_(cp.canvasDimensions_)
		, monitorBestOnly_(cp.monitorBestOnly_)
		, monitorValidOnly_(cp.monitorValidOnly_)
		, observeBoundaries_(cp.observeBoundaries_)
		, addPrintCommand_(cp.addPrintCommand_)
	{
		Gem::Common::copyCloneableSmartPointer(cp.progressPlotter2D_oa_, progressPlotter2D_oa_);
		Gem::Common::copyCloneableSmartPointer(cp.progressPlotter3D_oa_, progressPlotter3D_oa_);
		Gem::Common::copyCloneableSmartPointer(cp.progressPlotter4D_oa_, progressPlotter4D_oa_);
		Gem::Common::copyCloneableObjectsContainer(cp.fp_profVarVec_, fp_profVarVec_);
	}

	/***************************************************************************/
	/**
	 * The destuctor
	 */
	virtual ~GProgressPlotterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GProgressPlotterT<ind_type, fp_type>& operator=(const GProgressPlotterT<ind_type, fp_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GProgressPlotterT object
	 *
	 * @param  cp A constant reference to another GProgressPlotterT object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GProgressPlotterT<ind_type, fp_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GProgressPlotterT object
	 *
	 * @param  cp A constant reference to another GProgressPlotterT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GProgressPlotterT<ind_type, fp_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/**************************************************************************/
	/**
	 * Sets the specifications of the variables to be profiled. Note that
	 * boolean and integer variables specified in the argument will simply
	 * be ignored.
	 */
	void setProfileSpec(std::string parStr) {
		// Check that the parameter string isn't empty
		if(parStr.empty()) {
			glogger
			<< "In GPluggableOptimizationMonitorsT<>::setProfileSpec(std::string): Error!" << std::endl
			<< "Parameter string " << parStr << " is empty" << std::endl
			<< GEXCEPTION;
		}

		//---------------------------------------------------------------------------
		// Clear the parameter vectors
		fp_profVarVec_.clear();

		// Parse the parameter string
		GParameterPropertyParser ppp(parStr);

		//---------------------------------------------------------------------------
		// Retrieve the parameters

		std::tuple<
			typename std::vector<parPropSpec<fp_type>>::const_iterator
			, typename std::vector<parPropSpec<fp_type>>::const_iterator
		> t_d = ppp.getIterators<fp_type>();

		typename std::vector<parPropSpec<fp_type>>::const_iterator fp_cit = std::get<0>(t_d);
		typename std::vector<parPropSpec<fp_type>>::const_iterator d_end = std::get<1>(t_d);
		for(; fp_cit!=d_end; ++fp_cit) { // Note: fp_cit is already set to the begin of the double parameter arrays
			fp_profVarVec_.push_back(*fp_cit);
		}

		//---------------------------------------------------------------------------
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only the best individuals should be monitored.
	 */
	void setMonitorBestOnly(bool monitorBestOnly = true) {
		monitorBestOnly_ = monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only the best individuals should be monitored.
	 */
	bool getMonitorBestOnly() const {
		return monitorBestOnly_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only valid individuals should be monitored.
	 */
	void setMonitorValidOnly(bool monitorValidOnly = true) {
		monitorValidOnly_ = monitorValidOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only valid individuals should be monitored.
	 */
	bool getMonitorValidOnly() const {
		return monitorValidOnly_;
	}

	/***************************************************************************/
	/**
	 * Allows to spefify whether scan boundaries should be observed
	 */
	void setObserveBoundaries(bool observeBoundaries) {
		observeBoundaries_ = observeBoundaries;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether boundaries should be observed
	 */
	bool getObserveBoundaries() const {
		return observeBoundaries_;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether parameters should be profiled
	 */
	bool parameterProfileCreationRequested() const {
		return !fp_profVarVec_.empty();
	}

	/***************************************************************************/
	/**
	 * Retrieves the number of variables that will be profiled
	 */
	std::size_t nProfileVars() const {
		return fp_profVarVec_.size();
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions
	 */
	void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		canvasDimensions_ = canvasDimensions;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions using separate x and y values
	 */
	void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		canvasDimensions_ = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	}

	/***************************************************************************/
	/**
	 * Gives access to the canvas dimensions
	 */
	std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		return canvasDimensions_;
	}

	/******************************************************************************/
	/**
	 * Allows to add a "Print" command to the end of the script so that picture files are created
	 */
	void setAddPrintCommand(bool addPrintCommand) {
		addPrintCommand_ = addPrintCommand;
	}

	/******************************************************************************/
	/**
	 * Allows to retrieve the current value of the addPrintCommand_ variable
	 */
	bool getAddPrintCommand() const {
		return addPrintCommand_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the filename
	 */
	void setFileName(std::string fileName) {
		fileName_ = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current filename to which information will be emitted
	 */
	std::string getFileName() const {
		return fileName_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas label
	 */
	void setCanvasLabel(const std::string& canvasLabel) {
		gpd_oa_.setCanvasLabel(canvasLabel);
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the canvas label
	 */
	std::string getCanvasLabel() const {
		return gpd_oa_.getCanvasLabel();
	}

	/***************************************************************************/
	/**
	 * Determines a suitable label for a given parPropSpec value
	 */
	std::string getLabel(const parPropSpec<fp_type>& s) const {
		std::string result;

		std::size_t var_mode = std::get<0>(s.var);
		std::string var_name = std::get<1>(s.var);
		std::size_t var_pos  = std::get<2>(s.var);

		switch(var_mode) {
			//--------------------------------------------------------------------
			case 0: // parameters are identified by id
			{
				result = std::string("variable id ") + boost::lexical_cast<std::string>(var_pos);
			}
				break;

				//--------------------------------------------------------------------
			case 1:
			{
				result = var_name + "[" + boost::lexical_cast<std::string>(var_pos) + "]";
			}
				break;

				//--------------------------------------------------------------------
			case 2:
			{
				result = var_name;
			}
				break;

				//--------------------------------------------------------------------
			default:
			{
				glogger
				<< "In GProgressPlotterT<ind_type, fp_type>::getLabel(): Error" << std::endl
				<< "Invalid mode " << var_mode << " requested" << std::endl
				<< GEXCEPTION;
			}
				break;

				//--------------------------------------------------------------------
		};

		return result;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		switch(im) {
			case Gem::Geneva::INFOINIT:
			{
				switch(this->nProfileVars()) {
					case 1:
					{
						progressPlotter2D_oa_ = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());

						progressPlotter2D_oa_->setPlotMode(Gem::Common::CURVE);
						progressPlotter2D_oa_->setPlotLabel("Fitness as a function of a parameter value");
						progressPlotter2D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
						progressPlotter2D_oa_->setYAxisLabel("Fitness");

						gpd_oa_.registerPlotter(progressPlotter2D_oa_);
					}
						break;
					case 2:
					{
						progressPlotter3D_oa_ = std::shared_ptr<Gem::Common::GGraph3D>(new Gem::Common::GGraph3D());

						progressPlotter3D_oa_->setPlotLabel("Fitness as a function of parameter values");
						progressPlotter3D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
						progressPlotter3D_oa_->setYAxisLabel(this->getLabel(fp_profVarVec_[1]));
						progressPlotter3D_oa_->setZAxisLabel("Fitness");

						gpd_oa_.registerPlotter(progressPlotter3D_oa_);
					}
						break;

					case 3:
					{
						progressPlotter4D_oa_ = std::shared_ptr<Gem::Common::GGraph4D>(new Gem::Common::GGraph4D());

						progressPlotter4D_oa_->setPlotLabel("Fitness (color-coded) as a function of parameter values");
						progressPlotter4D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
						progressPlotter4D_oa_->setYAxisLabel(this->getLabel(fp_profVarVec_[1]));
						progressPlotter4D_oa_->setZAxisLabel(this->getLabel(fp_profVarVec_[2]));

						gpd_oa_.registerPlotter(progressPlotter4D_oa_);
					}
						break;

					default:
					{
						glogger
						<< "NOTE: In GProgressPlotterT<ind_type, fp_type>::informationFunction(INFOINIT):" << std::endl
						<< "Number of profiling dimensions " << this->nProfileVars() << " can not be displayed." << std::endl
						<< "No graphical output will be created." << std::endl
						<< GLOGGING;
					}
						break;
				}

				gpd_oa_.setCanvasDimensions(canvasDimensions_);
			}
				break;

			case Gem::Geneva::INFOPROCESSING:
			{
				bool isDirty = true;
				double primaryFitness;

				if(monitorBestOnly_) { // Monitor the best individuals only
					std::shared_ptr<GParameterSet> p = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
					if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::useRawEvaluation_) {
						primaryFitness = p->fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
					} else {
						primaryFitness = p->transformedFitness();
					}

					if(!monitorValidOnly_ || p->isValid()) {
						switch(this->nProfileVars()) {
							case 1:
							{
								fp_type val0    = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[0].var);

								if(observeBoundaries_) {
									if(
										val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
										) {
										progressPlotter2D_oa_->add(std::tuple<double,double>(double(val0), primaryFitness));
									}
								} else {
									progressPlotter2D_oa_->add(std::tuple<double,double>(double(val0), primaryFitness));
								}
							}
								break;

							case 2:
							{
								fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[0].var);
								fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[1].var);

								if(observeBoundaries_) {
									if(
										val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
										&& val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
										) {
										progressPlotter3D_oa_->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
									}
								} else {
									progressPlotter3D_oa_->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
								}
							}
								break;

							case 3:
							{
								fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[0].var);
								fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[1].var);
								fp_type val2 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[2].var);

								if(observeBoundaries_) {
									if(
										val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
										&& val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
										&& val2 >= fp_profVarVec_[2].lowerBoundary && val2 <= fp_profVarVec_[2].upperBoundary
										) {
										progressPlotter4D_oa_->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
									}
								} else {
									progressPlotter4D_oa_->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
								}
							}
								break;

							default: // Do nothing by default. The number of profiling dimensions is too large
								break;
						}
					}
				} else { // Monitor all individuals
					typename Gem::Geneva::GOptimizationAlgorithmT<ind_type>::iterator it;
					for(it=goa->begin(); it!=goa->end(); ++it) {

						if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::useRawEvaluation_) {
							primaryFitness = (*it)->fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
						} else {
							primaryFitness = (*it)->fitness(0, PREVENTREEVALUATION, USETRANSFORMEDFITNESS);
						}

						if(!monitorValidOnly_ || (*it)->isValid()) {
							switch(this->nProfileVars()) {
								case 1:
								{
									fp_type val0    = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[0].var);

									if(observeBoundaries_) {
										if(
											val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
											) {
											progressPlotter2D_oa_->add(std::tuple<double,double>(double(val0), primaryFitness));
										}
									} else {
										progressPlotter2D_oa_->add(std::tuple<double,double>(double(val0), primaryFitness));
									}
								}
									break;

								case 2:
								{
									fp_type val0 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[0].var);
									fp_type val1 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[1].var);

									if(observeBoundaries_) {
										if(
											val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
											&& val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
											) {
											progressPlotter3D_oa_->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
										}
									} else {
										progressPlotter3D_oa_->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
									}
								}
									break;

								case 3:
								{
									fp_type val0 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[0].var);
									fp_type val1 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[1].var);
									fp_type val2 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[2].var);

									if(observeBoundaries_) {
										if(
											val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
											&& val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
											&& val2 >= fp_profVarVec_[2].lowerBoundary && val2 <= fp_profVarVec_[2].upperBoundary
											) {
											progressPlotter4D_oa_->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
										}
									} else {
										progressPlotter4D_oa_->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
									}
								}
									break;

								default: // Do nothing by default. The number of profiling dimensions is too large
									break;
							}
						}
					}
				}
			}
				break;

			case Gem::Geneva::INFOEND:
			{
				// Make sure 1-D data is sorted
				if(1 == this->nProfileVars()) {
					progressPlotter2D_oa_->sortX();
				}

				// Inform the plot designer whether it should print png files
				gpd_oa_.setAddPrintCommand(addPrintCommand_);

				// Write out the result. Note that we add
				gpd_oa_.writeToFile(fileName_);

				// Remove all plotters
				gpd_oa_.resetPlotters();
				progressPlotter2D_oa_.reset();
				progressPlotter3D_oa_.reset();
				progressPlotter4D_oa_.reset();
			}
				break;

			default:
			{
				glogger
				<< "In GProgressPlotterT<ind_type, fp_type>::informationFunction(): Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
			}
				break;
		};
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GProgressPlotterT<>");
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GProgressPlotterT<ind_type, fp_type reference independent of this object and convert the pointer
		const GProgressPlotterT<ind_type, fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GProgressPlotterT", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(fp_profVarVec_, p_load->fp_profVarVec_), token);
		compare_t(IDENTITY(gpd_oa_, p_load->gpd_oa_), token);
		compare_t(IDENTITY(progressPlotter2D_oa_, p_load->progressPlotter2D_oa_), token);
		compare_t(IDENTITY(progressPlotter3D_oa_, p_load->progressPlotter3D_oa_), token);
		compare_t(IDENTITY(progressPlotter4D_oa_, p_load->progressPlotter4D_oa_), token);
		compare_t(IDENTITY(fileName_, p_load->fileName_), token);
		compare_t(IDENTITY(canvasDimensions_, p_load->canvasDimensions_), token);
		compare_t(IDENTITY(monitorBestOnly_, p_load->monitorBestOnly_), token);
		compare_t(IDENTITY(monitorValidOnly_, p_load->monitorValidOnly_), token);
		compare_t(IDENTITY(observeBoundaries_, p_load->observeBoundaries_), token);
		compare_t(IDENTITY(addPrintCommand_, p_load->addPrintCommand_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GProgressPlotterT<ind_type, fp_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GProgressPlotterT<ind_type, fp_type reference independent of this object and convert the pointer
		const GProgressPlotterT<ind_type, fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... and then our local data
		Gem::Common::copyCloneableObjectsContainer(p_load->fp_profVarVec_, fp_profVarVec_);
		gpd_oa_.load(p_load->gpd_oa_);
		copyCloneableSmartPointer(p_load->progressPlotter2D_oa_, progressPlotter2D_oa_);
		copyCloneableSmartPointer(p_load->progressPlotter3D_oa_, progressPlotter3D_oa_);
		copyCloneableSmartPointer(p_load->progressPlotter4D_oa_, progressPlotter4D_oa_);
		fileName_ = p_load->fileName_;
		canvasDimensions_ = p_load->canvasDimensions_;
		monitorBestOnly_ = p_load->monitorBestOnly_;
		monitorValidOnly_ = p_load->monitorValidOnly_;
		observeBoundaries_ = p_load->observeBoundaries_;
		addPrintCommand_ = p_load->addPrintCommand_;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GProgressPlotterT<ind_type, fp_type>(*this);
	}

private:
	std::vector<parPropSpec<fp_type>> fp_profVarVec_; ///< Holds information about variables to be profiled

	Gem::Common::GPlotDesigner gpd_oa_; ///< A wrapper for the plots

	// These are temporaries
	std::shared_ptr<Gem::Common::GGraph2D> progressPlotter2D_oa_;
	std::shared_ptr<Gem::Common::GGraph3D> progressPlotter3D_oa_;
	std::shared_ptr<Gem::Common::GGraph4D> progressPlotter4D_oa_;

	std::string fileName_ = std::string("progressScan.C"); ///< The name of the file the output should be written to. Note that the class will add the name of the algorithm it acts on
	std::tuple<std::uint32_t,std::uint32_t> canvasDimensions_; ///< The dimensions of the canvas

	bool monitorBestOnly_ = false;  ///< Indicates whether only the best individuals should be monitored
	bool monitorValidOnly_ = false; ///< Indicates whether only valid individuals should be plotted
	bool observeBoundaries_ = false; ///< When set to true, the plotter will ignore values outside of a scan boundary

	bool addPrintCommand_ = false; ///< Asks the GPlotDesigner to add a print command to result files

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GProgressPlotterT<ind_type, fp_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GProgressPlotterT<ind_type, fp_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GProgressPlotterT<ind_type, fp_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log all candidate solutions found to a file, including the parameetr
 * values. NOTE that the file may become very large! Results are output in the following format:
 * param1 param2 ... param_m eval1 eval2 ... eval_n . By default, no commas and
 * explanations are printed. If withNameAndType is set to true, the values are
 * prepended by a line with variable names and types. If withCommas is set to true,
 * commas will be printed in-between values. It is possible to filter the results by
 * asking the class to only log solutions better than a given set of values. What
 * is considered better depends on whether evaluation criteria are maximized or minimized
 * and is determined from the individual. Note that this class can only be instantiated
 * if individual_type is either a derivative of GParamterSet or is an object of the
 * GParameterSet class itself.
 */
template <typename ind_type>
class GAllSolutionFileLoggerT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOMT",	boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this))
		& BOOST_SERIALIZATION_NVP(fileName_)
		& BOOST_SERIALIZATION_NVP(boundaries_)
		& BOOST_SERIALIZATION_NVP(boundariesActive_)
		& BOOST_SERIALIZATION_NVP(withNameAndType_)
		& BOOST_SERIALIZATION_NVP(withCommas_)
		& BOOST_SERIALIZATION_NVP(useRawFitness_)
		& BOOST_SERIALIZATION_NVP(showValidity_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if individual_type is a derivative of GParameterSet
	static_assert(
		std::is_base_of<GParameterSet, ind_type>::value
		, "GParameterSet is no base class of ind_type"
	);

public:
	/***************************************************************************/
	/**
	 * The default constructor. Note that some variables may be initialized in the class body.
	 */
	GAllSolutionFileLoggerT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a file name. Note that some variables may be initialized in the class body.
	 */
	GAllSolutionFileLoggerT(const std::string& fileName)
		: fileName_(fileName)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a file name and boundaries.
	 * Note that some variables may be initialized in the class body.
	 */
	GAllSolutionFileLoggerT(
		const std::string& fileName
		, const std::vector<double>& boundaries
	)
		: fileName_(fileName)
		, boundaries_(boundaries)
		, boundariesActive_(true)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GAllSolutionFileLoggerT(const GAllSolutionFileLoggerT<ind_type>& cp)
		: fileName_(cp.fileName_)
		, boundaries_(cp.boundaries_)
		, boundariesActive_(cp.boundariesActive_)
		, withNameAndType_(cp.withNameAndType_)
		, withCommas_(cp.withCommas_)
		, useRawFitness_(cp.useRawFitness_)
		, showValidity_(cp.showValidity_)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GAllSolutionFileLoggerT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GAllSolutionFileLoggerT<ind_type>& operator=(const GAllSolutionFileLoggerT<ind_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GAllSolutionFileLoggerT<ind_type> object
	 *
	 * @param  cp A constant reference to another GAllSolutionFileLoggerT<ind_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GAllSolutionFileLoggerT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GAllSolutionFileLoggerT<ind_type> object
	 *
	 * @param  cp A constant reference to another GAllSolutionFileLoggerT<ind_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GAllSolutionFileLoggerT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GAllSolutionFileLoggerT<>");
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GAllSolutionFileLoggerT<ind_type> reference independent of this object and convert the pointer
		const GAllSolutionFileLoggerT<ind_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAllSolutionFileLoggerT<ind_type>>(cp, this);

		GToken token("GAllSolutionFileLoggerT", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(fileName_, p_load->fileName_), token);
		compare_t(IDENTITY(boundaries_, p_load->boundaries_), token);
		compare_t(IDENTITY(boundariesActive_, p_load->boundariesActive_), token);
		compare_t(IDENTITY(withNameAndType_, p_load->withNameAndType_), token);
		compare_t(IDENTITY(withCommas_, p_load->withCommas_), token);
		compare_t(IDENTITY(useRawFitness_, p_load->useRawFitness_), token);
		compare_t(IDENTITY(showValidity_, p_load->showValidity_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		fileName_ = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return fileName_;
	}

	/***************************************************************************/
	/**
	 * Sets the boundaries
	 */
	void setBoundaries(std::vector<double> boundaries) {
		boundaries_ = boundaries;
		boundariesActive_ = true;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the boundaries
	 */
	std::vector<double> getBoundaries() const {
		return boundaries_;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether boundaries are active
	 */
	bool boundariesActive() const {
		return boundariesActive_;
	}

	/***************************************************************************/
	/**
	 * Allows to inactivate boundaries
	 */
	void setBoundariesInactive() {
		boundariesActive_ = false;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether explanations should be printed for parameter-
	 * and fitness values.
	 */
	void setPrintWithNameAndType(bool withNameAndType) {
		withNameAndType_ = withNameAndType;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether explanations should be printed for parameter-
	 * and fitness values
	 */
	bool getPrintWithNameAndType() const {
		return withNameAndType_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether commas should be printed in-between values
	 */
	void setPrintWithCommas(bool withCommas) {
		withCommas_ = withCommas;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether commas should be printed in-between values
	 */
	bool getPrintWithCommas() const {
		return withCommas_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether the true (instead of the transformed) fitness should be shown
	 */
	void setUseTrueFitness(bool useRawFitness) {
		useRawFitness_ = useRawFitness;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
	 */
	bool getUseTrueFitness() const {
		return useRawFitness_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether the validity of a solution should be shown
	 */
	void setShowValidity(bool showValidity) {
		showValidity_ = showValidity;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether the validity of a solution will be shown
	 */
	bool getShowValidity() const {
		return showValidity_;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		switch(im) {
			case Gem::Geneva::INFOINIT:
			{
				// If the file pointed to by fileName_ already exists, make a back-up
				if(bf::exists(fileName_)) {
					const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
					std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

					glogger
					<< "In GAllSolutionFileLoggerT<T>::informationFunction(): Error!" << std::endl
					<< "Attempt to output information to file " << fileName_ << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(fileName_, newFileName);
				}
			}
				break;

			case Gem::Geneva::INFOPROCESSING:
			{
				// Open the external file
				boost::filesystem::ofstream data(fileName_, std::ofstream::app);

				// Loop over all individuals of the algorithm.
				for(std::size_t pos=0; pos<goa->size(); pos++) {
					std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

					// Note that isGoodEnough may throw if loop acts on a "dirty" individual
					if(!boundariesActive_ || ind->isGoodEnough(boundaries_)) {
						// Append the data to the external file
						if(0 == pos && goa->inFirstIteration()) { // Only output name and type in the very first line (if at all)
							data << ind->toCSV(withNameAndType_, withCommas_, useRawFitness_, showValidity_);
						} else {
							data << ind->toCSV(false, withCommas_, useRawFitness_, showValidity_);
						}
					}
				}

				// Close the external file
				data.close();
			}
				break;

			case Gem::Geneva::INFOEND:
				// nothing
				break;

			default:
			{
				glogger
				<< "In GAllSolutionFileLoggerT<ind_type>: Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
			}
				break;
		};
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GAllSolutionFileLoggerT<ind_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GAllSolutionFileLoggerT<ind_type> reference independent of this object and convert the pointer
		const GAllSolutionFileLoggerT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... and then our local data
		fileName_ = p_load->fileName_;
		boundaries_ = p_load->boundaries_;
		boundariesActive_ = p_load->boundariesActive_;
		withNameAndType_ = p_load->withNameAndType_;
		withCommas_ = p_load->withCommas_;
		useRawFitness_ = p_load->useRawFitness_;
		showValidity_ = p_load->showValidity_;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GAllSolutionFileLoggerT<ind_type>(*this);
	}

private:
	/***************************************************************************/

	std::string fileName_ = "CompleteSolutionLog.txt"; ///< The name of the file to which solutions should be stored
	std::vector<double> boundaries_; ///< Value boundaries used to filter logged solutions
	bool boundariesActive_ = false; ///< Set to true if boundaries have been set
	bool withNameAndType_ = false; ///< When set to true, explanations for values are printed
	bool withCommas_ = false; ///< When set to true, commas will be printed in-between values
	bool useRawFitness_ = true; ///< Indicates whether true- or transformed fitness should be output
	bool showValidity_ = true; ///< Indicates whether the validity of a solution should be shown

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAllSolutionFileLoggerT<ind_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAllSolutionFileLoggerT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAllSolutionFileLoggerT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class prints out all evaluations of each iteration. The format is
 * eval0_0, eval0_1, ... ,eval0_n, ..., evalm_0, evalm_1, ... ,evalm_n
 */
template <typename ind_type>
class GIterationResultsFileLoggerT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOMT",	boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this))
		& BOOST_SERIALIZATION_NVP(fileName_)
		& BOOST_SERIALIZATION_NVP(withCommas_)
		& BOOST_SERIALIZATION_NVP(useRawFitness_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if individual_type is a derivative of GParameterSet
	static_assert(
		std::is_base_of<GParameterSet, ind_type>::value
		, "GParameterSet is no base class of ind_type"
	);

public:
	/***************************************************************************/
	/**
	 * The default constructor. Note that some variables may be initialized
	 * in the class body.
	 */
	GIterationResultsFileLoggerT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a file name. Note that some variables may be initialized
	 * in the class body.
	 */
	GIterationResultsFileLoggerT(const std::string& fileName)
		: fileName_(fileName)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GIterationResultsFileLoggerT(const GIterationResultsFileLoggerT<ind_type>& cp)
		: fileName_(cp.fileName_)
		, withCommas_(cp.withCommas_)
		, useRawFitness_(cp.useRawFitness_)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GIterationResultsFileLoggerT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GIterationResultsFileLoggerT<ind_type>& operator=(const GIterationResultsFileLoggerT<ind_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GIterationResultsFileLoggerT<ind_type> object
	 *
	 * @param  cp A constant reference to another GIterationResultsFileLoggerT<ind_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GIterationResultsFileLoggerT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GIterationResultsFileLoggerT<ind_type> object
	 *
	 * @param  cp A constant reference to another GIterationResultsFileLoggerT<ind_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GIterationResultsFileLoggerT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GIterationResultsFileLoggerT<>");
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GIterationResultsFileLoggerT<ind_type>
		// reference independent of this object and convert the pointer
		const GIterationResultsFileLoggerT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GIterationResultsFileLoggerT", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(fileName_, p_load->fileName_), token);
		compare_t(IDENTITY(withCommas_, p_load->withCommas_), token);
		compare_t(IDENTITY(useRawFitness_, p_load->useRawFitness_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		fileName_ = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return fileName_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether commas should be printed in-between values
	 */
	void setPrintWithCommas(bool withCommas) {
		withCommas_ = withCommas;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether commas should be printed in-between values
	 */
	bool getPrintWithCommas() const {
		return withCommas_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether the true (instead of the transformed) fitness should be shown
	 */
	void setUseTrueFitness(bool useRawFitness) {
		useRawFitness_ = useRawFitness;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
	 */
	bool getUseTrueFitness() const {
		return useRawFitness_;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		switch(im) {
			case Gem::Geneva::INFOINIT:
			{
				// If the file pointed to by fileName_ already exists, make a back-up
				if(bf::exists(fileName_)) {
					const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
					std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

					glogger
					<< "In GIterationResultsFileLoggerT<T>::informationFunction(): Warning!" << std::endl
					<< "Attempt to output information to file " << fileName_ << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(fileName_, newFileName);
				}
			}
				break;

			case Gem::Geneva::INFOPROCESSING:
			{
				// Open the external file
				boost::filesystem::ofstream data(fileName_.c_str(), std::ofstream::app);
				std::vector<double> fitnessVec;

				// Loop over all individuals of the algorithm.
				std::size_t nIndividuals = goa->size();
				for(std::size_t pos=0; pos<nIndividuals; pos++) {
					std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
					fitnessVec = goa->at(pos)->fitnessVec(useRawFitness_);

					std::size_t nFitnessCriteria = goa->getNumberOfFitnessCriteria();
					for(std::size_t i=0; i<nFitnessCriteria; i++) {
						data << fitnessVec.at(i) << ((withCommas_ && (nFitnessCriteria*nIndividuals > (i+1)*(pos+1)))?", ":" ");
					}
				}
				data << std::endl;

				// Close the external file
				data.close();
			}
				break;

			case Gem::Geneva::INFOEND:
				// nothing
				break;

			default:
			{
				glogger
				<< "In GIterationResultsFileLoggerT<T>: Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
			}
				break;
		};
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GIterationResultsFileLoggerT<ind_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GIterationResultsFileLoggerT<ind_type>
		// reference independent of this object and convert the pointer
		const GIterationResultsFileLoggerT<ind_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... and then our local data
		fileName_ = p_load->fileName_;
		withCommas_ = p_load->withCommas_;
		useRawFitness_ = p_load->useRawFitness_;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GIterationResultsFileLoggerT<ind_type>(*this);
	}

private:
	/***************************************************************************/

	std::string fileName_ = "IterationResultsLog.txt"; ///< The name of the file to which solutions should be stored
	bool withCommas_ = true; ///< When set to true, commas will be printed in-between values
	bool useRawFitness_ = false; ///< Indicates whether true- or transformed fitness should be output

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIterationResultsFileLoggerT<ind_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIterationResultsFileLoggerT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIterationResultsFileLoggerT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log the number of adaptions made inside of adaptors
 * to a file. This is mostly needed for debugging and profiling purposes. The
 * number of adaptions made is a good measure for the adaption probability.
 */
template <typename ind_type>
class GNAdpationsLoggerT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOMT",	boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this))
		& BOOST_SERIALIZATION_NVP(fileName_)
		& BOOST_SERIALIZATION_NVP(canvasDimensions_)
		& BOOST_SERIALIZATION_NVP(gpd_oa_)
	  	& BOOST_SERIALIZATION_NVP(nAdaptionsHist2D_oa_)
	 	& BOOST_SERIALIZATION_NVP(nAdaptionsGraph2D_oa_)
		& BOOST_SERIALIZATION_NVP(fitnessGraph2D_oa_)
		& BOOST_SERIALIZATION_NVP(monitorBestOnly_)
		& BOOST_SERIALIZATION_NVP(addPrintCommand_)
		& BOOST_SERIALIZATION_NVP(maxIteration_)
		& BOOST_SERIALIZATION_NVP(nIterationsRecorded_)
		& BOOST_SERIALIZATION_NVP(nAdaptionsStore_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if individual_type is a derivative of GParameterSet
	static_assert(
		std::is_base_of<GParameterSet, ind_type>::value
		, "GParameterSet is no base class of ind_type"
	);

public:
	/***************************************************************************/
	/**
	 * The default constructor. Note that some variables may be
	 * initialized in the class body.
	 */
	GNAdpationsLoggerT()
		: canvasDimensions_(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, gpd_oa_("Number of adaptions per iteration", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a file name. Note that some variables may be
	 * initialized in the class body.
	 */
	explicit GNAdpationsLoggerT(const std::string& fileName)
		: fileName_(fileName)
		, canvasDimensions_(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, gpd_oa_("Number of adaptions per iteration", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GNAdpationsLoggerT(const GNAdpationsLoggerT<ind_type>& cp)
		: fileName_(cp.fileName_)
		, canvasDimensions_(cp.canvasDimensions_)
		, gpd_oa_(cp.gpd_oa_)
		, monitorBestOnly_(cp.monitorBestOnly_)
		, addPrintCommand_(cp.addPrintCommand_)
		, maxIteration_(cp.maxIteration_)
		, nIterationsRecorded_(cp.nIterationsRecorded_)
		, nAdaptionsStore_(cp.nAdaptionsStore_)
	{
		Gem::Common::copyCloneableSmartPointer(cp.nAdaptionsHist2D_oa_, nAdaptionsHist2D_oa_);
		Gem::Common::copyCloneableSmartPointer(cp.nAdaptionsGraph2D_oa_, nAdaptionsGraph2D_oa_);
		Gem::Common::copyCloneableSmartPointer(cp.fitnessGraph2D_oa_, fitnessGraph2D_oa_);
	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GNAdpationsLoggerT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GNAdpationsLoggerT<ind_type>& operator=(const GNAdpationsLoggerT<ind_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GNAdpationsLoggerT<ind_type> object
	 *
	 * @param  cp A constant reference to another GNAdpationsLoggerT<ind_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GNAdpationsLoggerT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GNAdpationsLoggerT<ind_type> object
	 *
	 * @param  cp A constant reference to another GNAdpationsLoggerT<ind_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GNAdpationsLoggerT<ind_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GNAdpationsLoggerT<ind_type> reference independent of this object and convert the pointer
		const GNAdpationsLoggerT<ind_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNAdpationsLoggerT<ind_type>>(cp, this);

		GToken token("GNAdpationsLoggerT", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(fileName_, p_load->fileName_), token);
		compare_t(IDENTITY(canvasDimensions_, p_load->canvasDimensions_), token);
		compare_t(IDENTITY(gpd_oa_, p_load->gpd_oa_), token);
		compare_t(IDENTITY(nAdaptionsHist2D_oa_, p_load->nAdaptionsHist2D_oa_), token);
		compare_t(IDENTITY(nAdaptionsGraph2D_oa_, p_load->nAdaptionsGraph2D_oa_), token);
		compare_t(IDENTITY(fitnessGraph2D_oa_, p_load->fitnessGraph2D_oa_), token);
		compare_t(IDENTITY(monitorBestOnly_, p_load->monitorBestOnly_), token);
		compare_t(IDENTITY(addPrintCommand_, p_load->addPrintCommand_), token);
		compare_t(IDENTITY(maxIteration_, p_load->maxIteration_), token);
		compare_t(IDENTITY(nIterationsRecorded_, p_load->nIterationsRecorded_), token);
		compare_t(IDENTITY(nAdaptionsStore_, p_load->nAdaptionsStore_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		fileName_ = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return fileName_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only the best individuals should be monitored.
	 */
	void setMonitorBestOnly(bool monitorBestOnly = true) {
		monitorBestOnly_ = monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only the best individuals should be monitored.
	 */
	bool getMonitorBestOnly() const {
		return monitorBestOnly_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions
	 */
	void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		canvasDimensions_ = canvasDimensions;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions using separate x and y values
	 */
	void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		canvasDimensions_ = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	}

	/***************************************************************************/
	/**
	 * Gives access to the canvas dimensions
	 */
	std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		return canvasDimensions_;
	}

	/******************************************************************************/
	/**
	 * Allows to add a "Print" command to the end of the script so that picture files are created
	 */
	void setAddPrintCommand(bool addPrintCommand) {
		addPrintCommand_ = addPrintCommand;
	}

	/******************************************************************************/
	/**
	 * Allows to retrieve the current value of the addPrintCommand_ variable
	 */
	bool getAddPrintCommand() const {
		return addPrintCommand_;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		using namespace Gem::Common;

		switch(im) {
			case Gem::Geneva::INFOINIT:
			{
				// If the file pointed to by fileName_ already exists, make a back-up
				if(bf::exists(fileName_)) {
					const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
					std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

					glogger
					<< "In GNAdpationsLoggerT<T>::informationFunction(): Error!" << std::endl
					<< "Attempt to output information to file " << fileName_ << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(fileName_, newFileName);
				}

				// Make sure the progress plotter has the desired size
				gpd_oa_.setCanvasDimensions(canvasDimensions_);

				// Set up a graph to monitor the best fitness found
				fitnessGraph2D_oa_ = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
				fitnessGraph2D_oa_->setXAxisLabel("Iteration");
				fitnessGraph2D_oa_->setYAxisLabel("Fitness");
				fitnessGraph2D_oa_->setPlotMode(Gem::Common::CURVE);
			}
				break;

			case Gem::Geneva::INFOPROCESSING:
			{
				std::uint32_t iteration = goa->getIteration();

				// Record the current fitness
				std::shared_ptr<GParameterSet> p = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
				(*fitnessGraph2D_oa_) & std::tuple<double,double>(double(iteration), double(p->fitness()));

				// Update the largest known iteration and the number of recorded iterations
				maxIteration_ = iteration;
				nIterationsRecorded_++;

				// Do the actual logging
				if(monitorBestOnly_) {
					std::shared_ptr<GParameterSet> best = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
					nAdaptionsStore_.push_back(std::tuple<double,double>(double(iteration), double(best->getNAdaptions())));
				} else { // Monitor all individuals
					// Loop over all individuals of the algorithm.
					for(std::size_t pos=0; pos<goa->size(); pos++) {
						std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
						nAdaptionsStore_.push_back(std::tuple<double,double>(double(iteration), double(ind->getNAdaptions())));
					}
				}
			}
				break;

			case Gem::Geneva::INFOEND:
			{
				std::vector<std::tuple<double, double>>::iterator it;

				if(monitorBestOnly_) {
					// Create the graph object
					nAdaptionsGraph2D_oa_ = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
					nAdaptionsGraph2D_oa_->setXAxisLabel("Iteration");
					nAdaptionsGraph2D_oa_->setYAxisLabel("Number of parameter adaptions");
					nAdaptionsGraph2D_oa_->setPlotMode(Gem::Common::CURVE);

					// Fill the object with data
					for(it=nAdaptionsStore_.begin(); it!=nAdaptionsStore_.end(); ++it) {
						(*nAdaptionsGraph2D_oa_) & *it;
					}

					// Add the histogram to the plot designer
					gpd_oa_.registerPlotter(nAdaptionsGraph2D_oa_);

				} else { // All individuals are monitored
					// Within nAdaptionsStore_, find the largest number of adaptions performed
					std::size_t maxNAdaptions = 0;
					for(it=nAdaptionsStore_.begin(); it!=nAdaptionsStore_.end(); ++it) {
						if(std::get<1>(*it) > maxNAdaptions) {
							maxNAdaptions = boost::numeric_cast<std::size_t>(std::get<1>(*it));
						}
					}

					// Create the histogram object
					nAdaptionsHist2D_oa_ = std::shared_ptr<GHistogram2D>(
						new GHistogram2D(
							nIterationsRecorded_
							, maxNAdaptions+1
							, 0., double(maxIteration_)
							, 0., double(maxNAdaptions)
						)
					);

					nAdaptionsHist2D_oa_->setXAxisLabel("Iteration");
					nAdaptionsHist2D_oa_->setYAxisLabel("Number of parameter adaptions");
					nAdaptionsHist2D_oa_->setDrawingArguments("BOX");

					// Fill the object with data
					for(it=nAdaptionsStore_.begin(); it!=nAdaptionsStore_.end(); ++it) {
						(*nAdaptionsHist2D_oa_) & *it;
					}

					// Add the histogram to the plot designer
					gpd_oa_.registerPlotter(nAdaptionsHist2D_oa_);
				}

				// Add the fitness monitor
				gpd_oa_.registerPlotter(fitnessGraph2D_oa_);

				// Inform the plot designer whether it should print png files
				gpd_oa_.setAddPrintCommand(addPrintCommand_);

				// Write out the result. Note that we add
				gpd_oa_.writeToFile(fileName_);

				// Remove all plotters
				gpd_oa_.resetPlotters();
				nAdaptionsHist2D_oa_.reset();
				nAdaptionsGraph2D_oa_.reset();
			}
				break;

			default:
			{
				glogger
				<< "In GNAdpationsLoggerT<T>>: Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
			}
				break;
		};
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GNAdpationsLoggerT<ind_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GNAdpationsLoggerT<ind_type> reference independent of this object and convert the pointer
		const GNAdpationsLoggerT<ind_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNAdpationsLoggerT<ind_type>>(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... and then our local data
		fileName_ = p_load->fileName_;
		canvasDimensions_ = p_load->canvasDimensions_;
		gpd_oa_ = p_load->gpd_oa_;
		Gem::Common::copyCloneableSmartPointer(p_load->nAdaptionsHist2D_oa_, nAdaptionsHist2D_oa_);
		Gem::Common::copyCloneableSmartPointer(p_load->nAdaptionsGraph2D_oa_, nAdaptionsGraph2D_oa_);
		Gem::Common::copyCloneableSmartPointer(p_load->fitnessGraph2D_oa_, fitnessGraph2D_oa_);
		monitorBestOnly_ = p_load->monitorBestOnly_;
		addPrintCommand_ = p_load->addPrintCommand_;
		maxIteration_ = p_load->maxIteration_;
		nIterationsRecorded_ = p_load->nIterationsRecorded_;
		nAdaptionsStore_ = p_load->nAdaptionsStore_;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GNAdpationsLoggerT<ind_type>(*this);
	}

private:
	/***************************************************************************/

	std::string fileName_ = "NAdaptions.C"; ///< The name of the file to which solutions should be stored

	std::tuple<std::uint32_t,std::uint32_t> canvasDimensions_; ///< The dimensions of the canvas

	Gem::Common::GPlotDesigner gpd_oa_; ///< A wrapper for the plots

	std::shared_ptr<Gem::Common::GHistogram2D> nAdaptionsHist2D_oa_;  ///< Holds the actual histogram
	std::shared_ptr<Gem::Common::GGraph2D>     nAdaptionsGraph2D_oa_; ///< Used if we only monitor the best solution in each iteration
	std::shared_ptr<Gem::Common::GGraph2D>     fitnessGraph2D_oa_;    ///< Lets us monitor the current fitness of the population

	bool monitorBestOnly_ = false; ///< Indicates whether only the best individuals should be monitored
	bool addPrintCommand_ = false; ///< Asks the GPlotDesigner to add a print command to result files

	std::size_t maxIteration_ = 0; ///< Holds the largest iteration recorded for the algorithm
	std::size_t nIterationsRecorded_ = 0; ///< Holds the number of iterations that were recorded (not necessarily == maxIteration_

	std::vector<std::tuple<double, double>> nAdaptionsStore_; ///< Holds all information about the number of adaptions

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNAdpationsLoggerT<ind_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNAdpationsLoggerT<ind_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNAdpationsLoggerT<ind_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log chosen properties of adaptors. Such properties
 * are limited to numeric entities, that may be converted to double
 */
template <typename ind_type, typename num_type>
class GAdaptorPropertyLoggerT
	: public GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOMT",	boost::serialization::base_object<GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(*this))
		& BOOST_SERIALIZATION_NVP(fileName_)
		& BOOST_SERIALIZATION_NVP(adaptorName_)
		& BOOST_SERIALIZATION_NVP(property_)
		& BOOST_SERIALIZATION_NVP(canvasDimensions_)
		& BOOST_SERIALIZATION_NVP(gpd_oa_)
		& BOOST_SERIALIZATION_NVP(adaptorPropertyHist2D_oa_)
		& BOOST_SERIALIZATION_NVP(fitnessGraph2D_oa_)
		& BOOST_SERIALIZATION_NVP(monitorBestOnly_)
		& BOOST_SERIALIZATION_NVP(addPrintCommand_)
		& BOOST_SERIALIZATION_NVP(maxIteration_)
		& BOOST_SERIALIZATION_NVP(nIterationsRecorded_)
		& BOOST_SERIALIZATION_NVP(adaptorPropertyStore_);
	}
	///////////////////////////////////////////////////////////////////////

	// Make sure this class can only be instantiated if individual_type is a derivative of GParameterSet
	static_assert(
		std::is_base_of<GParameterSet, ind_type>::value
		, "GParameterSet is no base class of ind_type"
	);

	static_assert(
		std::is_arithmetic<num_type>::value
		, "num_type should be an arithmetic type"
	);

public:
	/***************************************************************************/
	/**
	 * The default constructor. Note that some parmeters may be initialized in
	 * the class body.
	 */
	GAdaptorPropertyLoggerT()
		: canvasDimensions_(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, gpd_oa_("Adaptor properties", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a file name
	 */
	GAdaptorPropertyLoggerT(
		const std::string& fileName
		, const std::string& adaptorName
		, const std::string& property
	)
		: fileName_(fileName)
		, adaptorName_(adaptorName)
		, property_(property)
		, canvasDimensions_(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, gpd_oa_("Adaptor properties", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GAdaptorPropertyLoggerT(const GAdaptorPropertyLoggerT<ind_type, num_type>& cp)
		: fileName_(cp.fileName_)
		, adaptorName_(cp.adaptorName_)
		, property_(cp.property_)
		, canvasDimensions_(cp.canvasDimensions_)
		, gpd_oa_(cp.gpd_oa_)
		, monitorBestOnly_(cp.monitorBestOnly_)
		, addPrintCommand_(cp.addPrintCommand_)
		, maxIteration_(cp.maxIteration_)
		, nIterationsRecorded_(cp.nIterationsRecorded_)
		, adaptorPropertyStore_(cp.adaptorPropertyStore_)
	{
		// Copy the smart pointers over
		Gem::Common::copyCloneableSmartPointer(cp.adaptorPropertyHist2D_oa_, adaptorPropertyHist2D_oa_);
		Gem::Common::copyCloneableSmartPointer(cp.fitnessGraph2D_oa_, fitnessGraph2D_oa_);
	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GAdaptorPropertyLoggerT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GAdaptorPropertyLoggerT<ind_type, num_type>& operator=(const GAdaptorPropertyLoggerT<ind_type, num_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GAdaptorPropertyLoggerT<ind_type, num_type> object
	 *
	 * @param  cp A constant reference to another GAdaptorPropertyLoggerT<ind_type, num_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GAdaptorPropertyLoggerT<ind_type, num_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GAdaptorPropertyLoggerT<ind_type, num_type> object
	 *
	 * @param  cp A constant reference to another GAdaptorPropertyLoggerT<ind_type, num_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GAdaptorPropertyLoggerT<ind_type, num_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GAdaptorPropertyLoggerT<>");
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GAdaptorPropertyLoggerT<ind_type, num_type> reference independent of this object and convert the pointer
		const GAdaptorPropertyLoggerT<ind_type, num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerT<ind_type, num_type>>(cp, this);

		GToken token("GAdaptorPropertyLoggerT", e);

		// Compare our parent data ...
		Gem::Common::compare_base<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(fileName_, p_load->fileName_), token);
		compare_t(IDENTITY(adaptorName_, p_load->adaptorName_), token);
		compare_t(IDENTITY(property_, p_load->property_), token);
		compare_t(IDENTITY(canvasDimensions_, p_load->canvasDimensions_), token);
		compare_t(IDENTITY(gpd_oa_, p_load->gpd_oa_), token);
		compare_t(IDENTITY(adaptorPropertyHist2D_oa_, p_load->adaptorPropertyHist2D_oa_), token);
		compare_t(IDENTITY(fitnessGraph2D_oa_, p_load->fitnessGraph2D_oa_), token);
		compare_t(IDENTITY(monitorBestOnly_, p_load->monitorBestOnly_), token);
		compare_t(IDENTITY(addPrintCommand_, p_load->addPrintCommand_), token);
		compare_t(IDENTITY(maxIteration_, p_load->maxIteration_), token);
		compare_t(IDENTITY(nIterationsRecorded_, p_load->nIterationsRecorded_), token);
		compare_t(IDENTITY(adaptorPropertyStore_, p_load->adaptorPropertyStore_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		fileName_ = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return fileName_;
	}

	/***************************************************************************/
	/**
	 * Sets the name of the adaptor
	 */
	void setAdaptorName(std::string adaptorName) {
		adaptorName_ = adaptorName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the name of the adaptor
	 */
	std::string getAdaptorName() const {
		return adaptorName_;
	}

	/***************************************************************************/
	/**
	 * Sets the name of the property
	 */
	void setPropertyName(std::string property) {
		property_ = property;
	}

	/***************************************************************************/
	/**
	 * Retrieves the name of the property
	 */
	std::string getPropertyName() const {
		return property_;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only the best individuals should be monitored.
	 */
	void setMonitorBestOnly(bool monitorBestOnly = true) {
		monitorBestOnly_ = monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only the best individuals should be monitored.
	 */
	bool getMonitorBestOnly() const {
		return monitorBestOnly_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions
	 */
	void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		canvasDimensions_ = canvasDimensions;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions using separate x and y values
	 */
	void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		canvasDimensions_ = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	}

	/***************************************************************************/
	/**
	 * Gives access to the canvas dimensions
	 */
	std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		return canvasDimensions_;
	}

	/******************************************************************************/
	/**
	 * Allows to add a "Print" command to the end of the script so that picture files are created
	 */
	void setAddPrintCommand(bool addPrintCommand) {
		addPrintCommand_ = addPrintCommand;
	}

	/******************************************************************************/
	/**
	 * Allows to retrieve the current value of the addPrintCommand_ variable
	 */
	bool getAddPrintCommand() const {
		return addPrintCommand_;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, typename Gem::Geneva::GOptimizationAlgorithmT<ind_type> * const goa
	) override {
		using namespace Gem::Common;

		switch(im) {
			case Gem::Geneva::INFOINIT:
			{
				// If the file pointed to by fileName_ already exists, make a back-up
				if(bf::exists(fileName_)) {
					const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
					std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

					glogger
					<< "In GAdaptorPropertyLoggerT<S,T>::informationFunction(): Error!" << std::endl
					<< "Attempt to output information to file " << fileName_ << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(fileName_, newFileName);
				}

				// Make sure the progress plotter has the desired size
				gpd_oa_.setCanvasDimensions(canvasDimensions_);

				// Set up a graph to monitor the best fitness found
				fitnessGraph2D_oa_ = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
				fitnessGraph2D_oa_->setXAxisLabel("Iteration");
				fitnessGraph2D_oa_->setYAxisLabel("Fitness");
				fitnessGraph2D_oa_->setPlotMode(Gem::Common::CURVE);
			}
				break;

			case Gem::Geneva::INFOPROCESSING:
			{
				std::uint32_t iteration = goa->getIteration();

				// Record the current fitness
				std::shared_ptr<GParameterSet> p = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
				(*fitnessGraph2D_oa_) & std::tuple<double,double>(double(iteration), double(p->fitness()));

				// Update the largest known iteration and the number of recorded iterations
				maxIteration_ = iteration;
				nIterationsRecorded_++;

				// Will hold the adaptor properties
				std::vector<boost::any> data;

				// Do the actual logging
				if(monitorBestOnly_) {
					std::shared_ptr<GParameterSet> best = goa->GOptimizableI::template getBestIndividual<GParameterSet>();

					// Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
					best->queryAdaptor(adaptorName_, property_, data);

					// Attach the data to adaptorPropertyStore_
					std::vector<boost::any>::iterator prop_it;
					for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
						adaptorPropertyStore_.push_back(std::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
					}
				} else { // Monitor all individuals
					// Loop over all individuals of the algorithm.
					for(std::size_t pos=0; pos<goa->size(); pos++) {
						std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

						// Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
						ind->queryAdaptor(adaptorName_, property_, data);

						// Attach the data to adaptorPropertyStore_
						std::vector<boost::any>::iterator prop_it;
						for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
							adaptorPropertyStore_.push_back(std::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
						}
					}
				}
			}
				break;

			case Gem::Geneva::INFOEND:
			{
				std::vector<std::tuple<double, double>>::iterator it;

				// Within adaptorPropertyStore_, find the largest number of adaptions performed
				double maxProperty = 0.;
				for(it=adaptorPropertyStore_.begin(); it!=adaptorPropertyStore_.end(); ++it) {
					if(std::get<1>(*it) > maxProperty) {
						maxProperty = std::get<1>(*it);
					}
				}

				// Create the histogram object
				adaptorPropertyHist2D_oa_ = std::shared_ptr<GHistogram2D>(
					new GHistogram2D(
						nIterationsRecorded_
						, 100
						, 0., double(maxIteration_)
						, 0., maxProperty
					)
				);

				adaptorPropertyHist2D_oa_->setXAxisLabel("Iteration");
				adaptorPropertyHist2D_oa_->setYAxisLabel(std::string("Adaptor-Name: ") + adaptorName_ + std::string(", Property: ") + property_);
				adaptorPropertyHist2D_oa_->setDrawingArguments("BOX");

				// Fill the object with data
				for(it=adaptorPropertyStore_.begin(); it!=adaptorPropertyStore_.end(); ++it) {
					(*adaptorPropertyHist2D_oa_) & *it;
				}

				// Add the histogram to the plot designer
				gpd_oa_.registerPlotter(adaptorPropertyHist2D_oa_);

				// Add the fitness monitor
				gpd_oa_.registerPlotter(fitnessGraph2D_oa_);

				// Inform the plot designer whether it should print png files
				gpd_oa_.setAddPrintCommand(addPrintCommand_);

				// Write out the result. Note that we add
				gpd_oa_.writeToFile(fileName_);

				// Remove all plotters (they will survive inside of gpd)
				gpd_oa_.resetPlotters();
				adaptorPropertyHist2D_oa_.reset();
			}
				break;

			default:
			{
				glogger
				<< "In GAdaptorPropertyLoggerT<S,T>: Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
			}
				break;
		};
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GAdaptorPropertyLoggerT<ind_type, num_type object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GAdaptorPropertyLoggerT<ind_type, num_type> reference independent of this object and convert the pointer
		const GAdaptorPropertyLoggerT<ind_type, num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerT<ind_type, num_type>>(cp, this);

		// Load the parent classes' data ...
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::load_(cp);

		// ... and then our local data
		fileName_ = p_load->fileName_;
		adaptorName_ = p_load->adaptorName_;
		property_ = p_load->property_;
		canvasDimensions_ = p_load->canvasDimensions_;
		gpd_oa_ = p_load->gpd_oa_;
		Gem::Common::copyCloneableSmartPointer(p_load->adaptorPropertyHist2D_oa_, adaptorPropertyHist2D_oa_);
		Gem::Common::copyCloneableSmartPointer(p_load->fitnessGraph2D_oa_, fitnessGraph2D_oa_);
		monitorBestOnly_ = p_load->monitorBestOnly_;
		addPrintCommand_ = p_load->addPrintCommand_;
		maxIteration_ = p_load->maxIteration_;
		nIterationsRecorded_ = p_load->nIterationsRecorded_;
		adaptorPropertyStore_ = p_load->adaptorPropertyStore_;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GAdaptorPropertyLoggerT<ind_type, num_type>(*this);
	}

private:
	/***************************************************************************/

	std::string fileName_ = "NAdaptions.C"; ///< The name of the file to which solutions should be stored

	std::string adaptorName_ = "GDoubleGaussAdaptor"; ///< The  name of the adaptor for which properties should be logged
	std::string property_ = "sigma"; ///< The name of the property to be logged

	std::tuple<std::uint32_t,std::uint32_t> canvasDimensions_; ///< The dimensions of the canvas

	Gem::Common::GPlotDesigner gpd_oa_; ///< A wrapper for the plots

	std::shared_ptr<Gem::Common::GHistogram2D> adaptorPropertyHist2D_oa_;  ///< Holds the actual histogram
	std::shared_ptr<Gem::Common::GGraph2D>     fitnessGraph2D_oa_;    ///< Lets us monitor the current fitness of the population

	bool monitorBestOnly_ = false; ///< Indicates whether only the best individuals should be monitored
	bool addPrintCommand_ = false; ///< Asks the GPlotDesigner to add a print command to result files

	std::size_t maxIteration_ = 0; ///< Holds the largest iteration recorded for the algorithm
	std::size_t nIterationsRecorded_ = 0; ///< Holds the number of iterations that were recorded (not necessarily == maxIteration_

	std::vector<std::tuple<double, double>> adaptorPropertyStore_; ///< Holds all information about the number of adaptions

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		bool result = false;

		// Call the parent classes' functions
		if(GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorPropertyLoggerT<ind_type, num_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorPropertyLoggerT<ind_type, num_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		using boost::unit_test_framework::test_suite;
		using boost::unit_test_framework::test_case;

		// Call the parent classes' functions
		GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorPropertyLoggerT<ind_type, num_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GPLUGGABLEOPTIMIZATIONMONITORST_HPP_ */
