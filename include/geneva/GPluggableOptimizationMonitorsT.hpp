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
#include <chrono>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#ifndef GPLUGGABLEOPTIMIZATIONMONITORST_HPP_
#define GPLUGGABLEOPTIMIZATIONMONITORST_HPP_

// Geneva headers go here
#include "courtier/GExecutorT.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GLogger.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "geneva/GParameterPropertyParser.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/*
 * This is a collection of simple pluggable modules suitable for emitting certain specialized
 * information from within optimization algorithms. They can be plugged into G_OptimizationAlgorithm_Base<oa_type>
 * derivatives. A requirement is that they implement a function "informationFunction"
 * according to the API of G_OptimizationAlgorithm_Base<>::GBasePluggableOM .
 */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements the standard output common to all optimization algorithms.
 * It will usually already be registered as a pluggable optimization monitor, when
 * you instantiate a new optimization algorithm.
 */
template <typename oa_type>
class GStandardMonitorT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp(
			"GBasePluggableOM"
			, boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this)
		);
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
	GStandardMonitorT(const GStandardMonitorT<oa_type>& cp)
		: GBasePluggableOM<oa_type>(cp)
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
	const GStandardMonitorT<oa_type>& operator=(const GStandardMonitorT<oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GStandardMonitorT<oa_type> object
	 *
	 * @param  cp A constant reference to another GStandardMonitorT<oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GStandardMonitorT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GStandardMonitorT<oa_type> object
	 *
	 * @param  cp A constant reference to another GStandardMonitorT<oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GStandardMonitorT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		, oa_type * const goa
	) override {
		switch(im) {
			case Gem::Geneva::infoMode::INFOINIT: {
				glogger
				<< "Starting an optimization run with algorithm \"" << goa->getAlgorithmName() << "\"" << std::endl
				<< GLOGGING;
			}
				break;

			case Gem::Geneva::infoMode::INFOPROCESSING: {
				glogger
				<< std::setprecision(5)
				<< goa->getIteration() << ": "
			   << Gem::Common::g_to_string(goa->getBestCurrentPrimaryFitness())
				<< " // best past: " << Gem::Common::g_to_string(goa->getBestKnownPrimaryFitness())
				<< std::endl
				<< GLOGGING;
			}
				break;

			case Gem::Geneva::infoMode::INFOEND: {
				glogger
				<< "End of optimization reached in algorithm \"" << goa->getAlgorithmName()
				<< "\"" << std::endl
				<< GLOGGING;
			}
				break;
		}
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GStandardMonitorT<oa_type>");
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

		// Check that we are dealing with a GStandardMonitorT<oa_type> reference independent of this object and convert the pointer
		const GStandardMonitorT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GStandardMonitorT<oa_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... no local data

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GStandardMonitorT<oa_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GStandardMonitorT<oa_type> reference independent of this object and convert the pointer
		const GStandardMonitorT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... no local data
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GStandardMonitorT<oa_type>(*this);
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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GStandardMonitorT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GStandardMonitorT::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GStandardMonitorT::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};



/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to output fitness information for a given optimization run.
 * It takes care of successive runs and marks them in the output. Information
 * will be output in the same histogram both for the best individual(s) found so far
 * and for the best individual(s) of each iteration.
 */
template <typename oa_type>
class GFitnessMonitorT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOM", boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this))
		& BOOST_SERIALIZATION_NVP(m_xDim)
	  	& BOOST_SERIALIZATION_NVP(m_yDim)
		& BOOST_SERIALIZATION_NVP(m_nMonitorInds)
  		& BOOST_SERIALIZATION_NVP(m_resultFile)
	 	& BOOST_SERIALIZATION_NVP(m_infoInitRun)
		& BOOST_SERIALIZATION_NVP(m_globalFitnessGraphVec)
	   & BOOST_SERIALIZATION_NVP(m_iterationFitnessGraphVec);
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
	GFitnessMonitorT(const GFitnessMonitorT<oa_type>& cp)
		: GBasePluggableOM<oa_type>(cp)
	  	, m_xDim(cp.m_xDim)
	  	, m_yDim(cp.m_yDim)
		, m_nMonitorInds(cp.m_nMonitorInds)
		, m_resultFile(cp.m_resultFile)
		, m_infoInitRun(cp.m_infoInitRun)
	{
		Gem::Common::copyCloneableSmartPointerContainer(cp.m_globalFitnessGraphVec, m_globalFitnessGraphVec);
		Gem::Common::copyCloneableSmartPointerContainer(cp.m_iterationFitnessGraphVec, m_iterationFitnessGraphVec);
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
	const GFitnessMonitorT<oa_type>& operator=(const GFitnessMonitorT<oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GFitnessMonitorT<oa_type> object
	 *
	 * @param  cp A constant reference to another GFitnessMonitorT<oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GFitnessMonitorT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Checks for inequality with another GFitnessMonitorT<oa_type> object
	 *
	 * @param  cp A constant reference to another GFitnessMonitorT<oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GFitnessMonitorT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		m_resultFile = resultFile;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the current value of the result file name
	 *
	 * @return The current name of the result file
	 */
	std::string getResultFileName() const {
		return m_resultFile;
	}

	/***************************************************************************/
	/**
	 * Allows to set the dimensions of the canvas
	 *
	 * @param xDim The desired dimension of the canvas in x-direction
	 * @param yDim The desired dimension of the canvas in y-direction
	 */
	void setDims(const std::uint32_t &xDim, const std::uint32_t &yDim) {
		m_xDim = xDim;
		m_yDim = yDim;
	}

	/***************************************************************************/
	/**
	 * Retrieve the dimensions as a tuple
	 *
	 * @return The dimensions of the canvas as a tuple
	 */
	std::tuple<std::uint32_t, std::uint32_t> getDims() const {
		return std::tuple<std::uint32_t, std::uint32_t>(m_xDim, m_yDim);
	}

	/***************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in x-direction
	 *
	 * @return The dimension of the canvas in x-direction
	 */
	std::uint32_t getXDim() const {
		return m_xDim;
	}

	/***************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in y-direction
	 *
	 * @return The dimension of the canvas in y-direction
	 */
	std::uint32_t getYDim() const {
		return m_yDim;
	}

	/***************************************************************************/
	/**
	 * Sets the number of individuals in the population that should be monitored.
	 * If m_nMonitorInds == 0, the default will be set to 3, as fitness graphs are plotted in a row,
	 * and more than 3 will not give satisfactory graphical results. You may however
	 * request more monitored individuals, but will likely have to postprocess the ROOT script.
	 * If m_nMonitorInds is set to a larger number than there are individuals in the population,
	 * the value will be reset to that amount of individuals in informationFunction.
	 *
	 * @oaram nMonitorInds The number of individuals in the population that should be monitored
	 */
	void setNMonitorIndividuals(const std::size_t &nMonitorInds) {
		// Determine a suitable number of monitored individuals, if it hasn't already
		// been set externally.
		if(m_nMonitorInds == 0) {
			m_nMonitorInds = std::size_t(DEFNMONITORINDS);
		}

		m_nMonitorInds = nMonitorInds;
	}

	/***************************************************************************/
	/**
	 * Retrieves the number of individuals that are being monitored
	 *
	 * @return The number of individuals in the population being monitored
	 */
	std::size_t getNMonitorIndividuals() const {
		return m_nMonitorInds;
	}

	/***************************************************************************/
	/**
	 * Aggregates the work of all registered pluggable monitors
	 */
	virtual void informationFunction(
		const infoMode& im
		, oa_type * const goa
	) override {
		switch(im) {
			case Gem::Geneva::infoMode::INFOINIT: {
				// We set a marker whenever a new INFOINIT call happens. This way we
				// may "chain" algorithms and will get the entire progress information
				// for all algorithms
			} break;

			case Gem::Geneva::infoMode::INFOPROCESSING: {
				// Retrieve the list of globally- and iteration bests individuals
				auto global_bests = goa->G_Interface_Optimizer::template getBestGlobalIndividuals<GParameterSet>();
				auto iter_bests   = goa->G_Interface_Optimizer::template getBestIterationIndividuals<GParameterSet>();

				// Retrieve the current iteration in the population
				std::uint32_t iteration = goa->getIteration();

				// We expect both sizes to be identical
				if(global_bests.size() != iter_bests.size()) {
					glogger
					<< "In GFitnessMonitorT<oa_type>::informationFunction(): Error!" << std::endl
					<< "global_bests.size() = " << global_bests.size() << " != iter_bests.size() = " << iter_bests.size() << std::endl
					<< GEXCEPTION;
				}

				//------------------------------------------------------------------------------
				// Setup of local vectors

				if(!m_infoInitRun) {
					// Reset the number of monitored individuals to a suitable value, if necessary.
					if(m_nMonitorInds > global_bests.size()) {
						glogger
						<< "In GFitnessMonitorT<oa_type>::informationFunction(): Warning!" << std::endl
						<< "Requested number of individuals to be monitored in iteration " << iteration << " is larger" << std::endl
						<< "than the number of best individuals " << m_nMonitorInds << " / " << global_bests.size() << std::endl
						<< GWARNING;

						m_nMonitorInds = global_bests.size();
					}

					// Set up the plotters
					for(std::size_t ind = 0; ind<m_nMonitorInds; ind++) {
						std::shared_ptr <Gem::Common::GGraph2D> global_graph(new Gem::Common::GGraph2D());
						global_graph->setXAxisLabel("Iteration");
						global_graph->setYAxisLabel("Best Fitness");
						global_graph->setPlotLabel(std::string("Individual ") + boost::lexical_cast<std::string>(ind));
						global_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

						m_globalFitnessGraphVec.push_back(global_graph);

						std::shared_ptr <Gem::Common::GGraph2D> iteration_graph(new Gem::Common::GGraph2D());
						iteration_graph->setXAxisLabel("Iteration");
						iteration_graph->setYAxisLabel("Best Fitness");
						iteration_graph->setPlotLabel(std::string("Individual ") + boost::lexical_cast<std::string>(ind));
						iteration_graph->setPlotMode(Gem::Common::graphPlotMode::CURVE);

						m_iterationFitnessGraphVec.push_back(iteration_graph);

						// Add the iteration graph as secondary plotter
						global_graph->registerSecondaryPlotter(iteration_graph);
					}

					// Make sure m_globalFitnessGraphVec is only initialized once
					m_infoInitRun = true;
				} else {
					// We might have a situation where the number of best individuals changes in each
					// iteration, e.g. when dealing with pareto optimization in EA. In this case we reduce the
					// number of m_nMonitorInds to 1, which is the only safe option. Recorded data of other
					// individuals will then be lost -- the program will warn about this.
					if(m_nMonitorInds > global_bests.size()) {
						glogger
						<< "In GFitnessMonitorT<oa_type>::informationFunction(): Warning!" << std::endl
						<< "Requested number of individuals to be monitored in iteration " << iteration << " is larger" << std::endl
						<< "than the number of best individuals " << m_nMonitorInds << " / " << global_bests.size() << std::endl
						<< "This seems to be a result of a varying number of best individuals." << std::endl
						<< "We will now reduce the number of monitored individuals to 1 for the" << std::endl
						<< "rest of the optimization run. Recorded information for other individuals" << std::endl
						<< "will be deleted" << std::endl
						<< GWARNING;

						m_nMonitorInds = 1;
						m_globalFitnessGraphVec.resize(1);
						m_iterationFitnessGraphVec.resize(1);
					}
				}

				//------------------------------------------------------------------------------
				// Fill in the data for the best individuals

				std::vector<std::shared_ptr<Gem::Common::GGraph2D>>::iterator global_it;
				std::vector<std::shared_ptr<Gem::Common::GGraph2D>>::iterator iter_it;
				std::vector<std::shared_ptr<GParameterSet>>::iterator  global_ind_it;
				std::vector<std::shared_ptr<GParameterSet>>::iterator  iter_ind_it;

				std::size_t mInd = 0;
				for(
					global_it=m_globalFitnessGraphVec.begin(), iter_it=m_iterationFitnessGraphVec.begin(), global_ind_it = global_bests.begin(), iter_ind_it = iter_bests.begin()
					; global_it != m_globalFitnessGraphVec.end()
					; ++global_it, ++iter_it, ++global_ind_it, ++iter_ind_it
				) {
					(*global_it)->add(boost::numeric_cast<double>(iteration), (*global_ind_it)->fitness());
					(*iter_it  )->add(boost::numeric_cast<double>(iteration), (*iter_ind_it  )->fitness());
				}

				//------------------------------------------------------------------------------

			} break;

			case Gem::Geneva::infoMode::INFOEND: {
				/* nothing */
			} break;

			default: {
				glogger
				<< "In GFitnessMonitorT<oa_type>::informationFunction(): Received invalid infoMode " << im << std::endl
				<< GEXCEPTION;
			} break;
		}
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GFitnessMonitorT<oa_type>");
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

		// Check that we are dealing with a GFitnessMonitorT<oa_type> reference independent of this object and convert the pointer
		const GFitnessMonitorT *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GFitnessMonitorT<oa_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(m_xDim, p_load->m_xDim), token);
		compare_t(IDENTITY(m_yDim, p_load->m_yDim), token);
		compare_t(IDENTITY(m_nMonitorInds, p_load->m_nMonitorInds), token);
		compare_t(IDENTITY(m_resultFile, p_load->m_resultFile), token);
		compare_t(IDENTITY(m_infoInitRun, p_load->m_infoInitRun), token);
		compare_t(IDENTITY(m_globalFitnessGraphVec, p_load->m_globalFitnessGraphVec), token);
		compare_t(IDENTITY(m_iterationFitnessGraphVec, p_load->m_iterationFitnessGraphVec), token);

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GFitnessMonitorT<oa_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GFitnessMonitorT<oa_type> reference independent of this object and convert the pointer
		const GFitnessMonitorT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... and then our local data
		m_xDim = p_load->m_xDim;
		m_yDim = p_load->m_yDim;
		m_nMonitorInds = p_load->m_nMonitorInds;
		m_resultFile = p_load->m_resultFile;
		m_infoInitRun = p_load->m_infoInitRun;

		Gem::Common::copyCloneableSmartPointerContainer(p_load->m_globalFitnessGraphVec, m_globalFitnessGraphVec);
		Gem::Common::copyCloneableSmartPointerContainer(p_load->m_iterationFitnessGraphVec, m_iterationFitnessGraphVec);
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GFitnessMonitorT<oa_type>(*this);
	}

private:
	/************************************************************************/

	std::uint32_t m_xDim = DEFAULTXDIMOM; ///< The dimension of the canvas in x-direction
	std::uint32_t m_yDim = DEFAULTYDIMOM; ///< The dimension of the canvas in y-direction
	std::size_t m_nMonitorInds = DEFNMONITORINDS; ///< The number of individuals that should be monitored
	std::string m_resultFile = DEFAULTROOTRESULTFILEOM; ///< The name of the file to which data is emitted

	bool m_infoInitRun = false; ///< Allows to check whether the INFOINIT section of informationFunction has already been passed at least once
	std::vector<std::shared_ptr<Gem::Common::GGraph2D>> m_globalFitnessGraphVec; ///< Will hold progress information for the globally best individual
	std::vector<std::shared_ptr<Gem::Common::GGraph2D>> m_iterationFitnessGraphVec; ///< Will hold progress information for an iteration best's individual

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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFitnessMonitorT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFitnessMonitorT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GFitnessMonitorT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template <typename oa_type>
class GCollectiveMonitorT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this));

		// Some preparation needed if this is a load operation.
		// This is needed to work around a problem in Boost 1.58
		if (Archive::is_loading::value) {
			m_pluggable_monitors.clear();
		}

		ar
		& BOOST_SERIALIZATION_NVP(m_pluggable_monitors);
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
	GCollectiveMonitorT(const GCollectiveMonitorT<oa_type>& cp)
		: GBasePluggableOM<oa_type>(cp)
	{
		Gem::Common::copyCloneableSmartPointerContainer(cp.m_pluggable_monitors, m_pluggable_monitors);
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
	const GCollectiveMonitorT<oa_type>& operator=(const GCollectiveMonitorT<oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GCollectiveMonitorT<oa_type> object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT<oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GCollectiveMonitorT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GCollectiveMonitorT<oa_type> object
	 *
	 * @param  cp A constant reference to another GCollectiveMonitorT<oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GCollectiveMonitorT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		, oa_type * const goa
	) override {
		for(auto pm_ptr : m_pluggable_monitors) { // std::shared_ptr may be copied
			pm_ptr->informationFunction(im,goa);
		}
	}

	/***************************************************************************/
	/**
	 * Allows to register a new pluggable monitor
	 */
	void registerPluggableOM(
		std::shared_ptr<Gem::Geneva::GBasePluggableOM<oa_type>> om_ptr
	) {
		if(om_ptr) {
			m_pluggable_monitors.push_back(om_ptr);
		} else {
			glogger
			<< "In GCollectiveMonitorT<oa_type>::registerPluggableOM(): Error!" << std::endl
			<< "Got empty pointer to pluggable optimization monitor." << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * Checks if adaptors have been registered in the collective monitor
	 */
	bool hasOptimizationMonitors() const {
		return !m_pluggable_monitors.empty();
	}

	/***************************************************************************/
	/**
	 * Allows to clear all registered monitors
	 */
	void resetPluggbleOM() {
		m_pluggable_monitors.clear();
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GCollectiveMonitorT<oa_type>");
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

		// Check that we are dealing with a GCollectiveMonitorT<oa_type> reference independent of this object and convert the pointer
		const GCollectiveMonitorT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GCollectiveMonitorT<oa_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(m_pluggable_monitors, p_load->m_pluggable_monitors), token);

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GCollectiveMonitorT<oa_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GCollectiveMonitorT<oa_type> reference independent of this object and convert the pointer
		const GCollectiveMonitorT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... and then our local data
		Gem::Common::copyCloneableSmartPointerContainer(p_load->m_pluggable_monitors, m_pluggable_monitors);
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GCollectiveMonitorT<oa_type>(*this);
	}


private:
	std::vector<std::shared_ptr<Gem::Geneva::GBasePluggableOM<oa_type>>> m_pluggable_monitors; ///< The collection of monitors

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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GCollectiveMonitorT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GCollectiveMonitorT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GCollectiveMonitorT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template <typename fp_type, typename oa_type>
class GProgressPlotterTT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
			& make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this))
			& BOOST_SERIALIZATION_NVP(m_fp_profVarVec)
			& BOOST_SERIALIZATION_NVP(m_gpd)
			& BOOST_SERIALIZATION_NVP(m_progressPlotter2D_oa)
			& BOOST_SERIALIZATION_NVP(m_progressPlotter3D_oa)
			& BOOST_SERIALIZATION_NVP(m_progressPlotter4D_oa)
			& BOOST_SERIALIZATION_NVP(m_fileName)
			& BOOST_SERIALIZATION_NVP(m_canvasDimensions)
		   & BOOST_SERIALIZATION_NVP(m_monitorBestOnly)
			& BOOST_SERIALIZATION_NVP(m_monitorValidOnly)
		   & BOOST_SERIALIZATION_NVP(m_observeBoundaries)
			& BOOST_SERIALIZATION_NVP(m_addPrintCommand);
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
	GProgressPlotterTT()
		: m_gpd("Progress information", 1, 1)
		, m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1024,768))
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Construction with the information whether only the best individuals
	 * should be monitored and whether only valid items should be recorded.
	 * Some member variables may be initialized in the class body.
	 */
	GProgressPlotterTT(
		bool monitorBestOnly
		, bool monitorValidOnly
	)
		: m_gpd("Progress information", 1, 1)
		, m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1024,768))
		, m_monitorBestOnly(monitorBestOnly)
		, m_monitorValidOnly(monitorValidOnly)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GProgressPlotterTT(const GProgressPlotterTT<fp_type, oa_type>& cp)
		: GBasePluggableOM<oa_type>(cp)
		, m_gpd(cp.m_gpd)
		, m_fileName(cp.m_fileName)
		, m_canvasDimensions(cp.m_canvasDimensions)
		, m_monitorBestOnly(cp.m_monitorBestOnly)
		, m_monitorValidOnly(cp.m_monitorValidOnly)
		, m_observeBoundaries(cp.m_observeBoundaries)
		, m_addPrintCommand(cp.m_addPrintCommand)
	{
		Gem::Common::copyCloneableSmartPointer(cp.m_progressPlotter2D_oa, m_progressPlotter2D_oa);
		Gem::Common::copyCloneableSmartPointer(cp.m_progressPlotter3D_oa, m_progressPlotter3D_oa);
		Gem::Common::copyCloneableSmartPointer(cp.m_progressPlotter4D_oa, m_progressPlotter4D_oa);
		Gem::Common::copyCloneableObjectsContainer(cp.m_fp_profVarVec, m_fp_profVarVec);
	}

	/***************************************************************************/
	/**
	 * The destuctor
	 */
	virtual ~GProgressPlotterTT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GProgressPlotterTT<fp_type, oa_type>& operator=(const GProgressPlotterTT<fp_type, oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GProgressPlotterTT<fp_type, oa_type> object
	 *
	 * @param  cp A constant reference to another GProgressPlotterTT<fp_type, oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GProgressPlotterTT<fp_type, oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GProgressPlotterTT<fp_type, oa_type> object
	 *
	 * @param  cp A constant reference to another GProgressPlotterTT<fp_type, oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GProgressPlotterTT<fp_type, oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		m_fp_profVarVec.clear();

		// Parse the parameter string
		GParameterPropertyParser ppp(parStr);

		//---------------------------------------------------------------------------
		// Retrieve the parameters

		std::tuple<
			typename std::vector<parPropSpec<fp_type>>::const_iterator
			, typename std::vector<parPropSpec<fp_type>>::const_iterator
		> t_d = ppp.getIterators<fp_type>();

		typename std::vector<parPropSpec<fp_type>>::const_iterator fp_cit = std::get<0>(t_d);
		typename std::vector<parPropSpec<fp_type>>::const_iterator d_end  = std::get<1>(t_d);
		for(; fp_cit!=d_end; ++fp_cit) { // Note: fp_cit is already set to the begin of the double parameter arrays
			m_fp_profVarVec.push_back(*fp_cit);
		}

		//---------------------------------------------------------------------------
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only the best individuals should be monitored.
	 */
	void setMonitorBestOnly(bool monitorBestOnly = true) {
		m_monitorBestOnly = monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only the best individuals should be monitored.
	 */
	bool getMonitorBestOnly() const {
		return m_monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only valid individuals should be monitored.
	 */
	void setMonitorValidOnly(bool monitorValidOnly = true) {
		m_monitorValidOnly = monitorValidOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only valid individuals should be monitored.
	 */
	bool getMonitorValidOnly() const {
		return m_monitorValidOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to spefify whether scan boundaries should be observed
	 */
	void setObserveBoundaries(bool observeBoundaries) {
		m_observeBoundaries = observeBoundaries;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether boundaries should be observed
	 */
	bool getObserveBoundaries() const {
		return m_observeBoundaries;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether parameters should be profiled
	 */
	bool parameterProfileCreationRequested() const {
		return !m_fp_profVarVec.empty();
	}

	/***************************************************************************/
	/**
	 * Retrieves the number of variables that will be profiled
	 */
	std::size_t nProfileVars() const {
		return m_fp_profVarVec.size();
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions
	 */
	void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		m_canvasDimensions = canvasDimensions;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions using separate x and y values
	 */
	void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		m_canvasDimensions = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	}

	/***************************************************************************/
	/**
	 * Gives access to the canvas dimensions
	 */
	std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		return m_canvasDimensions;
	}

	/******************************************************************************/
	/**
	 * Allows to add a "Print" command to the end of the script so that picture files are created
	 */
	void setAddPrintCommand(bool addPrintCommand) {
		m_addPrintCommand = addPrintCommand;
	}

	/******************************************************************************/
	/**
	 * Allows to retrieve the current value of the m_addPrintCommand variable
	 */
	bool getAddPrintCommand() const {
		return m_addPrintCommand;
	}

	/***************************************************************************/
	/**
	 * Allows to set the filename
	 */
	void setFileName(std::string fileName) {
		m_fileName = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current filename to which information will be emitted
	 */
	std::string getFileName() const {
		return m_fileName;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas label
	 */
	void setCanvasLabel(const std::string& canvasLabel) {
		m_gpd.setCanvasLabel(canvasLabel);
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the canvas label
	 */
	std::string getCanvasLabel() const {
		return m_gpd.getCanvasLabel();
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
				<< "In GProgressPlotterTT<fp_type, oa_type>::getLabel(): Error" << std::endl
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
		, oa_type * const goa
	) override {
		switch(im) {
			case Gem::Geneva::infoMode::INFOINIT:
			{
				switch(this->nProfileVars()) {
					case 1:
					{
						m_progressPlotter2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());

						m_progressPlotter2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);
						m_progressPlotter2D_oa->setPlotLabel("Fitness as a function of a parameter value");
						m_progressPlotter2D_oa->setXAxisLabel(this->getLabel(m_fp_profVarVec[0]));
						m_progressPlotter2D_oa->setYAxisLabel("Fitness");

						m_gpd.registerPlotter(m_progressPlotter2D_oa);
					}
						break;
					case 2:
					{
						m_progressPlotter3D_oa = std::shared_ptr<Gem::Common::GGraph3D>(new Gem::Common::GGraph3D());

						m_progressPlotter3D_oa->setPlotLabel("Fitness as a function of parameter values");
						m_progressPlotter3D_oa->setXAxisLabel(this->getLabel(m_fp_profVarVec[0]));
						m_progressPlotter3D_oa->setYAxisLabel(this->getLabel(m_fp_profVarVec[1]));
						m_progressPlotter3D_oa->setZAxisLabel("Fitness");

						m_gpd.registerPlotter(m_progressPlotter3D_oa);
					}
						break;

					case 3:
					{
						m_progressPlotter4D_oa = std::shared_ptr<Gem::Common::GGraph4D>(new Gem::Common::GGraph4D());

						m_progressPlotter4D_oa->setPlotLabel("Fitness (color-coded) as a function of parameter values");
						m_progressPlotter4D_oa->setXAxisLabel(this->getLabel(m_fp_profVarVec[0]));
						m_progressPlotter4D_oa->setYAxisLabel(this->getLabel(m_fp_profVarVec[1]));
						m_progressPlotter4D_oa->setZAxisLabel(this->getLabel(m_fp_profVarVec[2]));

						m_gpd.registerPlotter(m_progressPlotter4D_oa);
					}
						break;

					default:
					{
						glogger
						<< "NOTE: In GProgressPlotterTT<fp_type, oa_type>::informationFunction(infoMode::INFOINIT):" << std::endl
						<< "Number of profiling dimensions " << this->nProfileVars() << " can not be displayed." << std::endl
						<< "No graphical output will be created." << std::endl
						<< GLOGGING;
					}
						break;
				}

				m_gpd.setCanvasDimensions(m_canvasDimensions);
			}
				break;

			case Gem::Geneva::infoMode::INFOPROCESSING:
			{
				bool isDirty = true;
				double primaryFitness;

				if(m_monitorBestOnly) { // Monitor the best individuals only
					std::shared_ptr<GParameterSet> p = goa->G_Interface_Optimizer::template getBestGlobalIndividual<GParameterSet>();
					if(GBasePluggableOM<oa_type>::m_useRawEvaluation) {
						primaryFitness = p->fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
					} else {
						primaryFitness = p->transformedFitness();
					}

					if(!m_monitorValidOnly || p->isValid()) {
						switch(this->nProfileVars()) {
							case 1:
							{
								fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(m_fp_profVarVec[0].var);

								if(m_observeBoundaries) {
									if(
										val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
										) {
										m_progressPlotter2D_oa->add(double(val0), primaryFitness);
									}
								} else {
									m_progressPlotter2D_oa->add(double(val0), primaryFitness);
								}
							}
								break;

							case 2:
							{
								fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(m_fp_profVarVec[0].var);
								fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(m_fp_profVarVec[1].var);

								if(m_observeBoundaries) {
									if(
										val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
										&& val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
										) {
										m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
									}
								} else {
									m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
								}
							}
								break;

							case 3:
							{
								fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(m_fp_profVarVec[0].var);
								fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(m_fp_profVarVec[1].var);
								fp_type val2 = p->GOptimizableEntity::getVarVal<fp_type>(m_fp_profVarVec[2].var);

								if(m_observeBoundaries) {
									if(
										val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
										&& val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
										&& val2 >= m_fp_profVarVec[2].lowerBoundary && val2 <= m_fp_profVarVec[2].upperBoundary
										) {
										m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
									}
								} else {
									m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
								}
							}
								break;

							default: // Do nothing by default. The number of profiling dimensions is too large
								break;
						}
					}
				} else { // Monitor all individuals
					typename oa_type::iterator it;
					for(it=goa->begin(); it!=goa->end(); ++it) {

						if(GBasePluggableOM<oa_type>::m_useRawEvaluation) {
							primaryFitness = (*it)->fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
						} else {
							primaryFitness = (*it)->fitness(0, PREVENTREEVALUATION, USETRANSFORMEDFITNESS);
						}

						if(!m_monitorValidOnly || (*it)->isValid()) {
							switch(this->nProfileVars()) {
								case 1:
								{
									fp_type val0    = (*it)->GOptimizableEntity::template getVarVal<fp_type>(m_fp_profVarVec[0].var);

									if(m_observeBoundaries) {
										if(
											val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
											) {
											m_progressPlotter2D_oa->add(double(val0), primaryFitness);
										}
									} else {
										m_progressPlotter2D_oa->add(double(val0), primaryFitness);
									}
								}
									break;

								case 2:
								{
									fp_type val0 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(m_fp_profVarVec[0].var);
									fp_type val1 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(m_fp_profVarVec[1].var);

									if(m_observeBoundaries) {
										if(
											val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
											&& val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
											) {
											m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
										}
									} else {
										m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
									}
								}
									break;

								case 3:
								{
									fp_type val0 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(m_fp_profVarVec[0].var);
									fp_type val1 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(m_fp_profVarVec[1].var);
									fp_type val2 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(m_fp_profVarVec[2].var);

									if(m_observeBoundaries) {
										if(
											val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
											&& val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
											&& val2 >= m_fp_profVarVec[2].lowerBoundary && val2 <= m_fp_profVarVec[2].upperBoundary
											) {
											m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
										}
									} else {
										m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
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

			case Gem::Geneva::infoMode::INFOEND:
			{
				// Make sure 1-D data is sorted
				if(1 == this->nProfileVars()) {
					m_progressPlotter2D_oa->sortX();
				}

				// Inform the plot designer whether it should print png files
				m_gpd.setAddPrintCommand(m_addPrintCommand);

				// Write out the result.
				m_gpd.writeToFile(m_fileName);

				// Remove all plotters
				m_gpd.resetPlotters();
				m_progressPlotter2D_oa.reset();
				m_progressPlotter3D_oa.reset();
				m_progressPlotter4D_oa.reset();
			}
				break;

			default:
			{
				glogger
				<< "In GProgressPlotterTT<fp_type, oa_type>::informationFunction(): Received invalid infoMode " << im << std::endl
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
		return std::string("GProgressPlotterTT<fp_type, oa_type>");
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

		// Check that we are dealing with a GProgressPlotterTT<fp_type reference independent of this object and convert the pointer
		const GProgressPlotterTT<fp_type, oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GProgressPlotterTT<fp_type, oa_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(m_fp_profVarVec, p_load->m_fp_profVarVec), token);
		compare_t(IDENTITY(m_gpd, p_load->m_gpd), token);
		compare_t(IDENTITY(m_progressPlotter2D_oa, p_load->m_progressPlotter2D_oa), token);
		compare_t(IDENTITY(m_progressPlotter3D_oa, p_load->m_progressPlotter3D_oa), token);
		compare_t(IDENTITY(m_progressPlotter4D_oa, p_load->m_progressPlotter4D_oa), token);
		compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
		compare_t(IDENTITY(m_canvasDimensions, p_load->m_canvasDimensions), token);
		compare_t(IDENTITY(m_monitorBestOnly, p_load->m_monitorBestOnly), token);
		compare_t(IDENTITY(m_monitorValidOnly, p_load->m_monitorValidOnly), token);
		compare_t(IDENTITY(m_observeBoundaries, p_load->m_observeBoundaries), token);
		compare_t(IDENTITY(m_addPrintCommand, p_load->m_addPrintCommand), token);

		// React on deviations from the expectation
		token.evaluate();
	}

protected:
	/************************************************************************/
	/**
	 * Loads the data of another object
	 *
	 * cp A pointer to another GProgressPlotterTT<fp_type, oa_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GProgressPlotterTT<fp_type, oa_type> reference independent of this object and convert the pointer
		const GProgressPlotterTT<fp_type, oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... and then our local data
		Gem::Common::copyCloneableObjectsContainer(p_load->m_fp_profVarVec, m_fp_profVarVec);
		m_gpd.load(p_load->m_gpd);
		copyCloneableSmartPointer(p_load->m_progressPlotter2D_oa, m_progressPlotter2D_oa);
		copyCloneableSmartPointer(p_load->m_progressPlotter3D_oa, m_progressPlotter3D_oa);
		copyCloneableSmartPointer(p_load->m_progressPlotter4D_oa, m_progressPlotter4D_oa);
		m_fileName = p_load->m_fileName;
		m_canvasDimensions = p_load->m_canvasDimensions;
		m_monitorBestOnly = p_load->m_monitorBestOnly;
		m_monitorValidOnly = p_load->m_monitorValidOnly;
		m_observeBoundaries = p_load->m_observeBoundaries;
		m_addPrintCommand = p_load->m_addPrintCommand;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GProgressPlotterTT<fp_type, oa_type>(*this);
	}

private:
	std::vector<parPropSpec<fp_type>> m_fp_profVarVec; ///< Holds information about variables to be profiled

	Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots

	// These are temporaries
	std::shared_ptr<Gem::Common::GGraph2D> m_progressPlotter2D_oa;
	std::shared_ptr<Gem::Common::GGraph3D> m_progressPlotter3D_oa;
	std::shared_ptr<Gem::Common::GGraph4D> m_progressPlotter4D_oa;

	std::string m_fileName = std::string("progressScan.C"); ///< The name of the file the output should be written to. Note that the class will add the name of the algorithm it acts on
	std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions; ///< The dimensions of the canvas

	bool m_monitorBestOnly = false;  ///< Indicates whether only the best individuals should be monitored
	bool m_monitorValidOnly = false; ///< Indicates whether only valid individuals should be plotted
	bool m_observeBoundaries = false; ///< When set to true, the plotter will ignore values outside of a scan boundary

	bool m_addPrintCommand = false; ///< Asks the GPlotDesigner to add a print command to result files

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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GProgressPlotterTT<fp_type, oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GProgressPlotterTT<fp_type, oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GProgressPlotterTT<fp_type, oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A partial specialization of GProgressPlotterTT for the broker executor.
 * We simply use the parent constructors.
 *
 * TODO: This can now be unified with GProgressPlotterT
 */
template <typename fp_type>
class GProgressPlotterT
	: public GProgressPlotterTT<fp_type, G_OptimizationAlgorithm_Base>
{
public:
	 // Simple inherit all constructors
	 using GProgressPlotterTT<fp_type, G_OptimizationAlgorithm_Base>::GProgressPlotterTT;
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
template <typename oa_type>
class GAllSolutionFileLoggerT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this))
		& BOOST_SERIALIZATION_NVP(m_fileName)
		& BOOST_SERIALIZATION_NVP(m_boundaries)
		& BOOST_SERIALIZATION_NVP(m_boundariesActive)
		& BOOST_SERIALIZATION_NVP(m_withNameAndType)
		& BOOST_SERIALIZATION_NVP(m_withCommas)
		& BOOST_SERIALIZATION_NVP(m_useRawFitness)
		& BOOST_SERIALIZATION_NVP(m_showValidity)
	   & BOOST_SERIALIZATION_NVP(m_printInitial)
	 	& BOOST_SERIALIZATION_NVP(m_showIterationBoundaries);
	}
	///////////////////////////////////////////////////////////////////////

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
		: m_fileName(fileName)
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
		: m_fileName(fileName)
		, m_boundaries(boundaries)
		, m_boundariesActive(true)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GAllSolutionFileLoggerT(const GAllSolutionFileLoggerT<oa_type>& cp)
		: m_fileName(cp.m_fileName)
		, m_boundaries(cp.m_boundaries)
		, m_boundariesActive(cp.m_boundariesActive)
		, m_withNameAndType(cp.m_withNameAndType)
		, m_withCommas(cp.m_withCommas)
		, m_useRawFitness(cp.m_useRawFitness)
		, m_showValidity(cp.m_showValidity)
		, m_printInitial(cp.m_printInitial)
		, m_showIterationBoundaries(cp.m_showIterationBoundaries)
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
	const GAllSolutionFileLoggerT<oa_type>& operator=(const GAllSolutionFileLoggerT<oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GAllSolutionFileLoggerT<oa_type> object
	 *
	 * @param  cp A constant reference to another GAllSolutionFileLoggerT<oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GAllSolutionFileLoggerT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GAllSolutionFileLoggerT<oa_type> object
	 *
	 * @param  cp A constant reference to another GAllSolutionFileLoggerT<oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GAllSolutionFileLoggerT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		return std::string("GAllSolutionFileLoggerT<oa_type>");
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

		// Check that we are dealing with a GAllSolutionFileLoggerT<oa_type> reference independent of this object and convert the pointer
		const GAllSolutionFileLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAllSolutionFileLoggerT<oa_type>>(cp, this);

		GToken token("GAllSolutionFileLoggerT<oa_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
		compare_t(IDENTITY(m_boundaries, p_load->m_boundaries), token);
		compare_t(IDENTITY(m_boundariesActive, p_load->m_boundariesActive), token);
		compare_t(IDENTITY(m_withNameAndType, p_load->m_withNameAndType), token);
		compare_t(IDENTITY(m_withCommas, p_load->m_withCommas), token);
		compare_t(IDENTITY(m_useRawFitness, p_load->m_useRawFitness), token);
		compare_t(IDENTITY(m_showValidity, p_load->m_showValidity), token);
	   compare_t(IDENTITY(m_printInitial, p_load->m_printInitial), token);
		compare_t(IDENTITY(m_showIterationBoundaries, p_load->m_showIterationBoundaries), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		m_fileName = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return m_fileName;
	}

	/***************************************************************************/
	/**
	 * Sets the boundaries
	 */
	void setBoundaries(std::vector<double> boundaries) {
		m_boundaries = boundaries;
		m_boundariesActive = true;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the boundaries
	 */
	std::vector<double> getBoundaries() const {
		return m_boundaries;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether boundaries are active
	 */
	bool boundariesActive() const {
		return m_boundariesActive;
	}

	/***************************************************************************/
	/**
	 * Allows to inactivate boundaries
	 */
	void setBoundariesInactive() {
		m_boundariesActive = false;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether explanations should be printed for parameter-
	 * and fitness values.
	 */
	void setPrintWithNameAndType(bool withNameAndType = true) {
		m_withNameAndType = withNameAndType;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether explanations should be printed for parameter-
	 * and fitness values
	 */
	bool getPrintWithNameAndType() const {
		return m_withNameAndType;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether commas should be printed in-between values
	 */
	void setPrintWithCommas(bool withCommas = true) {
		m_withCommas = withCommas;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether commas should be printed in-between values
	 */
	bool getPrintWithCommas() const {
		return m_withCommas;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether the true (instead of the transformed) fitness should be shown
	 */
	void setUseTrueFitness(bool useRawFitness = true) {
		m_useRawFitness = useRawFitness;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
	 */
	bool getUseTrueFitness() const {
		return m_useRawFitness;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether the validity of a solution should be shown
	 */
	void setShowValidity(bool showValidity = true) {
		m_showValidity = showValidity;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether the validity of a solution will be shown
	 */
	bool getShowValidity() const {
		return m_showValidity;
	}

 	/***************************************************************************/
	/**
	 * Allows to specifiy whether the initial population (prior to any
	 * optimization work) should be printed.
	 */
	void setPrintInitial(bool printInitial = true) {
		m_printInitial = printInitial;
	}

 	/***************************************************************************/
	/**
	 * Allows to check whether the initial population (prior to any
	 * optimization work) should be printed.
	 */
 	bool getPrintInitial() const {
		return m_printInitial;
	}

	/***************************************************************************/
 	/**
    * Allows to specifiy whether a comment line should be inserted
    * between iterations
    */
   void setShowIterationBoundaries(bool showIterationBoundaries = true) {
		m_showIterationBoundaries = showIterationBoundaries;
 	}

	/***************************************************************************/
   /**
    * Allows to check whether a comment line should be inserted
    * between iterations
    */
   bool getShowIterationBoundaries() const {
	   return m_showIterationBoundaries;
 	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, oa_type * const goa
	) override {
		switch(im) {
			case Gem::Geneva::infoMode::INFOINIT:
			{
				// If the file pointed to by m_fileName already exists, make a back-up
				if(bf::exists(m_fileName)) {
					std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

					glogger
					<< "In GAllSolutionFileLoggerT<oa_type>::informationFunction(): Warning!" << std::endl
					<< "Attempt to output information to file " << m_fileName << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(m_fileName, newFileName);
				}

				if(m_printInitial) {
					this->printPopulation("Initial population", goa);
				}
			}
			break;

			case Gem::Geneva::infoMode::INFOPROCESSING:
			{
				this->printPopulation("At end of iteration " + boost::lexical_cast<std::string>(goa->getIteration()), goa);
			}
				break;

			case Gem::Geneva::infoMode::INFOEND:
				// nothing
			break;

			default:
			{
				glogger
				<< "In GAllSolutionFileLoggerT<oa_type>: Received invalid infoMode " << im << std::endl
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
	 * cp A pointer to another GAllSolutionFileLoggerT<oa_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GAllSolutionFileLoggerT<oa_type> reference independent of this object and convert the pointer
		const GAllSolutionFileLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... and then our local data
		m_fileName = p_load->m_fileName;
		m_boundaries = p_load->m_boundaries;
		m_boundariesActive = p_load->m_boundariesActive;
		m_withNameAndType = p_load->m_withNameAndType;
		m_withCommas = p_load->m_withCommas;
		m_useRawFitness = p_load->m_useRawFitness;
		m_showValidity = p_load->m_showValidity;
		m_printInitial = p_load->m_printInitial;
		m_showIterationBoundaries = p_load->m_showIterationBoundaries;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GAllSolutionFileLoggerT<oa_type>(*this);
	}

private:
 	/***************************************************************************/
	/**
	 * Does the actual printing
	 */
 	void printPopulation(
		const std::string& iterationDescription
		, oa_type * const goa
	) {
		// Open the external file
		boost::filesystem::ofstream data(m_fileName, std::ofstream::app);

		if(m_showIterationBoundaries) {
			data
				<< "#" << std::endl
				<< "# -----------------------------------------------------------------------------" << std::endl
				<< "# " << iterationDescription << ":" << std::endl
			   << "#" << std::endl;
		}

		// Loop over all individuals of the algorithm.
		for(std::size_t pos=0; pos<goa->size(); pos++) {
			std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

			// Note that isGoodEnough may throw if loop acts on a "dirty" individual
			if(!m_boundariesActive || ind->isGoodEnough(m_boundaries)) {
				// Append the data to the external file
				if(0 == pos && goa->inFirstIteration()) { // Only output name and type in the very first line (if at all)
					data << ind->toCSV(m_withNameAndType, m_withCommas, m_useRawFitness, m_showValidity);
				} else {
					data << ind->toCSV(false, m_withCommas, m_useRawFitness, m_showValidity);
				}
			}
		}

		// Close the external file
		data.close();
	}

	/***************************************************************************/

	std::string m_fileName = "CompleteSolutionLog.txt"; ///< The name of the file to which solutions should be stored
	std::vector<double> m_boundaries; ///< Value boundaries used to filter logged solutions
	bool m_boundariesActive = false; ///< Set to true if boundaries have been set
	bool m_withNameAndType = false; ///< When set to true, explanations for values are printed
	bool m_withCommas = false; ///< When set to true, commas will be printed in-between values
	bool m_useRawFitness = true; ///< Indicates whether true- or transformed fitness should be output
	bool m_showValidity = true; ///< Indicates whether the validity of a solution should be shown
 	bool m_printInitial = false; ///< Indicates whether the initial population should be printed
 	bool m_showIterationBoundaries = false; ///< Indicates whether a comment indicating the end of an iteration should be printed

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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAllSolutionFileLoggerT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAllSolutionFileLoggerT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAllSolutionFileLoggerT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template <typename oa_type>
class GIterationResultsFileLoggerT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this))
		& BOOST_SERIALIZATION_NVP(m_fileName)
		& BOOST_SERIALIZATION_NVP(m_withCommas)
		& BOOST_SERIALIZATION_NVP(m_useRawFitness);
	}
	///////////////////////////////////////////////////////////////////////

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
		: m_fileName(fileName)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GIterationResultsFileLoggerT(const GIterationResultsFileLoggerT<oa_type>& cp)
		: m_fileName(cp.m_fileName)
		, m_withCommas(cp.m_withCommas)
		, m_useRawFitness(cp.m_useRawFitness)
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
	const GIterationResultsFileLoggerT<oa_type>& operator=(const GIterationResultsFileLoggerT<oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GIterationResultsFileLoggerT<oa_type> object
	 *
	 * @param  cp A constant reference to another GIterationResultsFileLoggerT<oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GIterationResultsFileLoggerT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GIterationResultsFileLoggerT<oa_type> object
	 *
	 * @param  cp A constant reference to another GIterationResultsFileLoggerT<oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GIterationResultsFileLoggerT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		return std::string("GIterationResultsFileLoggerT<oa_type>");
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

		// Check that we are dealing with a GIterationResultsFileLoggerT<oa_type>
		// reference independent of this object and convert the pointer
		const GIterationResultsFileLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		GToken token("GIterationResultsFileLoggerT<oa_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
		compare_t(IDENTITY(m_withCommas, p_load->m_withCommas), token);
		compare_t(IDENTITY(m_useRawFitness, p_load->m_useRawFitness), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		m_fileName = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return m_fileName;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether commas should be printed in-between values
	 */
	void setPrintWithCommas(bool withCommas) {
		m_withCommas = withCommas;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether commas should be printed in-between values
	 */
	bool getPrintWithCommas() const {
		return m_withCommas;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether the true (instead of the transformed) fitness should be shown
	 */
	void setUseTrueFitness(bool useRawFitness) {
		m_useRawFitness = useRawFitness;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
	 */
	bool getUseTrueFitness() const {
		return m_useRawFitness;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, oa_type * const goa
	) override {
		switch(im) {
			case Gem::Geneva::infoMode::INFOINIT:
			{
				// If the file pointed to by m_fileName already exists, make a back-up
				if(bf::exists(m_fileName)) {
					std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

					glogger
					<< "In GIterationResultsFileLoggerT<oa_type>::informationFunction(): Warning!" << std::endl
					<< "Attempt to output information to file " << m_fileName << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(m_fileName, newFileName);
				}
			}
				break;

			case Gem::Geneva::infoMode::INFOPROCESSING:
			{
				// Open the external file
				boost::filesystem::ofstream data(m_fileName.c_str(), std::ofstream::app);
				std::vector<double> fitnessVec;

				// Loop over all individuals of the algorithm.
				std::size_t nIndividuals = goa->size();
				for(std::size_t pos=0; pos<nIndividuals; pos++) {
					std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
					fitnessVec = goa->at(pos)->fitnessVec(m_useRawFitness);

					std::size_t nFitnessCriteria = goa->at(0)->getNumberOfFitnessCriteria();
					for(std::size_t i=0; i<nFitnessCriteria; i++) {
						data << fitnessVec.at(i) << ((m_withCommas && (nFitnessCriteria*nIndividuals > (i+1)*(pos+1)))?", ":" ");
					}
				}
				data << std::endl;

				// Close the external file
				data.close();
			}
				break;

			case Gem::Geneva::infoMode::INFOEND:
				// nothing
				break;

			default:
			{
				glogger
				<< "In GIterationResultsFileLoggerT<oa_type>: Received invalid infoMode " << im << std::endl
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
	 * cp A pointer to another GIterationResultsFileLoggerT<oa_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GIterationResultsFileLoggerT<oa_type>
		// reference independent of this object and convert the pointer
		const GIterationResultsFileLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... and then our local data
		m_fileName = p_load->m_fileName;
		m_withCommas = p_load->m_withCommas;
		m_useRawFitness = p_load->m_useRawFitness;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GIterationResultsFileLoggerT<oa_type>(*this);
	}

private:
	/***************************************************************************/

	std::string m_fileName = "IterationResultsLog.txt"; ///< The name of the file to which solutions should be stored
	bool m_withCommas = true; ///< When set to true, commas will be printed in-between values
	bool m_useRawFitness = false; ///< Indicates whether true- or transformed fitness should be output

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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIterationResultsFileLoggerT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIterationResultsFileLoggerT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GIterationResultsFileLoggerT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template <typename oa_type>
class GNAdpationsLoggerT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this))
		& BOOST_SERIALIZATION_NVP(m_fileName)
		& BOOST_SERIALIZATION_NVP(m_canvasDimensions)
		& BOOST_SERIALIZATION_NVP(m_gpd)
	  	& BOOST_SERIALIZATION_NVP(m_nAdaptionsHist2D_oa)
	 	& BOOST_SERIALIZATION_NVP(m_nAdaptionsGraph2D_oa)
		& BOOST_SERIALIZATION_NVP(m_fitnessGraph2D_oa)
		& BOOST_SERIALIZATION_NVP(m_monitorBestOnly)
		& BOOST_SERIALIZATION_NVP(m_addPrintCommand)
		& BOOST_SERIALIZATION_NVP(m_maxIteration)
		& BOOST_SERIALIZATION_NVP(m_nIterationsRecorded)
		& BOOST_SERIALIZATION_NVP(m_nAdaptionsStore);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor. Note that some variables may be
	 * initialized in the class body.
	 */
	GNAdpationsLoggerT()
		: m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, m_gpd("Number of adaptions per iteration", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a file name. Note that some variables may be
	 * initialized in the class body.
	 */
	explicit GNAdpationsLoggerT(const std::string& fileName)
		: m_fileName(fileName)
		, m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, m_gpd("Number of adaptions per iteration", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GNAdpationsLoggerT(const GNAdpationsLoggerT<oa_type>& cp)
		: m_fileName(cp.m_fileName)
		, m_canvasDimensions(cp.m_canvasDimensions)
		, m_gpd(cp.m_gpd)
		, m_monitorBestOnly(cp.m_monitorBestOnly)
		, m_addPrintCommand(cp.m_addPrintCommand)
		, m_maxIteration(cp.m_maxIteration)
		, m_nIterationsRecorded(cp.m_nIterationsRecorded)
		, m_nAdaptionsStore(cp.m_nAdaptionsStore)
	{
		Gem::Common::copyCloneableSmartPointer(cp.m_nAdaptionsHist2D_oa, m_nAdaptionsHist2D_oa);
		Gem::Common::copyCloneableSmartPointer(cp.m_nAdaptionsGraph2D_oa, m_nAdaptionsGraph2D_oa);
		Gem::Common::copyCloneableSmartPointer(cp.m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
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
	const GNAdpationsLoggerT<oa_type>& operator=(const GNAdpationsLoggerT<oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GNAdpationsLoggerT<oa_type> object
	 *
	 * @param  cp A constant reference to another GNAdpationsLoggerT<oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GNAdpationsLoggerT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GNAdpationsLoggerT<oa_type> object
	 *
	 * @param  cp A constant reference to another GNAdpationsLoggerT<oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GNAdpationsLoggerT<oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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

		// Check that we are dealing with a GNAdpationsLoggerT<oa_type> reference independent of this object and convert the pointer
		const GNAdpationsLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNAdpationsLoggerT<oa_type>>(cp, this);

		GToken token("GNAdpationsLoggerT<oa_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
		compare_t(IDENTITY(m_canvasDimensions, p_load->m_canvasDimensions), token);
		compare_t(IDENTITY(m_gpd, p_load->m_gpd), token);
		compare_t(IDENTITY(m_nAdaptionsHist2D_oa, p_load->m_nAdaptionsHist2D_oa), token);
		compare_t(IDENTITY(m_nAdaptionsGraph2D_oa, p_load->m_nAdaptionsGraph2D_oa), token);
		compare_t(IDENTITY(m_fitnessGraph2D_oa, p_load->m_fitnessGraph2D_oa), token);
		compare_t(IDENTITY(m_monitorBestOnly, p_load->m_monitorBestOnly), token);
		compare_t(IDENTITY(m_addPrintCommand, p_load->m_addPrintCommand), token);
		compare_t(IDENTITY(m_maxIteration, p_load->m_maxIteration), token);
		compare_t(IDENTITY(m_nIterationsRecorded, p_load->m_nIterationsRecorded), token);
		compare_t(IDENTITY(m_nAdaptionsStore, p_load->m_nAdaptionsStore), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		m_fileName = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return m_fileName;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only the best individuals should be monitored.
	 */
	void setMonitorBestOnly(bool monitorBestOnly = true) {
		m_monitorBestOnly = monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only the best individuals should be monitored.
	 */
	bool getMonitorBestOnly() const {
		return m_monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions
	 */
	void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		m_canvasDimensions = canvasDimensions;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions using separate x and y values
	 */
	void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		m_canvasDimensions = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	}

	/***************************************************************************/
	/**
	 * Gives access to the canvas dimensions
	 */
	std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		return m_canvasDimensions;
	}

	/******************************************************************************/
	/**
	 * Allows to add a "Print" command to the end of the script so that picture files are created
	 */
	void setAddPrintCommand(bool addPrintCommand) {
		m_addPrintCommand = addPrintCommand;
	}

	/******************************************************************************/
	/**
	 * Allows to retrieve the current value of the m_addPrintCommand variable
	 */
	bool getAddPrintCommand() const {
		return m_addPrintCommand;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, oa_type * const goa
	) override {
		using namespace Gem::Common;

		switch(im) {
			case Gem::Geneva::infoMode::INFOINIT:
			{
				// If the file pointed to by m_fileName already exists, make a back-up
				if(bf::exists(m_fileName)) {
					std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

					glogger
					<< "In GNAdpationsLoggerT<oa_type>::informationFunction(): Error!" << std::endl
					<< "Attempt to output information to file " << m_fileName << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(m_fileName, newFileName);
				}

				// Make sure the progress plotter has the desired size
				m_gpd.setCanvasDimensions(m_canvasDimensions);

				// Set up a graph to monitor the best fitness found
				m_fitnessGraph2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
				m_fitnessGraph2D_oa->setXAxisLabel("Iteration");
				m_fitnessGraph2D_oa->setYAxisLabel("Fitness");
				m_fitnessGraph2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);
			}
				break;

			case Gem::Geneva::infoMode::INFOPROCESSING:
			{
				std::uint32_t iteration = goa->getIteration();

				// Record the current fitness
				std::shared_ptr<GParameterSet> p = goa->G_Interface_Optimizer::template getBestGlobalIndividual<GParameterSet>();
				(*m_fitnessGraph2D_oa) & std::tuple<double,double>(double(iteration), double(p->fitness()));

				// Update the largest known iteration and the number of recorded iterations
				m_maxIteration = iteration;
				m_nIterationsRecorded++;

				// Do the actual logging
				if(m_monitorBestOnly) {
					std::shared_ptr<GParameterSet> best = goa->G_Interface_Optimizer::template getBestGlobalIndividual<GParameterSet>();
					m_nAdaptionsStore.push_back(std::tuple<double,double>(double(iteration), double(best->getNAdaptions())));
				} else { // Monitor all individuals
					// Loop over all individuals of the algorithm.
					for(std::size_t pos=0; pos<goa->size(); pos++) {
						std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
						m_nAdaptionsStore.push_back(std::tuple<double,double>(double(iteration), double(ind->getNAdaptions())));
					}
				}
			}
				break;

			case Gem::Geneva::infoMode::INFOEND:
			{
				std::vector<std::tuple<double, double>>::iterator it;

				if(m_monitorBestOnly) {
					// Create the graph object
					m_nAdaptionsGraph2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
					m_nAdaptionsGraph2D_oa->setXAxisLabel("Iteration");
					m_nAdaptionsGraph2D_oa->setYAxisLabel("Number of parameter adaptions");
					m_nAdaptionsGraph2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);

					// Fill the object with data
					for(it=m_nAdaptionsStore.begin(); it!=m_nAdaptionsStore.end(); ++it) {
						(*m_nAdaptionsGraph2D_oa) & *it;
					}

					// Add the histogram to the plot designer
					m_gpd.registerPlotter(m_nAdaptionsGraph2D_oa);

				} else { // All individuals are monitored
					// Within m_nAdaptionsStore, find the largest number of adaptions performed
					std::size_t maxNAdaptions = 0;
					for(it=m_nAdaptionsStore.begin(); it!=m_nAdaptionsStore.end(); ++it) {
						if(std::get<1>(*it) > maxNAdaptions) {
							maxNAdaptions = boost::numeric_cast<std::size_t>(std::get<1>(*it));
						}
					}

					// Create the histogram object
					m_nAdaptionsHist2D_oa = std::shared_ptr<GHistogram2D>(
						new GHistogram2D(
							m_nIterationsRecorded
							, maxNAdaptions+1
							, 0., double(m_maxIteration)
							, 0., double(maxNAdaptions)
						)
					);

					m_nAdaptionsHist2D_oa->setXAxisLabel("Iteration");
					m_nAdaptionsHist2D_oa->setYAxisLabel("Number of parameter adaptions");
					m_nAdaptionsHist2D_oa->setDrawingArguments("BOX");

					// Fill the object with data
					for(it=m_nAdaptionsStore.begin(); it!=m_nAdaptionsStore.end(); ++it) {
						(*m_nAdaptionsHist2D_oa) & *it;
					}

					// Add the histogram to the plot designer
					m_gpd.registerPlotter(m_nAdaptionsHist2D_oa);
				}

				// Add the fitness monitor
				m_gpd.registerPlotter(m_fitnessGraph2D_oa);

				// Inform the plot designer whether it should print png files
				m_gpd.setAddPrintCommand(m_addPrintCommand);

				// Write out the result. Note that we add
				m_gpd.writeToFile(m_fileName);

				// Remove all plotters
				m_gpd.resetPlotters();
				m_nAdaptionsHist2D_oa.reset();
				m_nAdaptionsGraph2D_oa.reset();
			}
				break;

			default:
			{
				glogger
				<< "In GNAdpationsLoggerT<oa_type>: Received invalid infoMode " << im << std::endl
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
	 * cp A pointer to another GNAdpationsLoggerT<oa_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GNAdpationsLoggerT<oa_type> reference independent of this object and convert the pointer
		const GNAdpationsLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNAdpationsLoggerT<oa_type>>(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... and then our local data
		m_fileName = p_load->m_fileName;
		m_canvasDimensions = p_load->m_canvasDimensions;
		m_gpd = p_load->m_gpd;
		Gem::Common::copyCloneableSmartPointer(p_load->m_nAdaptionsHist2D_oa, m_nAdaptionsHist2D_oa);
		Gem::Common::copyCloneableSmartPointer(p_load->m_nAdaptionsGraph2D_oa, m_nAdaptionsGraph2D_oa);
		Gem::Common::copyCloneableSmartPointer(p_load->m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
		m_monitorBestOnly = p_load->m_monitorBestOnly;
		m_addPrintCommand = p_load->m_addPrintCommand;
		m_maxIteration = p_load->m_maxIteration;
		m_nIterationsRecorded = p_load->m_nIterationsRecorded;
		m_nAdaptionsStore = p_load->m_nAdaptionsStore;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GNAdpationsLoggerT<oa_type>(*this);
	}

private:
	/***************************************************************************/

	std::string m_fileName = "NAdaptions.C"; ///< The name of the file to which solutions should be stored

	std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions; ///< The dimensions of the canvas

	Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots

	std::shared_ptr<Gem::Common::GHistogram2D> m_nAdaptionsHist2D_oa;  ///< Holds the actual histogram
	std::shared_ptr<Gem::Common::GGraph2D>     m_nAdaptionsGraph2D_oa; ///< Used if we only monitor the best solution in each iteration
	std::shared_ptr<Gem::Common::GGraph2D>     m_fitnessGraph2D_oa;    ///< Lets us monitor the current fitness of the population

	bool m_monitorBestOnly = false; ///< Indicates whether only the best individuals should be monitored
	bool m_addPrintCommand = false; ///< Asks the GPlotDesigner to add a print command to result files

	std::size_t m_maxIteration = 0; ///< Holds the largest iteration recorded for the algorithm
	std::size_t m_nIterationsRecorded = 0; ///< Holds the number of iterations that were recorded (not necessarily == m_maxIteration

	std::vector<std::tuple<double, double>> m_nAdaptionsStore; ///< Holds all information about the number of adaptions

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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNAdpationsLoggerT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNAdpationsLoggerT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GNAdpationsLoggerT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
template <typename num_type, typename oa_type>
class GAdaptorPropertyLoggerTT
	: public GBasePluggableOM<oa_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this))
		& BOOST_SERIALIZATION_NVP(m_fileName)
		& BOOST_SERIALIZATION_NVP(m_adaptorName)
		& BOOST_SERIALIZATION_NVP(m_property)
		& BOOST_SERIALIZATION_NVP(m_canvasDimensions)
		& BOOST_SERIALIZATION_NVP(m_gpd)
		& BOOST_SERIALIZATION_NVP(m_adaptorPropertyHist2D_oa)
		& BOOST_SERIALIZATION_NVP(m_fitnessGraph2D_oa)
		& BOOST_SERIALIZATION_NVP(m_monitorBestOnly)
		& BOOST_SERIALIZATION_NVP(m_addPrintCommand)
		& BOOST_SERIALIZATION_NVP(m_maxIteration)
		& BOOST_SERIALIZATION_NVP(m_nIterationsRecorded)
		& BOOST_SERIALIZATION_NVP(m_adaptorPropertyStore);
	}
	///////////////////////////////////////////////////////////////////////

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
	GAdaptorPropertyLoggerTT()
		: m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, m_gpd("Adaptor properties", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Initialization with a file name
	 */
	GAdaptorPropertyLoggerTT(
		const std::string& fileName
		, const std::string& adaptorName
		, const std::string& property
	)
		: m_fileName(fileName)
		, m_adaptorName(adaptorName)
		, m_property(property)
		, m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
		, m_gpd("Adaptor properties", 1, 2)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GAdaptorPropertyLoggerTT(const GAdaptorPropertyLoggerTT<num_type, oa_type>& cp)
		: m_fileName(cp.m_fileName)
		, m_adaptorName(cp.m_adaptorName)
		, m_property(cp.m_property)
		, m_canvasDimensions(cp.m_canvasDimensions)
		, m_gpd(cp.m_gpd)
		, m_monitorBestOnly(cp.m_monitorBestOnly)
		, m_addPrintCommand(cp.m_addPrintCommand)
		, m_maxIteration(cp.m_maxIteration)
		, m_nIterationsRecorded(cp.m_nIterationsRecorded)
		, m_adaptorPropertyStore(cp.m_adaptorPropertyStore)
	{
		// Copy the smart pointers over
		Gem::Common::copyCloneableSmartPointer(cp.m_adaptorPropertyHist2D_oa, m_adaptorPropertyHist2D_oa);
		Gem::Common::copyCloneableSmartPointer(cp.m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GAdaptorPropertyLoggerTT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GAdaptorPropertyLoggerTT<num_type, oa_type>& operator=(const GAdaptorPropertyLoggerTT<num_type, oa_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/************************************************************************/
	/**
	 * Checks for equality with another GAdaptorPropertyLoggerTT<num_type, oa_type> object
	 *
	 * @param  cp A constant reference to another GAdaptorPropertyLoggerTT<num_type, oa_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool operator==(const GAdaptorPropertyLoggerTT<num_type, oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/************************************************************************/
	/**
	 * Checks for inequality with another GAdaptorPropertyLoggerTT<num_type, oa_type> object
	 *
	 * @param  cp A constant reference to another GAdaptorPropertyLoggerTT<num_type, oa_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	virtual bool operator!=(const GAdaptorPropertyLoggerTT<num_type, oa_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		return std::string("GAdaptorPropertyLoggerTT");
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

		// Check that we are dealing with a GAdaptorPropertyLoggerTT<num_type, oa_type> reference independent of this object and convert the pointer
		const GAdaptorPropertyLoggerTT<num_type, oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerTT<num_type, oa_type>>(cp, this);

		GToken token("GAdaptorPropertyLoggerTT", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		// ... and then our local data
		compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
		compare_t(IDENTITY(m_adaptorName, p_load->m_adaptorName), token);
		compare_t(IDENTITY(m_property, p_load->m_property), token);
		compare_t(IDENTITY(m_canvasDimensions, p_load->m_canvasDimensions), token);
		compare_t(IDENTITY(m_gpd, p_load->m_gpd), token);
		compare_t(IDENTITY(m_adaptorPropertyHist2D_oa, p_load->m_adaptorPropertyHist2D_oa), token);
		compare_t(IDENTITY(m_fitnessGraph2D_oa, p_load->m_fitnessGraph2D_oa), token);
		compare_t(IDENTITY(m_monitorBestOnly, p_load->m_monitorBestOnly), token);
		compare_t(IDENTITY(m_addPrintCommand, p_load->m_addPrintCommand), token);
		compare_t(IDENTITY(m_maxIteration, p_load->m_maxIteration), token);
		compare_t(IDENTITY(m_nIterationsRecorded, p_load->m_nIterationsRecorded), token);
		compare_t(IDENTITY(m_adaptorPropertyStore, p_load->m_adaptorPropertyStore), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	 * Sets the file name
	 */
	void setFileName(std::string fileName) {
		m_fileName = fileName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current file name
	 */
	std::string getFileName() const {
		return m_fileName;
	}

	/***************************************************************************/
	/**
	 * Sets the name of the adaptor
	 */
	void setAdaptorName(std::string adaptorName) {
		m_adaptorName = adaptorName;
	}

	/***************************************************************************/
	/**
	 * Retrieves the name of the adaptor
	 */
	std::string getAdaptorName() const {
		return m_adaptorName;
	}

	/***************************************************************************/
	/**
	 * Sets the name of the property
	 */
	void setPropertyName(std::string property) {
		m_property = property;
	}

	/***************************************************************************/
	/**
	 * Retrieves the name of the property
	 */
	std::string getPropertyName() const {
		return m_property;
	}

	/***************************************************************************/
	/**
	 * Allows to specify whether only the best individuals should be monitored.
	 */
	void setMonitorBestOnly(bool monitorBestOnly = true) {
		m_monitorBestOnly = monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to check whether only the best individuals should be monitored.
	 */
	bool getMonitorBestOnly() const {
		return m_monitorBestOnly;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions
	 */
	void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		m_canvasDimensions = canvasDimensions;
	}

	/***************************************************************************/
	/**
	 * Allows to set the canvas dimensions using separate x and y values
	 */
	void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		m_canvasDimensions = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	}

	/***************************************************************************/
	/**
	 * Gives access to the canvas dimensions
	 */
	std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		return m_canvasDimensions;
	}

	/******************************************************************************/
	/**
	 * Allows to add a "Print" command to the end of the script so that picture files are created
	 */
	void setAddPrintCommand(bool addPrintCommand) {
		m_addPrintCommand = addPrintCommand;
	}

	/******************************************************************************/
	/**
	 * Allows to retrieve the current value of the m_addPrintCommand variable
	 */
	bool getAddPrintCommand() const {
		return m_addPrintCommand;
	}

	/***************************************************************************/
	/**
	 * Allows to emit information in different stages of the information cycle
	 * (initialization, during each cycle and during finalization)
	 */
	virtual void informationFunction(
		const infoMode& im
		, oa_type * const goa
	) override {
		using namespace Gem::Common;

		switch(im) {
			case Gem::Geneva::infoMode::INFOINIT:
			{
				// If the file pointed to by m_fileName already exists, make a back-up
				if(bf::exists(m_fileName)) {
					std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

					glogger
					<< "In GAdaptorPropertyLoggerTT::informationFunction(): Error!" << std::endl
					<< "Attempt to output information to file " << m_fileName << std::endl
					<< "which already exists. We will rename the old file to" << std::endl
					<< newFileName << std::endl
					<< GWARNING;

					bf::rename(m_fileName, newFileName);
				}

				// Make sure the progress plotter has the desired size
				m_gpd.setCanvasDimensions(m_canvasDimensions);

				// Set up a graph to monitor the best fitness found
				m_fitnessGraph2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
				m_fitnessGraph2D_oa->setXAxisLabel("Iteration");
				m_fitnessGraph2D_oa->setYAxisLabel("Fitness");
				m_fitnessGraph2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);
			}
				break;

			case Gem::Geneva::infoMode::INFOPROCESSING:
			{
				std::uint32_t iteration = goa->getIteration();

				// Record the current fitness
				std::shared_ptr<GParameterSet> p = goa->G_Interface_Optimizer::template getBestGlobalIndividual<GParameterSet>();
				(*m_fitnessGraph2D_oa) & std::tuple<double,double>(double(iteration), double(p->fitness()));

				// Update the largest known iteration and the number of recorded iterations
				m_maxIteration = iteration;
				m_nIterationsRecorded++;

				// Will hold the adaptor properties
				std::vector<boost::any> data;

				// Do the actual logging
				if(m_monitorBestOnly) {
					std::shared_ptr<GParameterSet> best = goa->G_Interface_Optimizer::template getBestGlobalIndividual<GParameterSet>();

					// Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
					best->queryAdaptor(m_adaptorName, m_property, data);

					// Attach the data to m_adaptorPropertyStore
					std::vector<boost::any>::iterator prop_it;
					for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
						m_adaptorPropertyStore.push_back(std::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
					}
				} else { // Monitor all individuals
					// Loop over all individuals of the algorithm.
					for(std::size_t pos=0; pos<goa->size(); pos++) {
						std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

						// Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
						ind->queryAdaptor(m_adaptorName, m_property, data);

						// Attach the data to m_adaptorPropertyStore
						std::vector<boost::any>::iterator prop_it;
						for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
							m_adaptorPropertyStore.push_back(std::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
						}
					}
				}
			}
				break;

			case Gem::Geneva::infoMode::INFOEND:
			{
				std::vector<std::tuple<double, double>>::iterator it;

				// Within m_adaptorPropertyStore, find the largest number of adaptions performed
				double maxProperty = 0.;
				for(it=m_adaptorPropertyStore.begin(); it!=m_adaptorPropertyStore.end(); ++it) {
					if(std::get<1>(*it) > maxProperty) {
						maxProperty = std::get<1>(*it);
					}
				}

				// Create the histogram object
				m_adaptorPropertyHist2D_oa = std::shared_ptr<GHistogram2D>(
					new GHistogram2D(
						m_nIterationsRecorded
						, 100
						, 0., double(m_maxIteration)
						, 0., maxProperty
					)
				);

				m_adaptorPropertyHist2D_oa->setXAxisLabel("Iteration");
				m_adaptorPropertyHist2D_oa->setYAxisLabel(std::string("Adaptor-Name: ") + m_adaptorName + std::string(", Property: ") + m_property);
				m_adaptorPropertyHist2D_oa->setDrawingArguments("BOX");

				// Fill the object with data
				for(it=m_adaptorPropertyStore.begin(); it!=m_adaptorPropertyStore.end(); ++it) {
					(*m_adaptorPropertyHist2D_oa) & *it;
				}

				// Add the histogram to the plot designer
				m_gpd.registerPlotter(m_adaptorPropertyHist2D_oa);

				// Add the fitness monitor
				m_gpd.registerPlotter(m_fitnessGraph2D_oa);

				// Inform the plot designer whether it should print png files
				m_gpd.setAddPrintCommand(m_addPrintCommand);

				// Write out the result. Note that we add
				m_gpd.writeToFile(m_fileName);

				// Remove all plotters (they will survive inside of gpd)
				m_gpd.resetPlotters();
				m_adaptorPropertyHist2D_oa.reset();
			}
				break;

			default:
			{
				glogger
				<< "In GAdaptorPropertyLoggerTT: Received invalid infoMode " << im << std::endl
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
	 * cp A pointer to another GAdaptorPropertyLoggerTT<num_type object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GAdaptorPropertyLoggerTT<num_type, oa_type> reference independent of this object and convert the pointer
		const GAdaptorPropertyLoggerTT<num_type, oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerTT<num_type, oa_type>>(cp, this);

		// Load the parent classes' data ...
		GBasePluggableOM<oa_type>::load_(cp);

		// ... and then our local data
		m_fileName = p_load->m_fileName;
		m_adaptorName = p_load->m_adaptorName;
		m_property = p_load->m_property;
		m_canvasDimensions = p_load->m_canvasDimensions;
		m_gpd = p_load->m_gpd;
		Gem::Common::copyCloneableSmartPointer(p_load->m_adaptorPropertyHist2D_oa, m_adaptorPropertyHist2D_oa);
		Gem::Common::copyCloneableSmartPointer(p_load->m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
		m_monitorBestOnly = p_load->m_monitorBestOnly;
		m_addPrintCommand = p_load->m_addPrintCommand;
		m_maxIteration = p_load->m_maxIteration;
		m_nIterationsRecorded = p_load->m_nIterationsRecorded;
		m_adaptorPropertyStore = p_load->m_adaptorPropertyStore;
	}

	/************************************************************************/
	/**
	 * Creates a deep clone of this object
	 */
	virtual GObject* clone_() const override {
		return new GAdaptorPropertyLoggerTT<num_type, oa_type>(*this);
	}

private:
	/***************************************************************************/

	std::string m_fileName = "NAdaptions.C"; ///< The name of the file to which solutions should be stored

	std::string m_adaptorName = "GDoubleGaussAdaptor"; ///< The  name of the adaptor for which properties should be logged
	std::string m_property = "sigma"; ///< The name of the property to be logged

	std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions; ///< The dimensions of the canvas

	Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots

	std::shared_ptr<Gem::Common::GHistogram2D> m_adaptorPropertyHist2D_oa;  ///< Holds the actual histogram
	std::shared_ptr<Gem::Common::GGraph2D>     m_fitnessGraph2D_oa;    ///< Lets us monitor the current fitness of the population

	bool m_monitorBestOnly = false; ///< Indicates whether only the best individuals should be monitored
	bool m_addPrintCommand = false; ///< Asks the GPlotDesigner to add a print command to result files

	std::size_t m_maxIteration = 0; ///< Holds the largest iteration recorded for the algorithm
	std::size_t m_nIterationsRecorded = 0; ///< Holds the number of iterations that were recorded (not necessarily == m_maxIteration

	std::vector<std::tuple<double, double>> m_adaptorPropertyStore; ///< Holds all information about the number of adaptions

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
		if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			result = true;
		}

		// no local data -- nothing to change

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorPropertyLoggerTT<num_type, oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorPropertyLoggerTT<num_type, oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GAdaptorPropertyLoggerTT<num_type, oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}
	/***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log the time needed for the processing step of each
 * individual. The output happens in the form of two root files, one holding histograms
 * for the processing times, the other showing the distribution of processing times for
 * each iteration in a 2D histogram
 */
template <typename oa_type>
class GProcessingTimesLoggerT
	: public GBasePluggableOM<oa_type>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM<oa_type>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_fileName_pth)
		 & BOOST_SERIALIZATION_NVP(m_canvasDimensions_pth)
		 & BOOST_SERIALIZATION_NVP(m_gpd_pth)
		 & BOOST_SERIALIZATION_NVP(m_fileName_pth2)
		 & BOOST_SERIALIZATION_NVP(m_canvasDimensions_pth2)
		 & BOOST_SERIALIZATION_NVP(m_gpd_pth2)
		 & BOOST_SERIALIZATION_NVP(m_fileName_txt)
		 & BOOST_SERIALIZATION_NVP(m_pre_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_post_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_all_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_pre_processing_times_hist2D)
		 & BOOST_SERIALIZATION_NVP(m_processing_times_hist2D)
	 	 & BOOST_SERIALIZATION_NVP(m_post_processing_times_hist2D)
		 & BOOST_SERIALIZATION_NVP(m_all_processing_times_hist2D)
		 & BOOST_SERIALIZATION_NVP(m_nBinsX)
		 & BOOST_SERIALIZATION_NVP(m_nBinsY);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor. Note that some variables may be initialized in the class body.
	  */
	 GProcessingTimesLoggerT()
		 : m_canvasDimensions_pth(std::tuple<std::uint32_t,std::uint32_t>(1600,1200))
		 , m_gpd_pth("Timings for the processing steps of individuals", 2, 2)
		 , m_canvasDimensions_pth2(std::tuple<std::uint32_t,std::uint32_t>(1600,1200))
		 , m_gpd_pth2("Timings for the processing steps of individuals vs. iteration", 2, 2)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization with a file name. Note that some variables may be initialized in the class body.
	  */
	 GProcessingTimesLoggerT(
		 const std::string& fileName_pth
		 , const std::string& fileName_pth2
		 , const std::string& fileName_txt
	 	 , std::size_t nBinsX
		 , std::size_t nBinsY
	 )
		 : m_fileName_pth(fileName_pth)
		 , m_canvasDimensions_pth(std::tuple<std::uint32_t,std::uint32_t>(1600,1200))
		 , m_gpd_pth("Timings for the processing steps of individuals", 2, 2)
		 , m_fileName_pth2(fileName_pth2)
		 , m_canvasDimensions_pth2(std::tuple<std::uint32_t,std::uint32_t>(1600,1200))
		 , m_gpd_pth2("Timings for the processing steps of individuals vs. iteration", 2, 2)
		 , m_fileName_txt(fileName_txt)
		 , m_nBinsX(nBinsX)
	    , m_nBinsY(nBinsY)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GProcessingTimesLoggerT(const GProcessingTimesLoggerT<oa_type>& cp)
		 : m_fileName_pth(cp.m_fileName_pth)
		 , m_canvasDimensions_pth(cp.m_canvasDimensions_pth)
		 , m_gpd_pth(cp.m_gpd_pth)
		 , m_fileName_pth2(cp.m_fileName_pth2)
		 , m_canvasDimensions_pth2(cp.m_canvasDimensions_pth2)
		 , m_gpd_pth2(cp.m_gpd_pth2)
		 , m_fileName_txt(cp.m_fileName_txt)
	 	 , m_nBinsX(cp.m_nBinsX)
		 , m_nBinsY(cp.m_nBinsY)
	 {
		 // No need to copy the histograms over, as they will be instantiated
		 // in the INFOINIT section.
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GProcessingTimesLoggerT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A standard assignment operator
	  */
	 const GProcessingTimesLoggerT<oa_type>& operator=(const GProcessingTimesLoggerT<oa_type>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /************************************************************************/
	 /**
	  * Checks for equality with another GProcessingTimesLoggerT<oa_type> object
	  *
	  * @param  cp A constant reference to another GProcessingTimesLoggerT<oa_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 virtual bool operator==(const GProcessingTimesLoggerT<oa_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /************************************************************************/
	 /**
	  * Checks for inequality with another GProcessingTimesLoggerT<oa_type> object
	  *
	  * @param  cp A constant reference to another GProcessingTimesLoggerT<oa_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 virtual bool operator!=(const GProcessingTimesLoggerT<oa_type>& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		 return std::string("GProcessingTimesLoggerT<oa_type>");
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

		 // Check that we are dealing with a GProcessingTimesLoggerT<oa_type> reference independent of this object and convert the pointer
		 const GProcessingTimesLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GProcessingTimesLoggerT<oa_type>>(cp, this);

		 GToken token("GProcessingTimesLoggerT<oa_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBasePluggableOM<oa_type>>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_fileName_pth, p_load->m_fileName_pth), token);
		 compare_t(IDENTITY(m_canvasDimensions_pth, p_load->m_canvasDimensions_pth), token);
		 compare_t(IDENTITY(m_gpd_pth, p_load->m_gpd_pth), token);
		 compare_t(IDENTITY(m_fileName_pth2, p_load->m_fileName_pth2), token);
		 compare_t(IDENTITY(m_canvasDimensions_pth2, p_load->m_canvasDimensions_pth2), token);
		 compare_t(IDENTITY(m_gpd_pth2, p_load->m_gpd_pth2), token);
		 compare_t(IDENTITY(m_fileName_txt, p_load->m_fileName_txt), token);
		 compare_t(IDENTITY(m_pre_processing_times_hist, p_load->m_pre_processing_times_hist), token);
		 compare_t(IDENTITY(m_processing_times_hist, p_load->m_processing_times_hist), token);
		 compare_t(IDENTITY(m_post_processing_times_hist, p_load->m_post_processing_times_hist), token);
		 compare_t(IDENTITY(m_all_processing_times_hist, p_load->m_all_processing_times_hist), token);
		 compare_t(IDENTITY(m_pre_processing_times_hist2D, p_load->m_pre_processing_times_hist2D), token);
		 compare_t(IDENTITY(m_processing_times_hist2D, p_load->m_processing_times_hist2D), token);
		 compare_t(IDENTITY(m_post_processing_times_hist2D, p_load->m_post_processing_times_hist2D), token);
		 compare_t(IDENTITY(m_all_processing_times_hist2D, p_load->m_all_processing_times_hist2D), token);
		 compare_t(IDENTITY(m_nBinsX, p_load->m_nBinsX), token);
		 compare_t(IDENTITY(m_nBinsY, p_load->m_nBinsY), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Sets the file name for the processing times histogram
	  */
	 void setFileName_pth(std::string fileName) {
		 m_fileName_pth = fileName;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current file name for the processing times histogram
	  */
	 std::string getFileName_pth() const {
		 return m_fileName_pth;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the file name for the processing times histograms (2D)
	  */
	 void setFileName_pth2(std::string fileName) {
		 m_fileName_pth2 = fileName;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current file name for the processing times histograms (2D)
	  */
	 std::string getFileName_pth2() const {
		 return m_fileName_pth2;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the file name for the text output
	  */
	 void setFileName_txt(std::string fileName) {
		 m_fileName_txt = fileName;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current file name for the text output
	  */
	 std::string getFileName_txt() const {
		 return m_fileName_txt;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions for the processing times histograms
	  */
	 void setCanvasDimensions_pth(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		 m_canvasDimensions_pth = canvasDimensions;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions using separate x and y values for the
	  * processing times histograms
	  */
	 void setCanvasDimensions_pth(std::uint32_t x, std::uint32_t y) {
		 m_canvasDimensions_pth = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	 }

	 /***************************************************************************/
	 /**
	  * Gives access to the canvas dimensions of the processing times histograms
	  */
	 std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions_pth() const {
		 return m_canvasDimensions_pth;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions for the processing times histograms (2D)
	  */
	 void setCanvasDimensions_pth2(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		 m_canvasDimensions_pth2 = canvasDimensions;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions using separate x and y values for the
	  * processing times histograms (2D)
	  */
	 void setCanvasDimensions_pth2(std::uint32_t x, std::uint32_t y) {
		 m_canvasDimensions_pth2 = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	 }

	 /***************************************************************************/
	 /**
	  * Gives access to the canvas dimensions of the processing times histograms (2D)
	  */
	 std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions_pth2() const {
		 return m_canvasDimensions_pth2;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the number of bins for the processing times histograms in y-direction
	  */
	 void setNBinsX(std::size_t nBinsX) {
		 if(nBinsX > 0) {
			 m_nBinsX = nBinsX;
		 } else {
			 glogger
				 << "In GProcessingTimesLoggerT<oa_type>::setNBinsX(): Error!" << std::endl
				 << "nBinsX is set to 0" << std::endl
				 << GEXCEPTION;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current number of bins for the processing times
	  * histograms in x-direction
	  */
	 std::size_t getNBinsX() const {
		 return m_nBinsX;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the number of bins for the processing times histograms in y-direction
	  */
	 void setNBinsY(std::size_t nBinsY) {
		 if(nBinsY > 0) {
			 m_nBinsY = nBinsY;
		 } else {
			 glogger
				 << "In GProcessingTimesLoggerT<oa_type>::setNBinsY(): Error!" << std::endl
				 << "nBinsY is set to 0" << std::endl
				 << GEXCEPTION;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current number of bins for the processing times
	  * histograms in y-direction
	  */
	 std::size_t getNBinsY() const {
		 return m_nBinsY;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to emit information in different stages of the information cycle
	  * (initialization, during each cycle and during finalization)
	  */
	 virtual void informationFunction(
		 const infoMode& im
		 , oa_type * const goa
	 ) override {
		 switch(im) {
			 case Gem::Geneva::infoMode::INFOINIT: {
				 //---------------------------------------------------------------
				 // Histograms

				 // If the file pointed to by m_fileName_pth already exists, make a back-up
				 if(bf::exists(m_fileName_pth)) {
					 std::string newFileName = m_fileName_pth + ".bak_" + Gem::Common::getMSSince1970();

					 glogger
						 << "In GProcessingTimesLoggerT<oa_type>::informationFunction(): Warning!" << std::endl
						 << "Attempt to output information to file " << m_fileName_pth << std::endl
						 << "which already exists. We will rename the old file to" << std::endl
						 << newFileName << std::endl
						 << GWARNING;

					 bf::rename(m_fileName_pth, newFileName);
				 }

				 // Make sure the processing times plotter has the desired size
				 m_gpd_pth.setCanvasDimensions(m_canvasDimensions_pth);

				 m_pre_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
				 m_pre_processing_times_hist->setXAxisLabel("Pre-processing time [s]");
				 m_pre_processing_times_hist->setYAxisLabel("Number of Entries");
				 m_pre_processing_times_hist->setDrawingArguments("hist");

				 m_gpd_pth.registerPlotter(m_pre_processing_times_hist);

				 m_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
				 m_processing_times_hist->setXAxisLabel("Main processing time [s]");
				 m_processing_times_hist->setYAxisLabel("Number of Entries");
				 m_processing_times_hist->setDrawingArguments("hist");

				 m_gpd_pth.registerPlotter(m_processing_times_hist);

				 m_post_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
				 m_post_processing_times_hist->setXAxisLabel("Post-processing time [s]");
				 m_post_processing_times_hist->setYAxisLabel("Number of Entries");
				 m_post_processing_times_hist->setDrawingArguments("hist");

				 m_gpd_pth.registerPlotter(m_post_processing_times_hist);

				 m_all_processing_times_hist = std::make_shared<Gem::Common::GHistogram1D>(m_nBinsX);
				 m_all_processing_times_hist->setXAxisLabel("Overall processing time for all steps [s]");
				 m_all_processing_times_hist->setYAxisLabel("Number of Entries");
				 m_all_processing_times_hist->setDrawingArguments("hist");

				 m_gpd_pth.registerPlotter(m_all_processing_times_hist);

				 //---------------------------------------------------------------
				 // 2D Histograms

				 // If the file pointed to by m_fileName_pth2 already exists, make a back-up
				 if(bf::exists(m_fileName_pth2)) {
					 std::string newFileName = m_fileName_pth2 + ".bak_" + Gem::Common::getMSSince1970();

					 glogger
						 << "In GProcessingTimesLoggerT<oa_type>::informationFunction(): Warning!" << std::endl
						 << "Attempt to output information to file " << m_fileName_pth2 << std::endl
						 << "which already exists. We will rename the old file to" << std::endl
						 << newFileName << std::endl
						 << GWARNING;

					 bf::rename(m_fileName_pth2, newFileName);
				 }

				 // Make sure the processing times has the desired size
				 m_gpd_pth2.setCanvasDimensions(m_canvasDimensions_pth2);

				 m_pre_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
				 m_pre_processing_times_hist2D->setXAxisLabel("Iteration");
				 m_pre_processing_times_hist2D->setYAxisLabel("Pre-processing time [s]");
				 m_pre_processing_times_hist2D->setZAxisLabel("Number of Entries");
				 m_pre_processing_times_hist2D->setDrawingArguments("box");

				 m_gpd_pth2.registerPlotter(m_pre_processing_times_hist2D);


				 m_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
				 m_processing_times_hist2D->setXAxisLabel("Iteration");
				 m_processing_times_hist2D->setYAxisLabel("Main processing time [s]");
				 m_processing_times_hist2D->setZAxisLabel("Number of Entries");
				 m_processing_times_hist2D->setDrawingArguments("box");

				 m_gpd_pth2.registerPlotter(m_processing_times_hist2D);

				 m_post_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
				 m_post_processing_times_hist2D->setXAxisLabel("Iteration");
				 m_post_processing_times_hist2D->setYAxisLabel("Post-processing time [s]");
				 m_post_processing_times_hist2D->setZAxisLabel("Number of Entries");
				 m_post_processing_times_hist2D->setDrawingArguments("box");

				 m_gpd_pth2.registerPlotter(m_post_processing_times_hist2D);

				 m_all_processing_times_hist2D = std::make_shared<Gem::Common::GHistogram2D>(m_nBinsX, m_nBinsY);
				 m_all_processing_times_hist2D->setXAxisLabel("Iteration");
				 m_all_processing_times_hist2D->setYAxisLabel("Overall processing time [s]");
				 m_all_processing_times_hist2D->setZAxisLabel("Number of Entries");
				 m_all_processing_times_hist2D->setDrawingArguments("box");

				 m_gpd_pth2.registerPlotter(m_all_processing_times_hist2D);

				 //---------------------------------------------------------------
				 // Make sure the output file is empty (rename, if it exists)

				 // If the file pointed to by m_fileName_txt already exists, make a back-up
				 if(bf::exists(m_fileName_txt)) {
					 std::string newFileName = m_fileName_txt + ".bak_" + Gem::Common::getMSSince1970();

					 glogger
						 << "In GProcessingTimesLoggerT<oa_type>::informationFunction(): Warning!" << std::endl
						 << "Attempt to output information to file " << m_fileName_pth2 << std::endl
						 << "which already exists. We will rename the old file to" << std::endl
						 << newFileName << std::endl
						 << GWARNING;

					 bf::rename(m_fileName_txt, newFileName);
				 }

				 //---------------------------------------------------------------

			 } break;

			 case Gem::Geneva::infoMode::INFOPROCESSING: {
				 // Open the external text-file
				 boost::filesystem::ofstream data_txt(m_fileName_txt, std::ofstream::app);

				 // Retrieve the current iteration in the population
				 double iteration = boost::numeric_cast<double>(goa->getIteration());

				 // Loop over all individuals of the algorithm.
				 for(std::size_t pos=0; pos<goa->size(); pos++) {
					 // Get access to each individual in sequence
					 std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

					 // Retrieve the processing timings
					 std::tuple<double,double,double> processingTimes = ind->getProcessingTimes();

					 double preProcessingTime = std::get<0>(processingTimes);
					 double mainProcessingTime = std::get<1>(processingTimes);
					 double postProcessingTime = std::get<2>(processingTimes);
					 double allProcessingTime = preProcessingTime + mainProcessingTime + postProcessingTime;

					 // Fill the timings into the histograms
					 m_pre_processing_times_hist->add(preProcessingTime); // PREPROCESSING
					 m_processing_times_hist->add(mainProcessingTime); // PROCESSING
					 m_post_processing_times_hist->add(postProcessingTime); // POSTPROCESSING
					 m_all_processing_times_hist->add(allProcessingTime); // OVERALL PROCESSING TIME

					 // Fill the timings into the 2D histograms ...
					 m_pre_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(preProcessingTime)); // PREPROCESSING
					 m_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(mainProcessingTime)); // PROCESSING
					 m_post_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(postProcessingTime)); // POSTPROCESSING
					 m_all_processing_times_hist2D->add(boost::numeric_cast<double>(iteration), boost::numeric_cast<double>(allProcessingTime)); // OVERALL PROCESSING TIME

					 data_txt << boost::numeric_cast<std::uint32_t>(iteration) << ", " << std::showpoint << preProcessingTime << ", " << mainProcessingTime << ", " << postProcessingTime << std::endl;
				 }

				 // Close the external text-file
				 data_txt.close();
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOEND: {
				 // Write out the results
				 m_gpd_pth.writeToFile(m_fileName_pth);
				 m_gpd_pth2.writeToFile(m_fileName_pth2);

				 // Remove all plotters
				 m_gpd_pth.resetPlotters();
				 m_gpd_pth2.resetPlotters();

				 m_pre_processing_times_hist.reset();
				 m_processing_times_hist.reset();
				 m_post_processing_times_hist.reset();
				 m_all_processing_times_hist.reset();

				 m_pre_processing_times_hist2D.reset();
				 m_processing_times_hist2D.reset();
				 m_post_processing_times_hist2D.reset();
				 m_all_processing_times_hist2D.reset();
			 }
				 break;
		 };
	 }

protected:
	 /************************************************************************/
	 /**
	  * Loads the data of another object
	  *
	  * cp A pointer to another GProcessingTimesLoggerT<oa_type> object, camouflaged as a GObject
	  */
	 virtual void load_(const GObject* cp) override {
		 // Check that we are dealing with a GProcessingTimesLoggerT<oa_type> reference independent of this object and convert the pointer
		 const GProcessingTimesLoggerT<oa_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 // Load the parent classes' data ...
		 GBasePluggableOM<oa_type>::load_(cp);

		 // ... and then our local data
		 m_fileName_pth = p_load->m_fileName_pth;
		 m_canvasDimensions_pth = p_load->m_canvasDimensions_pth;
		 m_gpd_pth = p_load->m_gpd_pth;

		 m_fileName_pth2 = p_load->m_fileName_pth2;
		 m_canvasDimensions_pth2 = p_load->m_canvasDimensions_pth2;
		 m_gpd_pth2 = p_load->m_gpd_pth2;

		 m_fileName_txt = p_load->m_fileName_txt;

		 Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processing_times_hist, m_pre_processing_times_hist);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_processing_times_hist, m_processing_times_hist);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_post_processing_times_hist, m_post_processing_times_hist);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_all_processing_times_hist, m_all_processing_times_hist);

		 Gem::Common::copyCloneableSmartPointer(p_load->m_pre_processing_times_hist2D, m_pre_processing_times_hist2D);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_processing_times_hist2D, m_processing_times_hist2D);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_post_processing_times_hist2D, m_post_processing_times_hist2D);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_all_processing_times_hist2D, m_all_processing_times_hist2D);

		 m_nBinsX = p_load->m_nBinsX;
	 }

	 /************************************************************************/
	 /**
	  * Creates a deep clone of this object
	  */
	 virtual GObject* clone_() const override {
		 return new GProcessingTimesLoggerT<oa_type>(*this);
	 }

private:
	 /***************************************************************************/

	 std::string m_fileName_pth = "processingTimingsHist.C"; ///< The name of the file to which timings should be written in ROOT format
	 std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions_pth; ///< The dimensions of the canvas
	 Gem::Common::GPlotDesigner m_gpd_pth; ///< A wrapper for the plots

	 std::string m_fileName_pth2 = "processingTimingsVsIteration.C"; ///< The name of the file to which timings should be written in ROOT format
	 std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions_pth2; ///< The dimensions of the canvas
	 Gem::Common::GPlotDesigner m_gpd_pth2; ///< A wrapper for the plots

	 std::string m_fileName_txt = "processingTimings.txt"; ///< The name of the file to which timings should be written in text format

	 std::shared_ptr<Gem::Common::GHistogram1D> m_pre_processing_times_hist;  ///< The amount of time needed for pre-processing
 	 std::shared_ptr<Gem::Common::GHistogram1D> m_processing_times_hist;  ///< The amount of time needed for processing
	 std::shared_ptr<Gem::Common::GHistogram1D> m_post_processing_times_hist;  ///< The amount of time needed for post-processing
	 std::shared_ptr<Gem::Common::GHistogram1D> m_all_processing_times_hist;  ///< The amount of time needed for the entire processing step

	 std::shared_ptr<Gem::Common::GHistogram2D> m_pre_processing_times_hist2D;  ///< The amount of time needed for pre-processing
	 std::shared_ptr<Gem::Common::GHistogram2D> m_processing_times_hist2D;  ///< The amount of time needed for processing
	 std::shared_ptr<Gem::Common::GHistogram2D> m_post_processing_times_hist2D;  ///< The amount of time needed for post-processing
	 std::shared_ptr<Gem::Common::GHistogram2D> m_all_processing_times_hist2D;  ///< The amount of time needed for the entire processing step

	 std::size_t m_nBinsX = Gem::Common::DEFAULTNBINSGPD; ///< The number of bins in the histograms in x-direction
	 std::size_t m_nBinsY = Gem::Common::DEFAULTNBINSGPD; ///< The number of bins in the histograms in y-direction

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
		 if(GBasePluggableOM<oa_type>::modify_GUnitTests()) {
			 result = true;
		 }

		 // no local data -- nothing to change

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GProcessingTimesLoggerT<oa_type>::modify_GUnitTests", "GEM_TESTING");
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
		 GBasePluggableOM<oa_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GProcessingTimesLoggerT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		 GBasePluggableOM<oa_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GProcessingTimesLoggerT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }
	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A partial specialization of the GAdaptorPropertyLoggerTT for
 * the broker executor.
 */
template <typename num_type>
class GAdaptorPropertyLoggerT
	: public GAdaptorPropertyLoggerTT<num_type, G_OptimizationAlgorithm_Base>
{
public:
	 using GAdaptorPropertyLoggerTT<num_type, G_OptimizationAlgorithm_Base>::GAdaptorPropertyLoggerTT;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

using GStandardMonitor = GStandardMonitorT<Gem::Geneva::G_OptimizationAlgorithm_Base>;
using GFitnessMonitor = GFitnessMonitorT<Gem::Geneva::G_OptimizationAlgorithm_Base>;
using GCollectiveMonitor = GCollectiveMonitorT<Gem::Geneva::G_OptimizationAlgorithm_Base>;
using GProgressPlotter = GProgressPlotterT<double>;
using GAllSolutionFileLogger = GAllSolutionFileLoggerT<Gem::Geneva::G_OptimizationAlgorithm_Base>;
using GIterationResultsFileLogger = GIterationResultsFileLoggerT<Gem::Geneva::G_OptimizationAlgorithm_Base>;
using GNAdpationsLogger = GNAdpationsLoggerT<Gem::Geneva::G_OptimizationAlgorithm_Base>;
template <typename num_type> using GAdaptorPropertyLogger = GAdaptorPropertyLoggerT<num_type>;
using GProcessingTimesLogger = GProcessingTimesLoggerT<Gem::Geneva::G_OptimizationAlgorithm_Base>;

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// Exports of classes

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GStandardMonitor)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFitnessMonitor)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GCollectiveMonitor)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GProgressPlotter)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAllSolutionFileLogger)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GIterationResultsFileLogger)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GNAdpationsLogger)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<double>)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<std::int32_t>)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<bool>)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GProcessingTimesLogger)

/******************************************************************************/

#endif /* GPLUGGABLEOPTIMIZATIONMONITORST_HPP_ */
