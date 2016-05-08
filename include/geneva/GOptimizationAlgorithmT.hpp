/**
 * @file GOptimizationAlgorithmT.hpp
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
#include <iostream>
#include <ctime>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#ifndef GOPTIMIZATIONALGORITHMT_HPP_
#define GOPTIMIZATIONALGORITHMT_HPP_

// Geneva headers go here
#include "common/GHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "courtier/GBrokerConnector2T.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GOptimizableI.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"

namespace Gem {
namespace Geneva {

/***************************************************************************/
/**
 * This class implements basic operations found in iteration-based optimization algorithms.
 * E.g., one might want to stop the optimization after a given number of cycles, or after
 * a given amount of time. The class also defines the interface functions common to these
 * algorithms, such as a general call to "optimize()".
 */
template <typename ind_type>
class GOptimizationAlgorithmT
	: public GMutableSetT<ind_type>
	, public GOptimizableI
{
public:
	// Forward declarations, as these classes are only defined at the end of this file
	class GOptimizationMonitorT;
	class GBasePluggableOMT;

	// Allow external entities to determine the type used for the individuals
	typedef ind_type individual_type;

private:
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GMutableSetT", boost::serialization::base_object<GMutableSetT<ind_type>>(*this))
		& make_nvp("GOptimizableI", boost::serialization::base_object<GOptimizableI>(*this))
		& BOOST_SERIALIZATION_NVP(iteration_)
		& BOOST_SERIALIZATION_NVP(offset_)
		& BOOST_SERIALIZATION_NVP(maxIteration_)
		& BOOST_SERIALIZATION_NVP(maxStallIteration_)
		& BOOST_SERIALIZATION_NVP(reportIteration_)
		& BOOST_SERIALIZATION_NVP(nRecordbestGlobalIndividuals_)
		& BOOST_SERIALIZATION_NVP(bestGlobalIndividuals_)
		& BOOST_SERIALIZATION_NVP(defaultPopulationSize_)
		& BOOST_SERIALIZATION_NVP(bestKnownPrimaryFitness_)
		& BOOST_SERIALIZATION_NVP(bestCurrentPrimaryFitness_)
		& BOOST_SERIALIZATION_NVP(stallCounter_)
		& BOOST_SERIALIZATION_NVP(stallCounterThreshold_)
		& BOOST_SERIALIZATION_NVP(cpInterval_)
		& BOOST_SERIALIZATION_NVP(cpBaseName_)
		& BOOST_SERIALIZATION_NVP(cpDirectory_)
		& BOOST_SERIALIZATION_NVP(cpSerMode_)
		& BOOST_SERIALIZATION_NVP(qualityThreshold_)
		& BOOST_SERIALIZATION_NVP(hasQualityThreshold_)
		& BOOST_SERIALIZATION_NVP(maxDuration_)
		& BOOST_SERIALIZATION_NVP(terminationFile_)
		& BOOST_SERIALIZATION_NVP(terminateOnFileModification_)
		& BOOST_SERIALIZATION_NVP(emitTerminationReason_)
		& BOOST_SERIALIZATION_NVP(halted_)
		& BOOST_SERIALIZATION_NVP(worstKnownValids_)
		& BOOST_SERIALIZATION_NVP(optimizationMonitor_ptr_)
		& BOOST_SERIALIZATION_NVP(default_pluggable_monitor_)
		& BOOST_SERIALIZATION_NVP(pluggable_monitors_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The default constructor. Note that most variables are initialized in the class body.
	 */
	GOptimizationAlgorithmT()
		: GMutableSetT<ind_type>()
		, bestGlobalIndividuals_(nRecordbestGlobalIndividuals_, Gem::Common::LOWERISBETTER)
	  	, bestIterationIndividuals_(0, Gem::Common::LOWERISBETTER) // unlimited size, so all individuals of an iteration fit in
		, bestKnownPrimaryFitness_(std::tuple<double,double>(0.,0.)) // will be set appropriately in the optimize() function
		, bestCurrentPrimaryFitness_(std::tuple<double,double>(0.,0.)) // will be set appropriately in the optimize() function
		, maxDuration_(boost::posix_time::duration_from_string(DEFAULTDURATION))
		, worstKnownValids_()
		, optimizationMonitor_ptr_(new typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT())
		, default_pluggable_monitor_(std::shared_ptr<typename Gem::Geneva::GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>())
		, pluggable_monitors_()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A constant reference to another GOptimizationAlgorithmT object
	 */
	GOptimizationAlgorithmT(const GOptimizationAlgorithmT<ind_type>& cp)
		: GMutableSetT<ind_type>(cp)
		, iteration_(cp.iteration_)
		, offset_(DEFAULTOFFSET)
		, maxIteration_(cp.maxIteration_)
		, maxStallIteration_(cp.maxStallIteration_)
		, reportIteration_(cp.reportIteration_)
		, nRecordbestGlobalIndividuals_(cp.nRecordbestGlobalIndividuals_)
		, bestGlobalIndividuals_(cp.bestGlobalIndividuals_)
		, bestIterationIndividuals_(cp.bestIterationIndividuals_)
		, defaultPopulationSize_(cp.defaultPopulationSize_)
		, bestKnownPrimaryFitness_(cp.bestKnownPrimaryFitness_)
		, bestCurrentPrimaryFitness_(cp.bestCurrentPrimaryFitness_)
		, stallCounter_(cp.stallCounter_)
		, stallCounterThreshold_(cp.stallCounterThreshold_)
		, cpInterval_(cp.cpInterval_)
		, cpBaseName_(cp.cpBaseName_)
		, cpDirectory_(cp.cpDirectory_)
		, cpSerMode_(cp.cpSerMode_)
		, qualityThreshold_(cp.qualityThreshold_)
		, hasQualityThreshold_(cp.hasQualityThreshold_)
		, maxDuration_(cp.maxDuration_)
		, terminationFile_(cp.terminationFile_)
		, terminateOnFileModification_(cp.terminateOnFileModification_)
		, emitTerminationReason_(cp.emitTerminationReason_)
		, halted_(cp.halted_)
		, worstKnownValids_(cp.worstKnownValids_)
		, optimizationMonitor_ptr_((cp.optimizationMonitor_ptr_)->GObject::template clone<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT>())
	{
		// Copy the pluggable optimization monitors over (if any)
		Gem::Common::copyCloneableSmartPointer(cp.default_pluggable_monitor_, default_pluggable_monitor_);
		Gem::Common::copyCloneableSmartPointerContainer(cp.pluggable_monitors_, pluggable_monitors_);
	}

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GOptimizationAlgorithmT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GOptimizationAlgorithmT<ind_type>& operator=(const GOptimizationAlgorithmT<ind_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GOptimizationAlgorithmT<ind_type> object
	 *
	 * @param  cp A constant reference to another GOptimizationAlgorithmT<ind_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GOptimizationAlgorithmT<ind_type>& cp) const {
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
	 * Checks for inequality with another GOptimizationAlgorithmT<ind_type> object
	 *
	 * @param  cp A constant reference to another GOptimizationAlgorithmT<ind_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GOptimizationAlgorithmT<ind_type>& cp) const {
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
	 * Performs the necessary administratory work of doing check-pointing
	 *
	 * @param better A boolean which indicates whether a better result was found
	 */
	void checkpoint(const bool& better) const {
		// Save checkpoints if required by the user
		if(cpInterval_ < 0 && better) saveCheckpoint();
		else if(cpInterval_ && iteration_%cpInterval_ == 0) saveCheckpoint();
	}

	/***************************************************************************/
	/** @brief Loads the state of the class from disc. */
	virtual void loadCheckpoint(const boost::filesystem::path&) = 0;

	/***************************************************************************/
	/**
	 * Checks whether the optimization process has been halted, because the halt() function
	 * has returned "true"
	 *
	 * @return A boolean indicating whether the optimization process has been halted
	 */
	bool halted() const {
		return halted_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the number of generations after which a checkpoint should be written.
	 * A negative value will result in automatic checkpointing, whenever a better solution
	 * was found.
	 *
	 * @param cpInterval The number of generations after which a checkpoint should be written
	 */
	void setCheckpointInterval(std::int32_t cpInterval) {
		cpInterval_ = cpInterval;
	}


	/***************************************************************************/
	/**
	 * Allows to retrieve the number of generations after which a checkpoint should be written
	 *
	 * @return The number of generations after which a checkpoint should be written
	 */
	std::uint32_t getCheckpointInterval() const {
		return cpInterval_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the base name of the checkpoint file and the directory where it
	 * should be stored.
	 *
	 * @param cpDirectory The directory where checkpoint files should be stored
	 * @param cpBaseName The base name used for the checkpoint files
	 */
	void setCheckpointBaseName(std::string cpDirectory, std::string cpBaseName) {
		// Do some basic checks
		if(cpBaseName == "empty" || cpBaseName.empty()) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
			<< "Error: Invalid cpBaseName: " << cpBaseName << std::endl
			<< GEXCEPTION;
		}

		if(cpDirectory == "empty" || cpDirectory.empty()) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
			<< "Error: Invalid cpDirectory: " << cpDirectory << std::endl
			<< GEXCEPTION;
		}

		cpBaseName_ = cpBaseName;

		// Check that the provided directory exists
		if(!boost::filesystem::exists(cpDirectory)) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(): Warning!" << std::endl
			<< "Directory " << cpDirectory << " does not exist and will be created automatically." << std::endl
			<< GWARNING;

			if(!boost::filesystem::create_directory(cpDirectory)) {
				glogger
				<< "In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(): Error!" << std::endl
				<< "Could not create directory " << cpDirectory << std::endl
				<< GEXCEPTION;
			}
		} else if(!boost::filesystem::is_directory(cpDirectory)) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(): Error!" << std::endl
			<< cpDirectory << " exists but is no directory." << std::endl
			<< GEXCEPTION;
		}

		// Add a trailing slash to the directory name, if necessary
		// TODO: THIS IS NOT PORTABLE TO WINDOWS!
		if(cpDirectory[cpDirectory.size() - 1] != '/') cpDirectory_ = cpDirectory + '/';
		else cpDirectory_ = cpDirectory;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the base name of the checkpoint file.
	 *
	 * @return The base name used for checkpoint files
	 */
	std::string getCheckpointBaseName() const {
		return cpBaseName_;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the directory where checkpoint files should be stored
	 *
	 * @return The base name used for checkpoint files
	 */
	std::string getCheckpointDirectory() const {
		return cpDirectory_;
	}

	/***************************************************************************/
	/**
	 * Determines whether checkpointing should be done in Text-, XML- or Binary-mode
	 *
	 * @param cpSerMode The desired new checkpointing serialization mode
	 */
	void setCheckpointSerializationMode(Gem::Common::serializationMode cpSerMode) {
		cpSerMode_ = cpSerMode;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current checkpointing serialization mode
	 *
	 * @return The current checkpointing serialization mode
	 */
	Gem::Common::serializationMode getCheckpointSerializationMode() const {
		return cpSerMode_;
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

		// Check that we are dealing with a GOptimizationAlgorithmT<ind_type> reference independent of this object and convert the pointer
		const GOptimizationAlgorithmT<ind_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GOptimizationAlgorithmT<ind_type>>(cp, this);

		GToken token("GOptimizationAlgorithmT<ind_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GMutableSetT<ind_type>>(IDENTITY(*this, *p_load), token);

		// ... and then the local data
		compare_t(IDENTITY(iteration_, p_load->iteration_), token);
		compare_t(IDENTITY(offset_, p_load->offset_), token);
		compare_t(IDENTITY(maxIteration_, p_load->maxIteration_), token);
		compare_t(IDENTITY(maxStallIteration_, p_load->maxStallIteration_), token);
		compare_t(IDENTITY(reportIteration_, p_load->reportIteration_), token);
		compare_t(IDENTITY(nRecordbestGlobalIndividuals_, p_load->nRecordbestGlobalIndividuals_), token);
		compare_t(IDENTITY(bestGlobalIndividuals_, p_load->bestGlobalIndividuals_), token);
		compare_t(IDENTITY(bestIterationIndividuals_, p_load->bestIterationIndividuals_), token);
		compare_t(IDENTITY(defaultPopulationSize_, p_load->defaultPopulationSize_), token);
		compare_t(IDENTITY(bestKnownPrimaryFitness_, p_load->bestKnownPrimaryFitness_), token);
		compare_t(IDENTITY(bestCurrentPrimaryFitness_, p_load->bestCurrentPrimaryFitness_), token);
		compare_t(IDENTITY(stallCounter_, p_load->stallCounter_), token);
		compare_t(IDENTITY(stallCounterThreshold_, p_load->stallCounterThreshold_), token);
		compare_t(IDENTITY(cpInterval_, p_load->cpInterval_), token);
		compare_t(IDENTITY(cpBaseName_, p_load->cpBaseName_), token);
		compare_t(IDENTITY(cpDirectory_, p_load->cpDirectory_), token);
		compare_t(IDENTITY(cpSerMode_, p_load->cpSerMode_), token);
		compare_t(IDENTITY(qualityThreshold_, p_load->qualityThreshold_), token);
		compare_t(IDENTITY(hasQualityThreshold_, p_load->hasQualityThreshold_), token);
		compare_t(IDENTITY(maxDuration_, p_load->maxDuration_), token);
		compare_t(IDENTITY(terminationFile_, p_load->terminationFile_), token);
		compare_t(IDENTITY(terminateOnFileModification_, p_load->terminateOnFileModification_), token);
		compare_t(IDENTITY(emitTerminationReason_, p_load->emitTerminationReason_), token);
		compare_t(IDENTITY(halted_, p_load->halted_), token);
		compare_t(IDENTITY(worstKnownValids_, p_load->worstKnownValids_), token);
		compare_t(IDENTITY(optimizationMonitor_ptr_, p_load->optimizationMonitor_ptr_), token);
		compare_t(IDENTITY(default_pluggable_monitor_, p_load->default_pluggable_monitor_), token);
		compare_t(IDENTITY(pluggable_monitors_, p_load->pluggable_monitors_), token);

		// React on deviations from the expectation
		token.evaluate();
	}


	/***************************************************************************/
	/**
	 * This function encapsulates some common functionality of iteration-based
	 * optimization algorithms. E.g., they all need a loop that stops if some
	 * predefined criterion is reached. This function is also the main entry
	 * point for all optimization algorithms.
	 *
	 * @param offset Specifies the iteration number to start with (e.g. useful when starting from a checkpoint file)
	 */
	virtual void optimize(const std::uint32_t& offset) override {
		// Reset the generation counter
		iteration_ = offset;

		// Set the iteration offset
		offset_ = offset;

		// Let the algorithm know that the optimization process hasn't been halted yet
		halted_ = false; // general halt criterion

		// Store any *clean* individuals that have been added to this algorithm
		// in the priority queue. This happens so that best individuals from a
		// previous "chained" optimization run aren't lost.
		addCleanStoredBests(bestGlobalIndividuals_);

		// Resize the population to the desired size and do some error checks.
		// This function will also check that individuals have indeed been registered
		adjustPopulation();

		// Set the individual's personalities (some algorithm-specific information needs to be stored
		// in individuals. Optimization algorithms need to re-implement this function to add
		// the required functionality.)
		setIndividualPersonalities();

		// Emit the info header, unless we do not want any info (parameter 0).
		// Note that this call needs to come after the initialization, so we have the
		// complete set of individuals available.
		if(reportIteration_) doInfo(infoMode::INFOINIT);

		// We want to know if no better values were found for a longer period of time
		double worstCase = this->getWorstCase();
		bestKnownPrimaryFitness_   = std::make_tuple(worstCase, worstCase);
		bestCurrentPrimaryFitness_ = std::make_tuple(worstCase, worstCase);

		stallCounter_ = 0;

		// Initialize the start time with the current time.
		startTime_ = boost::posix_time::microsec_clock::universal_time();

		// Give derived classes the opportunity to perform any necessary preparatory work.
		init();

		do {
			// Let all individuals know the current iteration
			markIteration();

			// Update fitness values and the stall counter
			updateStallCounter((bestCurrentPrimaryFitness_=cycleLogic()));

			// Add the best individuals to the bestGlobalIndividuals_
			// and bestIterationIndividuals_ vectors
			updateGlobalBestsPQ(bestGlobalIndividuals_);
			updateIterationBestsPQ(bestIterationIndividuals_);

			// Check whether a better value was found, and do the check-pointing, if necessary and requested.
			checkpoint(progress());

			// Let all individuals know about the best fitness known so far
			markBestFitness();

			// Let individuals know about the stalls encountered so far
			markNStalls();

			// Give derived classes an opportunity to act on stalls. NOTE that no action
			// may be taken that affects the "dirty" state of individuals
			if(stallCounterThreshold_ && stallCounterThresholdExceeded()) {
				actOnStalls();
			}

			// We want to provide feedback to the user in regular intervals.
			// Set the reportGeneration_ variable to 0 in order not to emit
			// any information at all.
			if(reportIteration_ && (iteration_%reportIteration_ == 0)) {
				doInfo(infoMode::INFOPROCESSING);
			}

			// update the iteration_ counter
			iteration_++;
		}
		while(!(halted_ = halt()));

		// Give derived classes the opportunity to perform any remaining clean-up work
		finalize();

		// Finalize the info output
		if(reportIteration_) doInfo(infoMode::INFOEND);

		// Remove information particular to the optimization algorithms from the individuals
		resetIndividualPersonalities();
	}

	/***************************************************************************/
	/**
	 * A little convenience function that helps to avoid having to specify explicit scopes
	 */
	virtual void optimize() override {
		GOptimizableI::optimize();
	}

	/***************************************************************************/
	/**
	 * Emits information specific to this class. The function can be overloaded
	 * in derived classes and it indeed makes sense to emit much more information
	 * than is done in this simple implementation.
	 *
	 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
	 */
	virtual void doInfo(const infoMode& im) BASE {
#ifdef DEBUG
		if(!optimizationMonitor_ptr_) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::doInfo():" << std::endl
			<< "optimizationMonitor_ptr_ is empty when it shouldn't be." << std::endl
			<< GEXCEPTION;
		}
#endif /* DEBUG */

		(this->optimizationMonitor_ptr_)->informationFunction(im, this);
	}

	/***************************************************************************/
	/**
	 * Checks whether a better solution was found. If so, the stallCounter_
	 * variable will have been set to 0
	 *
	 * @return A boolean indicating whether a better solution was found
	 */
	bool progress() const {
		return (0==stallCounter_);
	}

	/***************************************************************************/
	/**
	 * Allows to register the default pluggable optimization monitor. GOptimizationAlgorithmT<>
	 * registers a default monitor, which will essentially only output information about the
	 * current fitness and iteration. You can override this setting and store your own monitor
	 * with this function. The function will throw if you try to register an empty pointer, and
	 * there is no way to reset the monitor externally. Note that this function does NOT take
	 * ownership of the optimization monitor.
	 */
	void registerDefaultPluggableOM(
		std::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT> pluggableOM
	) {
		if(pluggableOM) {
			default_pluggable_monitor_ = pluggableOM;
		} else {
			glogger
			<< "In GoptimizationAlgorithmT<>::registerDefaultPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * Allows to register a pluggable optimization monitor. Note that this
	 * function does NOT take ownership of the optimization monitor.
	 */
	void registerPluggableOM(
		std::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT> pluggableOM
	) {
		if(pluggableOM) {
			pluggable_monitors_.push_back(pluggableOM);
		} else {
			glogger
			<< "In GoptimizationMonitorT<>::registerPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
			<< GEXCEPTION;
		}
	}

	/***************************************************************************/
	/**
	 * Registers an optimizationMonitor object (or a derivative) with this object. Note
	 * that this class will take ownership of the optimization monitor by cloning it.
	 * You can thus assign the same std::shared_ptr<GOptimizationAlgorithmT<ind_type>>
	 * to different objects.
	 *
	 * @param om_ptr A shared pointer to a specific optimization monitor
	 */
	void registerOptimizationMonitor(std::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT> om_ptr) {
#ifdef DEBUG
		if(!om_ptr) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::registerOptimizationMonitor():" << std::endl
			<< "om_ptr is empty when it shouldn't be." << std::endl
			<< GEXCEPTION;
		}
#endif /* DEBUG */

		this->optimizationMonitor_ptr_ = om_ptr->GObject::template clone<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT>();
	}

	/***************************************************************************/
	/**
	 * Retrieves the default population size
	 *
	 * @return The default population size
	 */
	std::size_t getDefaultPopulationSize() const {
		return defaultPopulationSize_;
	}

	/***************************************************************************/
	/**
	 * Retrieve the current population size
	 *
	 * @return The current population size
	 */
	std::size_t getPopulationSize() const {
		return this->size();
	}

	/***************************************************************************/
	/**
	 * Set the number of iterations after which the optimization should
	 * be stopped
	 *
	 * @param maxIteration The number of iterations after which the optimization should terminate
	 */
	void setMaxIteration(std::uint32_t maxIteration) {
		maxIteration_ = maxIteration;
	}

	/***************************************************************************/
	/**
	 * Retrieve the number of iterations after which optimization should
	 * be stopped
	 *
	 * @return The number of iterations after which the optimization should terminate
	 */
	std::uint32_t getMaxIteration() const {
		return maxIteration_;
	}

	/***************************************************************************/
	/**
	 * Sets the maximum number of generations allowed without improvement of the best
	 * individual. Set to 0 in order for this stop criterion to be disabled.
	 *
	 * @param The maximum number of allowed generations
	 */
	void setMaxStallIteration(std::uint32_t maxStallIteration) {
		maxStallIteration_ = maxStallIteration;
	}

	/***************************************************************************/
	/**
	 * Retrieves the maximum number of generations allowed in an optimization run without
	 * improvement of the best individual.
	 *
	 * @return The maximum number of generations
	 */
	std::uint32_t getMaxStallIteration() const {
		return maxStallIteration_;
	}

	/***************************************************************************/
	/**
	 * Sets the maximum allowed processing time
	 *
	 * @param maxDuration The maximum allowed processing time
	 */
	void setMaxTime(boost::posix_time::time_duration maxDuration) {
		using namespace boost::posix_time;

		// Only allow "real" values
		if(maxDuration.is_special() || maxDuration.is_negative()) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::setMaxTime() :" << std::endl
			<< "Invalid maxDuration." << std::endl
			<< GEXCEPTION;
		}

		maxDuration_ = maxDuration;
	}

	/***************************************************************************/
	/**
	 * Retrieves the value of the maxDuration_ parameter.
	 *
	 * @return The maximum allowed processing time
	 */
	boost::posix_time::time_duration getMaxTime() const {
		return maxDuration_;
	}

	/***************************************************************************/
	/**
	 *  Sets a quality threshold beyond which optimization is expected to stop
	 *
	 *  @param qualityThreshold A threshold beyond which optimization should stop
	 *  @param hasQualityThreshold Allows to (de-)activate the quality threshold
	 */
	void setQualityThreshold(double qualityThreshold, bool hasQualityThreshold) {
		qualityThreshold_ = qualityThreshold;
		hasQualityThreshold_=hasQualityThreshold;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current value of the quality threshold and also indicates whether
	 * the threshold is active
	 *
	 * @param hasQualityThreshold A boolean indicating whether a quality threshold has been set
	 * @return The current value of the quality threshold
	 */
	double getQualityThreshold(bool& hasQualityThreshold) const {
		hasQualityThreshold = hasQualityThreshold_;
		return qualityThreshold_;
	}

	/***************************************************************************/
	/**
	 *  Sets the name of a "termination file" (optimization is supposed to
	 *  stop when the modification time of this file is more recent than the
	 *  start of the optimizatoon rn.
	 *
	 *  @param terminationFile The name of a file used to initiate termination
	 *  @param hasQualityThreshold Allows to (de-)activate "touched termination"
	 */
	void setTerminationFile(std::string terminationFile, bool terminateOnFileModification) {
		terminationFile_ = terminationFile;
		terminateOnFileModification_ = terminateOnFileModification;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current name of the termination file and also indicates whether
	 * the "touched halt" is active
	 *
	 * @param terminateOnFileModification A boolean indicating whether "touched termination" is active
	 * @return The current value of the terminationFile_ variable
	 */
	std::string getTerminationFile(bool& terminateOnFileModification) const {
		terminateOnFileModification = terminateOnFileModification_;
	 	return terminationFile_;
	}

	/***************************************************************************/
	/**
	 * Removes the quality threshold
	 */
	void resetQualityThreshold() {
		hasQualityThreshold_ = false;
	}

	/***************************************************************************/
	/**
	 * Checks whether a quality threshold has been set
	 *
	 * @return A boolean indicating whether a quality threshold has been set
	 */
	bool hasQualityThreshold() const {
		return hasQualityThreshold_;
	}

	/***************************************************************************/
	/**
	 * Retrieve the current iteration of the optimization run
	 *
	 * @return The current iteration of the optimization run
	 */
	virtual std::uint32_t getIteration() const override {
		return iteration_;
	}

	/***************************************************************************/
	/**
	 * Returns the current offset used to calculate the current iteration. This
	 * is identical to the iteration the optimization starts with.
	 *
	 * @return The current iteration offset
	 */
	std::uint32_t getStartIteration() const {
		return offset_;
	}

	/***************************************************************************/
	/**
	 * Sets the number of iterations after which the algorithm should
	 * report about its inner state.
	 *
	 * @param iter The number of iterations after which information should be emitted
	 */
	void setReportIteration(std::uint32_t iter) {
		reportIteration_ = iter;
	}

	/***************************************************************************/
	/**
	 * Returns the number of iterations after which the algorithm should
	 * report about its inner state.
	 *
	 * @return The number of iterations after which information is emitted
	 */
	std::uint32_t getReportIteration() const {
		return reportIteration_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the current number of failed optimization attempts
	 *
	 * @return The current number of failed optimization attempts
	 */
	std::uint32_t getStallCounter() const {
		return stallCounter_;
	}

	/***************************************************************************/
	/**
	 * Allows to set the number of iterations without improvement, after which
	 * individuals are asked to update their internal data structures
	 */
	void setStallCounterThreshold(std::uint32_t stallCounterThreshold) {
		stallCounterThreshold_ = stallCounterThreshold;
	}

	/***************************************************************************/
	/**
	 * Allows to retrieve the number of iterations without improvement, after which
	 * individuals are asked to update their internal data structures
	 */
	std::uint32_t getStallCounterThreshold() const {
		return stallCounterThreshold_;
	}

	/***************************************************************************/
	/**
	 * Retrieve the best value found in the entire optimization run so far
	 *
	 * @return The best raw and transformed fitness found so far
	 */
	std::tuple<double, double> getBestKnownPrimaryFitness() const {
		return (bestGlobalIndividuals_.best())->getFitnessTuple();

		// return bestKnownPrimaryFitness_;
	}

	/***************************************************************************/
	/**
	 * Retrieves the best value found in the current iteration
	 *
	 * @return The best raw and transformed fitness found in the current iteration
	 */
	std::tuple<double, double> getBestCurrentPrimaryFitness() const {
		return bestCurrentPrimaryFitness_;
	}

	/***************************************************************************/
	/**
	 * Specifies whether information about termination reasons should be emitted
	 *
	 * @param etr A boolean which specifies whether reasons for the termination of the optimization run should be emitted
	 */
	void setEmitTerminationReason(bool emitTerminatioReason = true) {
		emitTerminationReason_ = emitTerminatioReason;
	}

	/***************************************************************************/
	/**
	 * Retrieves information on whether information about termination reasons should be emitted
	 *
	 * @return A boolean which specifies whether reasons for the termination of the optimization run will be emitted
	 */
	bool getEmitTerminationReason() const {
		return emitTerminationReason_;
	}

	/***************************************************************************/
	/**
	 * This function converts an individual at a given position to the derived
	 * type and returns it. In DEBUG mode, the function will check whether the
	 * requested position exists.
	 *
	 * Note that this function will only be accessible to the compiler if ind_type
	 * is a derivative of GOptimizableEntity, thanks to the magic of std::enable_if
	 * and type_traits.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GOptimizableEntity object, as required by the user
	 */
	template <typename target_type>
	std::shared_ptr<target_type> individual_cast(
		const std::size_t& pos
		, typename std::enable_if<std::is_base_of<GOptimizableEntity, target_type>::value>::type *dummy = nullptr
	) {
#ifdef DEBUG
		if(pos >= this->size()) {
			glogger
			<< "In GOptimizationAlgorithmT<ind_type>::individual_cast<>() : Error" << std::endl
			<< "Tried to access position " << pos << " which is >= array size " << this->size() << std::endl
			<< GEXCEPTION;

			// Make the compiler happy
			return std::shared_ptr<target_type>();
		}
#endif /* DEBUG */

		// Does error checks on the conversion internally
		return Gem::Common::convertSmartPointer<ind_type, target_type>(this->at(pos));
	}

	/***************************************************************************/
	/**
	 * This function is e.g. called from GSerialEA::adjustPopulation(). It
	 * currently only triggers actions for GParameterSet-derivatives. Optimization algorithms
	 * are unaffected. It might be useful to implement actions here as well, though, in order
	 * to make better use of Multi-Populations in Evolutionary Algorithms.
	 */
	virtual bool randomInit(const activityMode&) override {
		return false;
	}

	/***************************************************************************/
	/**
	 * Retrieve the number of processable items in the current iteration. This function should
	 * be overloaded for derived classes. It is used to determine a suitable wait factor for
	 * networked execution.
	 *
	 * @return The number of processable items in the current iteration
	 */
	virtual std::size_t getNProcessableItems() const {
		return this->size();
	}

	/***************************************************************************/
	/**
	 * Gives access to the current optimization monitor
	 *
	 * @return A std::shared_ptr to the current optimization monitor
	 */
	std::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT> getOptimizationMonitor() {
		return this->optimizationMonitor_ptr_;
	}

	/***************************************************************************/
	/**
	 * Adds local configuration options to a GParserBuilder object
	 *
	 * @param gpb The GParserBuilder object to which configuration options should be added
	 */
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
	) override {
		// Call our parent class'es function
		GMutableSetT<ind_type>::addConfigurationOptions(gpb);

		// Add local data
		gpb.registerFileParameter<std::uint32_t>(
			"maxIteration" // The name of the variable
			, DEFAULTMAXIT // The default value
			, [this](std::uint32_t maxIt){ this->setMaxIteration(maxIt); }
		)
		<< "The maximum allowed number of iterations";

		gpb.registerFileParameter<std::uint32_t>(
			"maxStallIteration" // The name of the variable
			, DEFAULTMAXSTALLIT // The default value
			, [this](std::uint32_t maxStallIt){ this->setMaxStallIteration(maxStallIt); }
		)
		<< "The maximum allowed number of iterations without improvement" << std::endl
		<< "0 means: no constraint.";

		gpb.registerFileParameter<std::string, bool>(
			"terminationFile" // The name of the variable
			, "touchedTerminationActive"
			, DEFAULTTERMINATIONFILE // The default value
			, false
			, [this](std::string tf, bool tfa){ this->setTerminationFile(tf, tfa); }
			, "touchedTermination"
		)
		<< "The name of a file which, when modified after the start of an" << std::endl
		<< "optimization run, instructs Geneva to terminate optimitation." << std::endl
		<< "This can be used to \"touch a file\" after the start of an optimization" << std::endl
		<< "run, which will lead to the termination of the run after the current iteration." << Gem::Common::nextComment()
		<< "Activates (1) or de-activates (0) the \"touched termination\"";

		gpb.registerFileParameter<std::uint32_t>(
			"indivdualUpdateStallCounterThreshold" // The name of the variable
			, DEFAULTSTALLCOUNTERTHRESHOLD // The default value
			, [this](std::uint32_t stallCounterThreshold){ this->setStallCounterThreshold(stallCounterThreshold); }
		)
		<< "The number of iterations without improvement after which" << std::endl
		<< "individuals are asked to update their internal data structures" << std::endl
		<< "through the actOnStalls() function. A value of 0 disables this check";

		gpb.registerFileParameter<std::uint32_t>(
			"reportIteration" // The name of the variable
			, DEFAULTREPORTITER // The default value
			, [this](std::uint32_t rI){ this->setReportIteration(rI); }
		)
		<< "The number of iterations after which a report should be issued";

		gpb.registerFileParameter<std::size_t>(
			"nRecordBestIndividuals" // The name of the variable
			, DEFNRECORDBESTINDIVIDUALS // The default value
			, [this](std::size_t nRecBI){ this->setNRecordBestIndividuals(nRecBI); }
		)
		<< "Indicates how many \"best\" individuals should be recorded in each iteration";

		gpb.registerFileParameter<std::int32_t>(
			"cpInterval" // The name of the variable
			, DEFAULTCHECKPOINTIT // The default value
			, [this](std::int32_t cpI){ this->setCheckpointInterval(cpI); }
		)
		<< "The number of iterations after which a checkpoint should be written." << std::endl
		<< "-1 means: Write a checkpoint file whenever an improvement was encountered" << std::endl
		<< " 0 means: Never emit checkpoint files.";

		gpb.registerFileParameter<std::string, std::string>(
			"cpDirectory"  // The name of the first variable
			, "cpBaseName" // The name of the second variable
			, DEFAULTCPDIR // Default value for the first variable
			, DEFAULTCPBASENAME // Default value for the second variable
			, [this](std::string cpDir, std::string cpBN){ this->setCheckpointBaseName(cpDir, cpBN); }
			, "checkpointLocation"
		)
		<< "The directory where checkpoint files should be stored." << Gem::Common::nextComment() // comments for the second option follow
		<< "The significant part of the checkpoint file name.";

		gpb.registerFileParameter<Gem::Common::serializationMode>(
			"cpSerMode" // The name of the variable
			, DEFAULTCPSERMODE // The default value
			, [this](Gem::Common::serializationMode sM){ this->setCheckpointSerializationMode(sM); }
		)
		<< "Determines whether check-pointing should be done in" << std::endl
		<< "text- (0), XML- (1), or binary-mode (2)";

		gpb.registerFileParameter<double, bool>(
			"threshold" // The name of the variable
			, "thresholdActive"
			, DEFAULTQUALITYTHRESHOLD // The default value
			, false
			, [this](double qt, bool ta){ this->setQualityThreshold(qt, ta); }
			, "qualityTermination"
		)
		<< "A threshold beyond which optimization is expected to stop" << std::endl
		<< "Note that in order to activate this threshold, you also need to" << std::endl
		<< "set \"hasQualityThreshold\" to 1." << Gem::Common::nextComment()
		<< "Activates (1) or de-activates (0) the quality threshold";

		gpb.registerFileParameter<boost::posix_time::time_duration>(
			"maxDuration" // The name of the variable
			, boost::posix_time::duration_from_string(DEFAULTDURATION) // The default value
			, [this](boost::posix_time::time_duration mt){ this->setMaxTime(mt); }
		)
		<< "The maximum allowed time-frame for the optimization";

		gpb.registerFileParameter<bool>(
			"emitTerminationReason" // The name of the variable
			, DEFAULTEMITTERMINATIONREASON // The default value
			, [this](bool etr){ this->setEmitTerminationReason(etr); }
		)
		<< "Triggers emission (1) or omission (0) of information about reasons for termination";
	}

	/***************************************************************************/
	/**
	 * Allows to assign a name to the role of this individual(-derivative). This is mostly important for the
	 * GBrokerEA class which should prevent objects of its type from being stored as an individual in its population.
	 * All other objects do not need to re-implement this function (unless they rely on the name for some reason).
	 */
	virtual std::string getIndividualCharacteristic() const override {
		return std::string("GENEVA_OPTIMIZATIONALGORITHM");
	}

	/******************************************************************************/
	/**
	 * Retrieves a parameter of a given type at the specified position
	 */
	virtual boost::any getVarVal(
		const std::string& descr
		, const std::tuple<std::size_t, std::string, std::size_t>& target
	) override {
		return GOptimizableI::getBestGlobalIndividual<GParameterSet>()->getVarVal(descr, target);
	}

	/***************************************************************************/
	/**
	 * Adds the best individuals of each iteration to a priority queue. The
	 * queue will be sorted by the first evaluation criterion of the individuals
	 * and may either have a limited or unlimited size. Note: this function is
	 * a trap -- the real action happens in overloads of this function, of which
	 * the one for GParameterSet-derivatives is likely the most important.
	 */
	virtual void updateGlobalBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {
		glogger
		<< "In GOptimizationAlgorithmT<ind_type>::updateGlobalBestsPQ(): Error!" << std::endl
		<< "This function should not have been called" << std::endl
		<< GEXCEPTION;
	}

	/***************************************************************************/
	/**
	 * Adds the best individuals of each iteration to a priority queue, which is cleared
	 * along the way, so results are only relevant for a given iteration. Note: this function is
	 * a trap -- the real action happens in overloads of this function, of which
	 * the one for GParameterSet-derivatives is likely the most important.
	 */
	virtual void updateIterationBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {
		glogger
		<< "In GOptimizationAlgorithmT<ind_type>::updateIterationBestsPQ(): Error!" << std::endl
		<< "This function should not have been called" << std::endl
		<< GEXCEPTION;
	}

	/***************************************************************************/
	/**
	 * If individuals have been stored in this population, they are added to the
	 * priority queue. This happens before the optimization cycle starts, so that
	 * best individuals from a previous "chained" optimization run aren't lost.
	 * Only those individuals are stored in the priority queue that do not have the
	 * "dirty flag" set.
	 */
	virtual void addCleanStoredBests(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {
		glogger
		<< "In GOptimizationAlgorithmT<ind_type>::addCleanStoredBests(): Error!" << std::endl
		<< "This function should not have been called" << std::endl
		<< GEXCEPTION;
	}

	/***************************************************************************/

	/** @brief Emits a name for this class / object; this can be a long name with spaces */
	virtual std::string name() const override = 0;

	/***************************************************************************/
	/**
	 * Helps to determine whether a given value is strictly better (i.e. better than equal)
	 * than another one. As "better" means something different for maximization and minimization,
	 * this function helps to make the code easier to understand. This function requires
	 * that at least one individual has been registered with the algorithm.
	 *
	 * @param newValue The new value
	 * @param oldValue The old value
	 * @return true if newValue is better than oldValue, otherwise false.
	 */
	virtual bool isBetter(double newValue, const double& oldValue) const override {
#ifdef DEBUG
		if(this->empty()) {
			glogger
			<< "In GOptimizationAlgorithmT<>::isBetter(): Error!" << std::endl
			<< "No individuals have been registered" << std::endl
			<< GEXCEPTION;
		}
#endif

		return this->at(0)->isBetter(newValue, oldValue);
	}

	/***************************************************************************/
	/**
	 * Helps to determine whether a given value is strictly worse (i.e. worse than equal)
	 * than another one. As "worse" means something different for maximization and minimization,
	 * this function helps to make the code easier to understand. This function requires
	 * that at least one individual has been registered with the algorithm.
	 *
	 * @param newValue The new value
	 * @param oldValue The old value
	 * @return true of newValue is worse than oldValue, otherwise false.
	 */
	virtual bool isWorse(double newValue, const double& oldValue) const override {
#ifdef DEBUG
		if(this->empty()) {
			glogger
			<< "In GOptimizationAlgorithmT<>::isWorse(): Error!" << std::endl
			<< "No individuals have been registered" << std::endl
			<< GEXCEPTION;
		}
#endif

		return this->at(0)->isWorse(newValue, oldValue);
	}

	/***************************************************************************/
	/**
	 * Retrieves the worst possible evaluation result, depending on whether we are
	 * in maximization or minimization mode
	 */
	virtual double getWorstCase() const override {
#ifdef DEBUG
		if(this->empty()) {
			glogger
			<< "In GOptimizationAlgorithmT<>::getWorstCase(): Error!" << std::endl
			<< "No individuals have been registered" << std::endl
			<< GEXCEPTION;
		}
#endif

		return this->at(0)->getWorstCase();
	}

	/***************************************************************************/
	/**
	 * Retrieves the best possible evaluation result, depending on whether we are
	 * in maximization or minimization mode
	 */
	virtual double getBestCase() const override {
#ifdef DEBUG
		if(this->empty()) {
			glogger
			<< "In GOptimizationAlgorithmT<>::getBestCase(): Error!" << std::endl
			<< "No individuals have been registered" << std::endl
			<< GEXCEPTION;
		}
#endif

		return this->at(0)->getBestCase();
	}

	/***************************************************************************/
	/**
	 * A little helper function that determines whether we are currently inside of the first
	 * iteration
	 *
	 * @return A boolean indicating whether we are inside of the first iteration
	 */
	bool inFirstIteration() const {
		return iteration_ == offset_;
	}

	/***************************************************************************/
	/**
	 * A little helper function that determines whether we are after the first iteration
	 *
	 * @return A boolean indicating whether we are after the first iteration
	 */
	bool afterFirstIteration() const {
		return iteration_ > offset_;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GOptimizationAlgorithm object
	 *
	 * @param cp Another GOptimizationAlgorithm object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp) override {
		// Check that we are dealing with a GOptimizationAlgorithmT<ind_type> reference independent of this object and convert the pointer
		const GOptimizationAlgorithmT<ind_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GOptimizationAlgorithmT<ind_type>>(cp, this);

		// Load the parent class'es data
		GMutableSetT<ind_type>::load_(cp);

		// and then our local data
		iteration_ = p_load->iteration_;
		offset_ = p_load->offset_;
		maxIteration_ = p_load->maxIteration_;
		maxStallIteration_ = p_load->maxStallIteration_;
		reportIteration_ = p_load->reportIteration_;
		nRecordbestGlobalIndividuals_ = p_load->nRecordbestGlobalIndividuals_;
		bestGlobalIndividuals_ = p_load->bestGlobalIndividuals_;
		bestIterationIndividuals_ = p_load->bestIterationIndividuals_;
		defaultPopulationSize_ = p_load->defaultPopulationSize_;
		bestKnownPrimaryFitness_ = p_load->bestKnownPrimaryFitness_;
		bestCurrentPrimaryFitness_ = p_load->bestCurrentPrimaryFitness_;
		stallCounter_ = p_load->stallCounter_;
		stallCounterThreshold_ = p_load->stallCounterThreshold_;
		cpInterval_ = p_load->cpInterval_;
		cpBaseName_ = p_load->cpBaseName_;
		cpDirectory_ = p_load->cpDirectory_;
		cpSerMode_ = p_load->cpSerMode_;
		qualityThreshold_ = p_load->qualityThreshold_;
		hasQualityThreshold_ = p_load->hasQualityThreshold_;
		terminationFile_ = p_load->terminationFile_;
		terminateOnFileModification_ = p_load->terminateOnFileModification_;
		maxDuration_ = p_load->maxDuration_;
		emitTerminationReason_ = p_load->emitTerminationReason_;
		halted_ = p_load->halted_;
		worstKnownValids_ = p_load->worstKnownValids_;
		this->optimizationMonitor_ptr_ = p_load->optimizationMonitor_ptr_->GObject::template clone<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT>();
		Gem::Common::copyCloneableSmartPointer(p_load->default_pluggable_monitor_, default_pluggable_monitor_);
		Gem::Common::copyCloneableSmartPointerContainer(p_load->pluggable_monitors_, pluggable_monitors_);
	}

	/***************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const override = 0;

	/***************************************************************************/
	/**
	 * Retrieves the best individual found up to now (which is the best individual
	 * in the priority queue).
	 */
	virtual std::shared_ptr<GParameterSet> customGetBestGlobalIndividual() override {
#ifdef DEBUG
		std::shared_ptr<GParameterSet> p = bestGlobalIndividuals_.best();
		if(p) return p;
		else {
			glogger
			<< "In GOptimizationAlgorithmT<T>::customGetBestGlobalIndividual(): Error!" << std::endl
			<< "Best individual seems to be empty" << std::endl
			<< GEXCEPTION;

			// Make the compiler happy
			return std::shared_ptr<GParameterSet>();
		}
#else
		return bestGlobalIndividuals_.best();
#endif
	}

	/***************************************************************************/
	/**
	 * Retrieves a list of the best individuals found (equal to the content of
	 * the priority queue)
	 */
	virtual std::vector<std::shared_ptr<GParameterSet>> customGetBestGlobalIndividuals() override {
		return bestGlobalIndividuals_.toVector();
	}

	/***************************************************************************/
	/**
	 * Retrieves the best individual found in the iteration (which is the best individual
	 * in the priority queue).
	 */
	virtual std::shared_ptr<GParameterSet> customGetBestIterationIndividual() override {
#ifdef DEBUG
		std::shared_ptr<GParameterSet> p = bestIterationIndividuals_.best();
		if(p) return p;
		else {
			glogger
			<< "In GOptimizationAlgorithmT<T>::customGetBestIterationIndividual(): Error!" << std::endl
			<< "Best individual seems to be empty" << std::endl
			<< GEXCEPTION;

			// Make the compiler happy
			return std::shared_ptr<GParameterSet>();
		}
#else
		return bestIterationIndividuals_.best();
#endif
	}

	/***************************************************************************/
	/**
	 * Retrieves a list of the best individuals found in the iteration (equal to the content of
	 * the priority queue)
	 */
	virtual std::vector<std::shared_ptr<GParameterSet>> customGetBestIterationIndividuals() override {
		return bestIterationIndividuals_.toVector();
	}

	/***************************************************************************/
	/**
	 * Allows to set the personality type of the individuals
	 */
	virtual void setIndividualPersonalities() {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setPersonality(this->getPersonalityTraits());
		}
	}

	/***************************************************************************/
	/**
	 * Resets the individual's personality types
	 */
	void resetIndividualPersonalities() {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) (*it)->resetPersonality();
	}

	/***************************************************************************/
	/** @brief Saves the state of the class to disc */
	virtual void saveCheckpoint() const = 0;

	/***************************************************************************/
	/** @brief The actual business logic to be performed during each iteration */
	virtual std::tuple<double, double> cycleLogic() BASE = 0;

	/***************************************************************************/
	/**
	 * Sets the default size of the population
	 *
	 * @param popSize The desired size of the population
	 */
	virtual void setDefaultPopulationSize(const std::size_t& defPopSize) BASE {
		defaultPopulationSize_ = defPopSize;
	}

	/***************************************************************************/
	/**
	 * Set the number of "best" individuals to be recorded in each iteration
	 *
	 * @param nRecordBestIndividuals The number of "best" individuals to be recorded in each iteration
	 */
	void setNRecordBestIndividuals(std::size_t nRecordBestIndividuals) {
		if(0 == nRecordBestIndividuals) {
			glogger
			<< "In GOptimizationAlgorithmT<>::setNRecordBestIndividuals(): Error!" << std::endl
			<< "Invalid number of individuals to be recorded: " << nRecordBestIndividuals << std::endl
			<< GEXCEPTION;
		}

		nRecordbestGlobalIndividuals_ = nRecordBestIndividuals;
		bestGlobalIndividuals_.setMaxSize(nRecordbestGlobalIndividuals_);
	}

	/***************************************************************************/
	/**
	 * Retrieve the number of best individuals to be recorded in each iteration
	 *
	 * @return The number of best individuals to be recorded in each iteration
	 */
	std::size_t getNRecordBestIndividuals() const {
		return nRecordbestGlobalIndividuals_;
	}

	/***************************************************************************/
	/**
	 * It is possible for derived classes to specify in overloaded versions of this
	 * function under which conditions the optimization should be stopped. The
	 * function is called from GOptimizationAlgorithmT<ind_type>::halt .
	 *
	 * @return boolean indicating that a stop condition was reached
	 */
	virtual bool customHalt() const BASE {
		/* nothing - specify your own criteria in derived classes. Make sure
		 * to emit a suitable message if execution was halted due to a
		 * custom criterion */
		return false;
	}

	/***************************************************************************/
	/**
	 * Fitness calculation for a population means optimization. The fitness is then determined
	 * by the best individual which, after the end of the optimization cycle, can be found in
	 * the first position of the array. Note that this function will only take into account the
	 * fitness of the first registered evaluation criterion in an individual.
	 *
	 * @param The id of an evaluation criterion (will be ignored by this function)
	 * @return The fitness of the best individual in the population
	 */
	virtual double fitnessCalculation() override {
		// Make sure the population is optimized
		GOptimizableI::optimize();

		// We use the raw fitness rather than the transformed fitness,
		// as this is custom also for "normal" individuals. Re-evaluation
		// at this point should never happen.
		return this->at(0)->fitness(0, Gem::Geneva::PREVENTREEVALUATION, Gem::Geneva::USERAWFITNESS);
	}

	/***************************************************************************/
	/**
	 * Allows derived classes to reset the stall counter.
	 */
	void resetStallCounter() {
		stallCounter_ = 0;
	}

	/***************************************************************************/
	/**
	 * Allows to perform initialization work before the optimization cycle starts. This
	 * function will usually be overloaded by derived functions, which should however,
	 * as one of their first actions, call this function.
	 */
	virtual void init() BASE
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Allows to perform any remaining work after the optimization cycle has finished.
	 * This function will usually be overloaded by derived functions, which should however
	 * call this function as one of their last actions.
	 */
	virtual void finalize() BASE
	{ /* nothing */ }

	/***************************************************************************/
	/** @brief Retrieve a personality trait object belong to this algorithm */
	virtual std::shared_ptr<GPersonalityTraits> getPersonalityTraits() const = 0;

	/***************************************************************************/
	/** @brief Resizes the population to the desired level and does some error checks */
	virtual void adjustPopulation() = 0;

	/***************************************************************************/
	/**
	 * Lets individuals know about the current iteration of the optimization
	 * cycle.
	 */
	virtual void markIteration() BASE {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setAssignedIteration(iteration_);
		}
	}

	/***************************************************************************/
	/**
	 * Updates the worst known valid evaluations up to the current iteration
	 * and stores the fitness-values internally. Note: The first tuple-value
	 * in the vector signifies the untransformed (but possible == MIN/MAX_DOUBLE)
	 * evaluation, the second value the potentially transformed value.
	 */
	void updateWorstKnownValid() {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		std::size_t nFitnessCriteria = (*(this->begin()))->getNumberOfFitnessCriteria();

		// Is this the first call ? Fill worstKnownValids_ with data
		if(inFirstIteration()) {
			for(it=this->begin(); it!=this->end(); ++it) {
				(*it)->populateWorstKnownValid();
			}

			// Initialize our own, local worstKnownValids_
			worstKnownValids_ = (*(this->begin()))->getWorstKnownValids();
		}

		for(it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
			if((*it)->getNumberOfFitnessCriteria() != nFitnessCriteria) {
				glogger
				<< "In GOptimizationAlgorithmT<>::updateWorstKnownValid(): Error!" << std::endl
				<< "Got " << (*it)->getNumberOfFitnessCriteria() << " fitness criteria in individual " << (it-this->begin()) << std::endl
				<< "but expected " << nFitnessCriteria << " criteria" << std::endl
				<< GEXCEPTION;
			}

			if(!worstKnownValids_.empty() && worstKnownValids_.size() != nFitnessCriteria) {
				glogger
				<< "In GOptimizationAlgorithmT<>::updateWorstKnownValid(): Error!" << std::endl
				<< "Got invalid number of evaluation criteria in worstKnownValids_:" << std::endl
				<< "Got " << worstKnownValids_.size() << " but expected " << nFitnessCriteria << std::endl
				<< GEXCEPTION;
			}
#endif /* DEBUG */

			if((*it)->isClean() && (*it)->isValid()) { // Is this an individual which has been evaluated and fulfills all constraints ?
				for(std::size_t id=0; id<nFitnessCriteria; id++) {
					(*it)->challengeWorstValidFitness(worstKnownValids_.at(id), id);
				}
			}
		}
	}

	/***************************************************************************/
	/**
	 * Let the individuals know about the worst known valid solution so far
	 */
	void markWorstKnownValid() {
		this->updateWorstKnownValid();
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setWorstKnownValid(worstKnownValids_);
		}
	}

	/***************************************************************************/
	/**
	 * Triggers an update of the individual's evaluation (e.g. in order to
	 * act on the information regarding best or worst evaluations found
	 */
	void triggerEvaluationUpdate() {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->postEvaluationUpdate();
		}
	}

	/***************************************************************************/
	/**
	 * Work to be performed right after the individuals were evaluated. NOTE:
	 * this setup is sub-optimal, as this function isn't called from within
	 * GOptimizationAlgorithmT directly, but only from derived classes. This happens
	 * to prevent an additional split of the cycleLogic function.
	 */
	void postEvaluationWork() {
		// Find the worst known valid solution in the current iteration and
		// propagate the knowledge to all individuals
		this->markWorstKnownValid();

		// Individuals may choose to update their fitness depending on
		// the information relayed in this function. Give them a chance
		// to do so.
		this->triggerEvaluationUpdate();
	}

	/***************************************************************************/
	/**
	 * Let individuals know the number of stalls encountered so far
	 */
	void markNStalls() {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setNStalls(stallCounter_);
		}
	}

	/***************************************************************************/
	/**
	 * Gives derived classes an opportunity to update their internal structures.
	 * NOTE that no action may be taken here that affects the "dirty" state
	 * of individuals. A typical usage scenario would be the update of the adaptor
	 * settings in evolutionary algorithms.
	 */
	virtual void actOnStalls() BASE
	{ /* nothing */ }

	/***************************************************************************/
	/** @brief Calculates the fitness of all required individuals; to be re-implemented in derived classes */
	virtual void runFitnessCalculation() override = 0;

	/***************************************************************************/

    std::uniform_real_distribution<double> m_uniform_real_distribution; ///< Access to uniformly distributed double random values

private:
	/***************************************************************************/
	/**
	 * Update the stall counter. We use the transformed fitness for comparison
	 * here, so we can usually deal with finite values (due to the transformation
	 * in the case of a constraint violation).
	 */
	void updateStallCounter(const std::tuple<double, double>& bestEval) {
		if(this->isBetter(std::get<G_TRANSFORMED_FITNESS>(bestEval), std::get<G_TRANSFORMED_FITNESS>(bestKnownPrimaryFitness_))) {
			bestKnownPrimaryFitness_ = bestEval;
			stallCounter_ = 0;
		} else {
			stallCounter_++;
		}
	}

	/***************************************************************************/
	/**
	 * This function returns true once a given time (set with
	 * GOptimizationAlgorithm<ind_type>::setMaxTime()) has passed.
	 * It is used in the GOptimizationAlgorithmT<ind_type>::halt() function.
	 *
	 * @return A boolean indicating whether a given amount of time has passed
	 */
	bool timedHalt() const {
		using namespace boost::posix_time;
		ptime currentTime = microsec_clock::universal_time();
		if((currentTime - startTime_) >= maxDuration_) {
			if(emitTerminationReason_) {
				glogger
				<< "Terminating optimization run because maximum time frame has been exceeded." << std::endl
				<< GLOGGING;
			}

			return true;
		} else {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * This function returns true once the quality is below or above a given threshold
	 * (depending on whether we maximize or minimize). This function uses user-visible
	 * (i.e. untransformed) fitness values, as a quality threshold will usually be
	 * set using a true "physical" value.
	 *
	 * @return A boolean indicating whether the quality is above or below a given threshold
	 */
	bool qualityHalt() const {
		if(this->isBetter(std::get<G_RAW_FITNESS>(bestKnownPrimaryFitness_), qualityThreshold_)) {
			if(emitTerminationReason_) {
				glogger
				<< "Terminating optimization run because" << std::endl
				<< "quality threshold " << qualityThreshold_ << " has been reached." << std::endl
				<< "Best untransformed quality found was " << std::get<G_RAW_FITNESS>(bestKnownPrimaryFitness_) << std::endl
				<< "with termination in iteration " << iteration_ << std::endl
				<< GLOGGING;
			}

			return true;
		} else {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * This function returns true once a given number of stalls has been exceeded in a row
	 *
	 * @return A boolean indicating whether the optimization has stalled too often in a row
	 */
	bool stallHalt() const {
		if(stallCounter_ > maxStallIteration_) {
			if(emitTerminationReason_) {
				std::cout
				<< "Terminating optimization run because" << std::endl
				<< "maximum number of stalls " << maxStallIteration_ << " has been exceeded." << std::endl
				<< "This is considered to be a criterion for convergence." << std::endl;
			}

			return true;
		} else {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * This function returns true once a maximum number of iterations has been exceeded
	 *
	 * @return A boolean indicating whether the maximum number of iterations has been exceeded
	 */
	bool iterationHalt() const {
		if(iteration_ >= (maxIteration_ + offset_)) {
			if(emitTerminationReason_) {
				std::cout
				<< "Terminating optimization run because" << std::endl
				<< "iteration threshold " << maxIteration_ << " has been reached." << std::endl;
			}

			return true;
		} else {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * This function returns true if a SIGHUP / CTRL_CLOSE_EVENT signal was sent (provided the user
	 * has registered the GObject::sigHupHandler signal handler
	 *
	 * @return A boolean indicating whether the program was interrupted with a SIGHUP or CTRL_CLOSE_EVENT signal
	 */
	bool sigHupHalt() const {
		if(GObject::G_SIGHUP_SENT()) {
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
			std::cout
			<< "Terminating optimization run because a CTRL_CLOSE_EVENT signal has been received" << std::endl;
#else
			std::cout
			<< "Terminating optimization run because a SIGHUP signal has been received" << std::endl;
#endif
			return true;
		}
		else return false;
	}

 	/***************************************************************************/
	/**
	 * Triggers termination of the optimization run, when a file with a user-defined
	 * name is modified (e.g. "touch'ed") after the optimization run was started. Note
	 * that the function will silently return false if the file does not exist, as it
	 * is assumed that users may "touch" the file for termination only, so that the
	 * possibility exists that the file isn't there until that time.
	 */
	bool touchHalt() const {
		namespace bf = boost::filesystem;

		// Create a suitable path object
		bf::path p(terminationFile_);

		// Return if the file doesn't exist
		if(!bf::exists(p)) {
			return false;
		}

		// Determine the modification time of the file
		std::time_t t = boost::filesystem::last_write_time(p);
		boost::posix_time::ptime modTime = boost::posix_time::from_time_t(t);
		// boost::posix_time::time_duration diff = modTime - startTime_;

		// Check if the file was modified after the start of the optimization run
		// if(diff.total_microseconds() > 0) {
		if(modTime > startTime_) {
			if(emitTerminationReason_) {
				std::cout
				<< "Terminating optimization run because" << std::endl
				<< p << " was modified after the start of the optimization" << std::endl;
			}

			return true;
		} else {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * A wrapper for the customHalt() function that allows us to emit the termination reason
	 *
	 * @return A boolean indicating whether a custom halt criterion has been reached
	 */
	bool customHalt_() const {
		if(customHalt()) {
			if(emitTerminationReason_) {
				glogger
				<< "Terminating optimization run because custom halt criterion has triggered." << std::endl
				<< GLOGGING;
			}

			return true;
		} else {
			return false;
		}

	}

	/***************************************************************************/
	/**
	 * This function checks whether a halt criterion has been reached. The most
	 * common criterion is the maximum number of iterations. Set the maxIteration_
	 * counter to 0 if you want to disable this criterion.
	 *
	 * @return A boolean indicating whether a halt criterion has been reached
	 */
	bool halt() const {
		// Have we exceeded the maximum number of iterations and
		// do we indeed intend to stop in this case ?
		if(maxIterationHaltset() && iterationHalt()) return true;

		// Has the optimization stalled too often ?
		if(stallHaltSet() && stallHalt()) return true;

		// Have we received a SIGHUP signal ?
		if(sigHupHalt()) return true;

		// Do we have a scheduled halt time ? The comparatively expensive
		// timedHalt() calculation is only called if maxDuration_
		// is at least one microsecond.
		if(maxDurationHaltSet() && timedHalt()) return true;

		// Are we supposed to stop when the quality has exceeded a threshold ?
		if(qualityThresholdHaltSet() && qualityHalt()) return true;

		// Are we supposed to stop when a file was modified after the start of the optimization run ?
		if(terminateOnFileModification_ && touchHalt()) return true;

		// Has the user specified an additional stop criterion ?
		if(customHalt_()) return true;

		// Fine, we can continue.
		return false;
	}

	/***************************************************************************/
	/**
	 * Check whether the max-iteration halt is set
	 *
	 * @return A boolean indicating whether the "max-iteration halt" has been set
	 */
	bool maxIterationHaltset() const {
		if(0 == maxIteration_) return false;
		else return true;
	}

	/***************************************************************************/
	/**
	 * Check whether a halt criterion based on the number of stalls has been set
	 *
	 * @return A boolean indicating whether a halt criterion based on the number of stalls has been set
	 */
	bool stallHaltSet() const {
		if(0 == maxStallIteration_) return false;
		else return true;
	}

	/***************************************************************************/
	/**
	 * Check whether the maxDuration-halt criterion has been set
	 *
	 * @return A boolean indication whether the max-duration halt criterion has been set
	 */
	bool maxDurationHaltSet() const {
		if(0 == maxDuration_.total_microseconds()) return false;
		else return true;
	}

	/***************************************************************************/
	/**
	 * Check whether the quality-threshold halt-criterion has been set
	 *
	 * @return A boolean indicating whether the quality-threshold halt-criterion has been set
	 */
	bool qualityThresholdHaltSet() const {
		return hasQualityThreshold_;
	}

	/***************************************************************************/
	/**
	 * Marks the globally best known fitness in all individuals
	 */
	void markBestFitness() {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setBestKnownPrimaryFitness(this->getBestKnownPrimaryFitness());
		}
	}

	/***************************************************************************/
	/**
	 * Indicates whether the stallCounterThreshold_ has been exceeded
	 */
	bool stallCounterThresholdExceeded() const {
		return (stallCounter_ > stallCounterThreshold_);
	}

	/***************************************************************************/

	std::uint32_t iteration_ = 0; ///< The current iteration
	std::uint32_t offset_ = DEFAULTOFFSET; ///< An iteration offset which can be used, if the optimization starts from a checkpoint file
	std::uint32_t maxIteration_ = DEFAULTMAXIT; ///< The maximum number of iterations
	std::uint32_t maxStallIteration_ = DEFAULTMAXSTALLIT; ///< The maximum number of generations without improvement, after which optimization is stopped
	std::uint32_t reportIteration_ = DEFAULTREPORTITER; ///< The number of generations after which a report should be issued

	std::size_t nRecordbestGlobalIndividuals_ = DEFNRECORDBESTINDIVIDUALS; ///< Indicates the number of best individuals to be recorded/updated in each iteration
	GParameterSetFixedSizePriorityQueue bestGlobalIndividuals_; ///< A priority queue with the best individuals found so far
	GParameterSetFixedSizePriorityQueue bestIterationIndividuals_; ///< A priority queue with the best individuals of a given iteration

	std::size_t defaultPopulationSize_ = DEFAULTPOPULATIONSIZE; ///< The nominal size of the population
	std::tuple<double, double> bestKnownPrimaryFitness_; ///< Records the best primary fitness found so far
	std::tuple<double, double> bestCurrentPrimaryFitness_; ///< Records the best fitness found in the current iteration

	std::uint32_t stallCounter_ = 0; ///< Counts the number of iterations without improvement
	std::uint32_t stallCounterThreshold_ = DEFAULTSTALLCOUNTERTHRESHOLD; ///< The number of stalls after which individuals are asked to update their internal data structures

	std::int32_t cpInterval_ = DEFAULTCHECKPOINTIT; ///< Number of iterations after which a checkpoint should be written. -1 means: Write whenever an improvement was encountered
	std::string cpBaseName_ = DEFAULTCPBASENAME; ///< The base name of the checkpoint file
	std::string cpDirectory_ = DEFAULTCPDIR; ///< The directory where checkpoint files should be stored
	Gem::Common::serializationMode cpSerMode_ = DEFAULTCPSERMODE; ///< Determines whether check-pointing should be done in text-, XML, or binary mode
	double qualityThreshold_ = DEFAULTQUALITYTHRESHOLD; ///< A threshold beyond which optimization is expected to stop
	bool hasQualityThreshold_ = false; ///< Specifies whether a qualityThreshold has been set
	boost::posix_time::time_duration maxDuration_ = boost::posix_time::duration_from_string(DEFAULTDURATION); ///< Maximum time frame for the optimization
	mutable boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
	std::string terminationFile_ = DEFAULTTERMINATIONFILE; ///< The name of a file which, when modified after the start of the optimization run, will cause termination of the run
	bool terminateOnFileModification_ = false;
	bool emitTerminationReason_ = DEFAULTEMITTERMINATIONREASON; ///< Specifies whether information about reasons for termination should be emitted
	bool halted_ = false; ///< Set to true when halt() has returned "true"
	std::vector<std::tuple<double, double>> worstKnownValids_; ///< Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: tranformed)
	std::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT> optimizationMonitor_ptr_;
	std::shared_ptr<typename Gem::Geneva::GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT> default_pluggable_monitor_; ///< A default monitor
	std::vector<std::shared_ptr<typename Gem::Geneva::GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>> pluggable_monitors_; ///< A collection of monitors

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		bool result = false;

		// Call the parent class'es function
		if(GMutableSetT<ind_type>::modify_GUnitTests()) result = true;

		this->setMaxIteration(this->getMaxIteration() + 1);
		result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GOptimizationAlgorithmT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent class'es function
		GMutableSetT<ind_type>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GOptimizationAlgorithmT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent class'es function
		GMutableSetT<ind_type>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		condnotset("GOptimizationAlgorithmT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/


public:
	/******************************************************************************/
	////////////////////////////////////////////////////////////////////////////////
	/******************************************************************************/
	/**
	 * The base class of all pluggable optimization monitors
	 */
	class GBasePluggableOMT : public GObject
	{
		///////////////////////////////////////////////////////////////////////
		friend class boost::serialization::access;

		template<typename Archive>
		void serialize(Archive & ar, const unsigned int){
			using boost::serialization::make_nvp;

			ar
			& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
			& BOOST_SERIALIZATION_NVP(useRawEvaluation_);
		}
		///////////////////////////////////////////////////////////////////////

	public:
		/***************************************************************************/
		/**
		 * The default constructor. Some member variables may be initialized in the class body.
		 */
		GBasePluggableOMT()
		{ /* nothing */ }

		/***************************************************************************/
		/**
		 * The copy constructor
		 */
		GBasePluggableOMT(
			const typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT& cp
		)
			: useRawEvaluation_(cp.useRawEvaluation_)
		{ /* nothing */ }

		/***************************************************************************/
		/**
		 * The Destructor
		 */
		virtual ~GBasePluggableOMT()
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * Checks for equality with another GBasePluggableOMT object
		 *
		 * @param  cp A constant reference to another GBasePluggableOMT object
		 * @return A boolean indicating whether both objects are equal
		 */
		virtual bool operator==(const typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT& cp) const {
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
		 * Checks for inequality with another GBasePluggableOMT object
		 *
		 * @param  cp A constant reference to another GBasePluggableOMT object
		 * @return A boolean indicating whether both objects are inequal
		 */
		virtual bool operator!=(const typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT& cp) const {
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

			// Check that we are dealing with a GBasePluggableOMT reference independent of this object and convert the pointer
			const GBasePluggableOMT *p_load = Gem::Common::g_convert_and_compare<GObject, GBasePluggableOMT >(cp, this);

			GToken token("GBasePluggableOMT", e);

			// Compare our parent data ...
			Gem::Common::compare_base<GObject>(IDENTITY(*this, *p_load), token);

			// ... and then our local data
			compare_t(IDENTITY(useRawEvaluation_, p_load->useRawEvaluation_), token);

			// React on deviations from the expectation
			token.evaluate();
		}

		/***************************************************************************/
		/**
		 * Overload this function in derived classes, specifying actions for
		 * initialization, the optimization cycles and finalization.
		 */
		virtual void informationFunction(
			const infoMode& im
			, GOptimizationAlgorithmT<ind_type> * const goa
		) BASE = 0;

		/***************************************************************************/
		/**
		 * Allows to set the useRawEvaluation_ variable
		 */
		void setUseRawEvaluation(bool useRaw) {
			useRawEvaluation_ = useRaw;
		}

		/***************************************************************************/
		/**
		 * Allows to retrieve the value of the useRawEvaluation_ variable
		 */
		bool getUseRawEvaluation() const {
			return useRawEvaluation_;
		}

	protected:
		/************************************************************************/
		/**
		 * Loads the data of another object
		 *
		 * cp A pointer to another GBasePluggableOMT object, camouflaged as a GObject
		 */
		virtual void load_(const GObject* cp) override {
			// Check that we are dealing with a GBasePluggableOMT reference independent of this object and convert the pointer
			const GBasePluggableOMT *p_load = Gem::Common::g_convert_and_compare<GObject, GBasePluggableOMT >(cp, this);

			// Load the parent classes' data ...
			GObject::load_(cp);

			// ... and then our local data
			useRawEvaluation_ = p_load->useRawEvaluation_;
		}

		/************************************************************************/
		/** @brief Creates a deep clone of this object */
		virtual GObject* clone_() const override = 0;

		/***************************************************************************/
		bool useRawEvaluation_ = false; ///< Specifies whether the true (unmodified) evaluation should be used

	public:
		/************************************************************************/
		/**
		 * Applies modifications to this object. This is needed for testing purposes
		 */
		virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
			bool result = false;

			// Call the parent class'es function
			if(GObject::modify_GUnitTests()) result = true;

			if(true == this->getUseRawEvaluation()) {
				this->setUseRawEvaluation(false);
			} else {
				this->setUseRawEvaluation(true);
			}
			result = true;

			return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
			condnotset("GOptimizationAlgorithmT<>::GBasePluggableOMT::modify_GUnitTests", "GEM_TESTING");
			return false;
#endif /* GEM_TESTING */
		}

		/************************************************************************/
		/**
		 * Performs self tests that are expected to succeed. This is needed for testing purposes
		 */
		virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
			// Call the parent class'es function
			GObject::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
			condnotset("GOptimizationAlgorithmT<>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
		}

		/************************************************************************/
		/**
		 * Performs self tests that are expected to fail. This is needed for testing purposes
		 */
		virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
			// Call the parent class'es function
			GObject::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
			condnotset("GOptimizationAlgorithmT<>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
		}

		/************************************************************************/
	};

	/***************************************************************************/
	/////////////////////////////////////////////////////////////////////////////
	/***************************************************************************/
	/**
	 * This nested class defines the interface of optimization monitors, as used
	 * in the Geneva library. It also provides users with some basic information.
	 */
	class GOptimizationMonitorT : public GObject
	{
		///////////////////////////////////////////////////////////////////////
		friend class boost::serialization::access;

		template<typename Archive>
		void serialize(Archive & ar, const unsigned int){
			using boost::serialization::make_nvp;

			ar
			& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
			& BOOST_SERIALIZATION_NVP(quiet_);
		}
		///////////////////////////////////////////////////////////////////////

	public:
		/************************************************************************/
		/**
		 * The default constructor
		 */
		GOptimizationMonitorT()
			: GObject()
			, quiet_(false)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * The copy constructor
		 *
		 * @param cp A copy of another GOptimizationMonitorT object
		 */
		GOptimizationMonitorT(
			const typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT& cp
		)
			: GObject(cp)
			, quiet_(cp.quiet_)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * The destructor
		 */
		virtual ~GOptimizationMonitorT()
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * Checks for equality with another GOptimizationMonitorT object
		 *
		 * @param  cp A constant reference to another GOptimizationMonitorT object
		 * @return A boolean indicating whether both objects are equal
		 */
		virtual bool operator==(const typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT& cp) const {
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
		 * Checks for inequality with another GOptimizationMonitorT object
		 *
		 * @param  cp A constant reference to another GOptimizationMonitorT object
		 * @return A boolean indicating whether both objects are inequal
		 */
		virtual bool operator!=(const typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT& cp) const {
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

			// Check that we are dealing with a GOptimizationMonitorT reference independent of this object and convert the pointer
			const GOptimizationMonitorT *p_load = Gem::Common::g_convert_and_compare<GObject, GOptimizationMonitorT >(cp, this);

			GToken token("GOptimizationMonitorT", e);

			// Compare our parent data ...
			Gem::Common::compare_base<GObject>(IDENTITY(*this, *p_load), token);

			// ... and then our local data
			compare_t(IDENTITY(quiet_, p_load->quiet_), token);

			// React on deviations from the expectation
			token.evaluate();
		}

		/************************************************************************/
		/**
		 * The actual information function. It is up to the user to define what happens
		 * in each step. This function only enforces the emission of simple progress
		 * information to the command line in each iteration (unless the "quiet_" variable
		 * has been set.
		 *
		 * @param im The mode in which the information function is called
		 * @param goa A pointer to the current optimization algorithm for which information should be emitted
		 */
		void informationFunction(
			const infoMode& im
			, GOptimizationAlgorithmT<ind_type> * const goa
		) {
			// Perform any action defined by the user through pluggable monitor objects
			for(auto pm_ptr: pluggable_monitors_) { // std::shared_ptr may be copied
				pm_ptr->informationFunction(im,goa);
			}

			// Act on the information mode provided
			switch(im) {
				case Gem::Geneva::infoMode::INFOINIT:
				{
					if(!quiet_) {
						std::cout << "Starting an optimization run with algorithm \"" << goa->getAlgorithmName() << "\"" << std::endl;
					}
					this->firstInformation(goa);
				}
					break;

				case Gem::Geneva::infoMode::INFOPROCESSING:
				{
					// We output raw values here, as this is likely what the user is interested in
					if(!quiet_) {
						std::cout
						<< std::setprecision(5)
						<< goa->getIteration() << ": "
						<< Gem::Common::g_to_string(goa->getBestCurrentPrimaryFitness())
						<< " // best past: " << Gem::Common::g_to_string(goa->getBestKnownPrimaryFitness())
						<< std::endl;
					}
					this->cycleInformation(goa);
				}
					break;

				case Gem::Geneva::infoMode::INFOEND:
				{
					this->lastInformation(goa);
					if(!quiet_) std::cout << "End of optimization reached in algorithm \""<< goa->getAlgorithmName() << "\"" << std::endl;
				}
					break;

				default:
				{
					glogger
					<< "Received invalid infoMode " << im << std::endl
					<< GEXCEPTION;
				}
					break;
			};
		}

		/************************************************************************/
		/**
		 * Prevents any information from being emitted by this object
		 */
		void preventInformationEmission() {
			quiet_ = true;
		}

		/************************************************************************/
		/**
		 * Allows this object to emit information
		 */
		void allowInformationEmission() {
			quiet_ = false;
		}

		/************************************************************************/
		/**
		 * Allows to check whether the emission of information is prevented
		 *
		 * @return A boolean which indicates whether information emission is prevented
		 */
		bool informationEmissionPrevented() const {
			return quiet_;
		}

		/************************************************************************/
		/**
		 * Allows to register a pluggable optimization monitor
		 */
		void registerPluggableOM(
			std::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT> pluggableOM
		) {
			if(pluggableOM) {
				pluggable_monitors_.push_back(pluggableOM);
			} else {
				glogger
				<< "In GoptimizationMonitorT<>::registerPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
				<< GEXCEPTION;
			}
		}

		/************************************************************************/
		/**
		 * Allows to reset the local pluggable optimization monitors
		 */
		void resetPluggableOM() {
			pluggable_monitors_.clear();
		}

		/******************************************************************************/
		/**
		 * Allows to check whether pluggable optimization monitors were registered
		 */
		bool hasOptimizationMonitors() const {
			return !pluggable_monitors_.empty();
		}

	protected:
		/************************************************************************/
		/**
		 * A function that is called once before the optimization starts
		 *
		 * @param goa A pointer to the current optimization algorithm for which information should be emitted
		 */
		virtual void firstInformation(GOptimizationAlgorithmT<ind_type> * const goa) BASE
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * A function that is called during each optimization cycle. It is possible to
		 * extract quite comprehensive information in each iteration. Have a look at
		 * the examples accompanying Geneva for further information.
		 *
		 * @param goa A pointer to the current optimization algorithm for which information should be emitted
		 */
		virtual void cycleInformation(GOptimizationAlgorithmT<ind_type> * const goa) BASE
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * A function that is called once at the end of the optimization cycle
		 *
		 * @param goa A pointer to the current optimization algorithm for which information should be emitted
		 */
		virtual void lastInformation(GOptimizationAlgorithmT<ind_type> * const goa) BASE
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * Loads the data of another object
		 *
		 * cp A pointer to another GOptimizationMonitorT object, camouflaged as a GObject
		 */
		virtual void load_(const GObject* cp) override {
			// Check that we are dealing with a GOptimizationMonitorT reference independent of this object and convert the pointer
			const GOptimizationMonitorT *p_load = Gem::Common::g_convert_and_compare<GObject, GOptimizationMonitorT >(cp, this);

			// Load the parent classes' data ...
			GObject::load_(cp);

			// ... and then our local data
			quiet_ = p_load->quiet_;

			// Note: we do not load the pluggable information function, as it is
			// meant as a short-term medium for information retrieval and may be
			// an object specific to a given optimization monitor object
		}

		/************************************************************************/
		/**
		 * Creates a deep clone of this object
		 */
		virtual GObject* clone_() const override {
			return new typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT(*this);
		}

	private:
		/************************************************************************/

		bool quiet_; ///< Specifies whether any information should be emitted at all

		std::vector<std::shared_ptr<typename Gem::Geneva::GOptimizationAlgorithmT<ind_type>::GBasePluggableOMT>> pluggable_monitors_; ///< A collection of monitors

	public:
		/************************************************************************/
		/**
		 * Applies modifications to this object. This is needed for testing purposes
		 */
		virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
			bool result = false;

			// Call the parent class'es function
			if(GObject::modify_GUnitTests()) result = true;

			if(this->informationEmissionPrevented()) {
				this->allowInformationEmission();
			} else {
				this->preventInformationEmission();
			}
			result = true;

			return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
			condnotset("GOptimizationAlgorithmT<>::GOptimizationMonitorT::modify_GUnitTests", "GEM_TESTING");
			return false;
#endif /* GEM_TESTING */
		}

		/************************************************************************/
		/**
		 * Performs self tests that are expected to succeed. This is needed for testing purposes
		 */
		virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
			// Call the parent class'es function
			GObject::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
			condnotset("GOptimizationAlgorithmT<>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
		}

		/************************************************************************/
		/**
		 * Performs self tests that are expected to fail. This is needed for testing purposes
		 */
		virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
			// Call the parent class'es function
			GObject::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
			condnotset("GOptimizationAlgorithmT<>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
		}

		/************************************************************************/
	};
	/***************************************************************************/
	/////////////////////////////////////////////////////////////////////////////
	/***************************************************************************/
};

/******************************************************************************/
/**
 * Allows to perform initialization work before the optimization cycle starts. This
 * function will usually be overloaded by derived functions, which should however,
 * as one of their first actions, call this function. It is not recommended  to perform
 * any "real" optimization work here, such as evaluation of individuals. Use the
 * optimizationInit() function instead.
 */
template <>
inline void GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::init()
{ /* nothong */ }

/******************************************************************************/
/**
 * Adds the individuals of this iteration to a priority queue. The
 * queue will be sorted by the first evaluation criterion of the individuals
 * and may either have a limited or unlimited size, depending on user-
 * settings
 */
template <>
inline void GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::updateGlobalBestsPQ(
	GParameterSetFixedSizePriorityQueue& bestIndividuals
) BASE {
	const bool CLONE = true;
	const bool DONOTREPLACE = false;

#ifdef DEBUG
	if(this->empty()) {
		glogger
		<< "In GBaseParChildT<GParameterSet>::updateGlobalBestsPQ() :" << std::endl
		<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
		<< GEXCEPTION;
	}
#endif /* DEBUG */

	// We simply add all individuals to the queue -- only the best ones will actually be added (and cloned)
	// Unless we have asked for the queue to have an unlimited size, the queue will be resized as required
	// by its maximum allowed size.
	bestIndividuals.add(this->data, CLONE, DONOTREPLACE);
}

/******************************************************************************/
/**
 * Adds the individuals of this iteration to a priority queue. The
 * queue will be sorted by the first evaluation criterion of the individuals
 * and may either have a limited or unlimited size, depending on user-
 * settings
 */
template <>
inline void GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::updateIterationBestsPQ(
	GParameterSetFixedSizePriorityQueue& bestIndividuals
) BASE {
	const bool CLONE = true;
	const bool REPLACE = true;

#ifdef DEBUG
	if(this->empty()) {
		glogger
		<< "In GBaseParChildT<GParameterSet>::updateIterationBestsPQ() :" << std::endl
		<< "Tried to retrieve the best individuals even though the population is empty." << std::endl
		<< GEXCEPTION;
	}
#endif /* DEBUG */

	// We simply add all individuals to the queue. They will automatically be sorted.
	bestIndividuals.add(this->data, CLONE, REPLACE);
}

/******************************************************************************/
/**
 * If individuals have been stored in this population, they are added to the
 * priority queue. This happens before the optimization cycle starts, so that
 * best individuals from a previous "chained" optimization run aren't lost.
 * Only those individuals are stored in the priority queue that do not have the
 * "dirty flag" set.
 */
template <>
inline void GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::addCleanStoredBests(
	GParameterSetFixedSizePriorityQueue& bestIndividuals
) {
	const bool CLONE = true;

	// We simply add all *clean* individuals to the queue -- only the best ones will actually be added
	// (and cloned) Unless we have asked for the queue to have an unlimited size, the queue will be
	// resized as required by its maximum allowed size.
	GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::iterator it;
	for(it=this->begin(); it!=this->end(); ++it) {
		if((*it)->isClean()) {
			bestIndividuals.add(*it, CLONE);
		}
	}
}

/******************************************************************************/
/**
 * Allows to perform any remaining work after the optimization cycle has finished.
 * This function will usually be overloaded by derived functions, which should however
 * call this function as one of their last actions. It is not recommended  to perform
 * any "real" optimization work here, such as evaluation of individuals. Use the
 * optimizationFinalize() function instead.
 */
template <>
inline void GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::finalize()
{ /* nothing */ }

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {
template<typename ind_type>
struct is_abstract<Gem::Geneva::GOptimizationAlgorithmT<ind_type>> : public boost::true_type {};
template<typename ind_type>
struct is_abstract< const Gem::Geneva::GOptimizationAlgorithmT<ind_type>> : public boost::true_type {};
}
}

/******************************************************************************/

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GOptimizableEntity>::GOptimizationMonitorT)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::GOptimizationMonitorT)
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GBrokerConnector2T<Gem::Geneva::GParameterSet>)

/******************************************************************************/

#endif /* GOPTIMIZATIONALGORITHMT_HPP_ */
