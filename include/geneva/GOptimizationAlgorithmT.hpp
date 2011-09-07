/**
 * @file GOptimizationAlgorithmT.hpp
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

// Boost header files go here

#ifndef GOPTIMIZATIONALGORITHMT_HPP_
#define GOPTIMIZATIONALGORITHMT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

/**************************************************************************************/

// Geneva headers go here
#include "courtier/GBrokerConnectorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GOptimizableI.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GParameterSet.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************************/
/**
 * This class implements basic operations found in iteration-based optimization algorithms.
 * E.g., one might want to stop the optimization after a given number of cycles, or after
 * a given amount of time. The class also defines the interface functions common to these
 * algorithms, such as a general call to "optimize()".
 */
template <typename ind_type = Gem::Geneva::GIndividual>
class GOptimizationAlgorithmT
	: public GMutableSetT<ind_type>
	, public GOptimizableI
{
public:
	// Forward declaration, as this class is only defined at the end of this file
	class GOptimizationMonitorT;

private:
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GMutableSetT", boost::serialization::base_object<GMutableSetT<ind_type> >(*this))
	     & make_nvp("GOptimizableI", boost::serialization::base_object<GOptimizableI>(*this))
	     & BOOST_SERIALIZATION_NVP(iteration_)
	     & BOOST_SERIALIZATION_NVP(offset_)
	     & BOOST_SERIALIZATION_NVP(maxIteration_)
	     & BOOST_SERIALIZATION_NVP(maxStallIteration_)
	     & BOOST_SERIALIZATION_NVP(reportIteration_)
	     & BOOST_SERIALIZATION_NVP(defaultPopulationSize_)
	     & BOOST_SERIALIZATION_NVP(bestPastFitness_)
	     & BOOST_SERIALIZATION_NVP(bestCurrentFitness_)
	     & BOOST_SERIALIZATION_NVP(stallCounter_)
  	     & BOOST_SERIALIZATION_NVP(cpInterval_)
	     & BOOST_SERIALIZATION_NVP(cpBaseName_)
	     & BOOST_SERIALIZATION_NVP(cpDirectory_)
	     & BOOST_SERIALIZATION_NVP(cpSerMode_)
	     & BOOST_SERIALIZATION_NVP(qualityThreshold_)
	     & BOOST_SERIALIZATION_NVP(hasQualityThreshold_)
	     & BOOST_SERIALIZATION_NVP(maxDuration_)
	     & BOOST_SERIALIZATION_NVP(emitTerminationReason_)
	     & BOOST_SERIALIZATION_NVP(halted_)
	     & BOOST_SERIALIZATION_NVP(optimizationMonitor_ptr_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/**************************************************************************************/
	/**
	 * The default constructor
	 */
	GOptimizationAlgorithmT()
		: GMutableSetT<ind_type>()
		, iteration_(0)
		, offset_(DEFAULTOFFSET)
		, maxIteration_(DEFAULTMAXIT)
		, maxStallIteration_(DEFAULTMAXSTALLIT)
		, reportIteration_(DEFAULTREPORTITER)
		, defaultPopulationSize_(0)
		, bestPastFitness_(0.) // will be set appropriately in the optimize() function
		, bestCurrentFitness_(0.) // will be set appropriately in the optimize() function
		, stallCounter_(0)
		, cpInterval_(DEFAULTCHECKPOINTIT)
		, cpBaseName_(DEFAULTCPBASENAME)
		, cpDirectory_(DEFAULTCPDIR)
		, cpSerMode_(DEFAULTCPSERMODE)
		, qualityThreshold_(DEFAULTQUALITYTHRESHOLD)
		, hasQualityThreshold_(false)
		, maxDuration_(boost::posix_time::duration_from_string(DEFAULTDURATION))
		, emitTerminationReason_(false)
		, halted_(false)
		, optimizationMonitor_ptr_(new typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT())
	{ /* nothing */ }

	/**************************************************************************************/
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
		, defaultPopulationSize_(cp.defaultPopulationSize_)
		, bestPastFitness_(cp.bestPastFitness_)
		, bestCurrentFitness_(cp.bestCurrentFitness_)
		, stallCounter_(cp.stallCounter_)
		, cpInterval_(cp.cpInterval_)
		, cpBaseName_(cp.cpBaseName_)
		, cpDirectory_(cp.cpDirectory_)
		, cpSerMode_(cp.cpSerMode_)
		, qualityThreshold_(cp.qualityThreshold_)
		, hasQualityThreshold_(cp.hasQualityThreshold_)
		, maxDuration_(cp.maxDuration_)
		, emitTerminationReason_(cp.emitTerminationReason_)
		, halted_(cp.halted_)
		, optimizationMonitor_ptr_((cp.optimizationMonitor_ptr_)->GObject::clone<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT>())
	{ /* nothing */ }

	/**************************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GOptimizationAlgorithmT()
	{ /* nothing */ }

	/**************************************************************************************/
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

	/**************************************************************************************/
	/** @brief Loads the state of the class from disc. */
	virtual void loadCheckpoint(const std::string&) = 0;

	/**************************************************************************************/
	/**
	 * Checks whether the optimization process has been halted, because the halt() function
	 * has returned "true"
	 *
	 * @return A boolean indicating whether the optimization process has been halted
	 */
	bool halted() const {
		return halted_;
	}

	/**************************************************************************************/
	/**
	 * Checks whether a better solution was found and updates the stallCounter_ variable
	 * as necessary.
	 *
	 * @param bestEval The best known evaluation of the current iteration
	 * @return A boolean indicating whether a better solution was found
	 */
	bool ifProgress(const double& bestEval) {
		// Check whether an improvement has been achieved
		bool better = isBetter(bestEval, bestPastFitness_);
		if(better) {
			bestPastFitness_ = bestEval;
			stallCounter_ = 0;
		}
		else {
			stallCounter_++;
		}

		return better;
	}

	/**************************************************************************************/
	/**
	 * Allows to set the number of generations after which a checkpoint should be written.
	 * A negative value will result in automatic checkpointing, whenever a better solution
	 * was found.
	 *
	 * @param cpInterval The number of generations after which a checkpoint should be written
	 */
	void setCheckpointInterval(boost::int32_t cpInterval) {
		cpInterval_ = cpInterval;
	}

	/**************************************************************************************/
	/**
	 * Allows to retrieve the number of generations after which a checkpoint should be written
	 *
	 * @return The number of generations after which a checkpoint should be written
	 */
	boost::uint32_t getCheckpointInterval() const {
		return cpInterval_;
	}

	/**************************************************************************************/
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
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
					<< "Error: Invalid cpBaseName: " << cpBaseName
			);
		}

		if(cpDirectory == "empty" || cpDirectory.empty()) {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
					<< "Error: Invalid cpDirectory: " << cpDirectory << std::endl
			);
		}

		cpBaseName_ = cpBaseName;

		// Check that the provided directory exists
		if(!boost::filesystem::exists(cpDirectory)) {
			std::cerr
				<< "In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(): Warning!" << std::endl
				<< "Directory " << cpDirectory << " does not exist and will be created automatically." << std::endl;

			if(!boost::filesystem::create_directory(cpDirectory)) {
				raiseException(
					"In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(): Error!" << std::endl
					<< "Could not create directory " << cpDirectory << std::endl
				);
			}
		} else if(!boost::filesystem::is_directory(cpDirectory)) {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName(): Error!" << std::endl
					<< cpDirectory << " exists but is no directory." << std::endl
			);
		}

		// Add a trailing slash to the directory name, if necessary
		// TODO: THIS IS NOT PORTABLE TO WINDOWS!
	    if(cpDirectory[cpDirectory.size() - 1] != '/') cpDirectory_ = cpDirectory + '/';
	    else cpDirectory_ = cpDirectory;
	}

	/**************************************************************************************/
	/**
	 * Allows to retrieve the base name of the checkpoint file.
	 *
	 * @return The base name used for checkpoint files
	 */
	std::string getCheckpointBaseName() const {
		return cpBaseName_;
	}

	/**************************************************************************************/
	/**
	 * Allows to retrieve the directory where checkpoint files should be stored
	 *
	 * @return The base name used for checkpoint files
	 */
	std::string getCheckpointDirectory() const {
		return cpDirectory_;
	}

	/**************************************************************************************/
	/**
	 * Determines whether checkpointing should be done in Text-, XML- or Binary-mode
	 *
	 * @param cpSerMode The desired new checkpointing serialization mode
	 */
	void setCheckpointSerializationMode(Gem::Common::serializationMode cpSerMode) {
		cpSerMode_ = cpSerMode;
	}

	/**************************************************************************************/
	/**
	 * Retrieves the current checkpointing serialization mode
	 *
	 * @return The current checkpointing serialization mode
	 */
	Gem::Common::serializationMode getCheckpointSerializationMode() const {
		return cpSerMode_;
	}

	/**************************************************************************************/
	/**
	 * Checks for equality with another GOptimizationAlgorithmT object
	 *
	 * @param  cp A constant reference to another GOptimizationAlgorithmT object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GOptimizationAlgorithmT<ind_type>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOptimizationAlgorithm<ind_type>::operator==","cp", CE_SILENT);
	}

	/**************************************************************************************/
	/**
	 * Checks for inequality with another GOptimizationAlgorithmT object
	 *
	 * @param  cp A constant reference to another GOptimizationAlgorithmT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GOptimizationAlgorithmT<ind_type>& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOptimizationAlgorithmT<ind_type>::operator!=","cp", CE_SILENT);
	}

	/**************************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GOptimizationAlgorithmT<ind_type> *p_load = GObject::gobject_conversion<GOptimizationAlgorithmT<ind_type> >(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
	    deviations.push_back(GMutableSetT<ind_type>::checkRelationshipWith(cp, e, limit, caller, y_name, withMessages));

		// ... and then our local data
		EXPECTATIONCHECK(iteration_);
		EXPECTATIONCHECK(offset_);
		EXPECTATIONCHECK(maxIteration_);
		EXPECTATIONCHECK(maxStallIteration_);
		EXPECTATIONCHECK(reportIteration_);
		EXPECTATIONCHECK(defaultPopulationSize_);
		EXPECTATIONCHECK(bestPastFitness_);
		EXPECTATIONCHECK(bestCurrentFitness_);
		EXPECTATIONCHECK(stallCounter_);
		EXPECTATIONCHECK(cpInterval_);
		EXPECTATIONCHECK(cpBaseName_);
		EXPECTATIONCHECK(cpDirectory_);
		EXPECTATIONCHECK(cpSerMode_);
		EXPECTATIONCHECK(qualityThreshold_);
		EXPECTATIONCHECK(hasQualityThreshold_);
		EXPECTATIONCHECK(maxDuration_);
		EXPECTATIONCHECK(emitTerminationReason_);
		EXPECTATIONCHECK(halted_);
		EXPECTATIONCHECK(optimizationMonitor_ptr_);

		return evaluateDiscrepancies("GOptimizationAlgorithmT<ind_type>", caller, deviations, e);
	}

	/**************************************************************************************/
	/**
	 * This function encapsulates some common functionality of iteration-based
	 * optimization algorithms. E.g., they all need a loop that stops if some
	 * predefined criterion is reached. This function is also the main entry
	 * point for all optimization algorithms.
	 *
	 * @param offset Specifies the iteration number to start with (e.g. useful when starting from a checkpoint file)
	 */
	virtual void optimize(const boost::uint32_t& offset) {
		// Check that we are dealing with an "authorized" optimization algorithm
		if(this->getOptimizationAlgorithm() == PERSONALITY_NONE) {
			raiseException(
					"In GOptimizationAlgorithmT<T>::optimize():" << std::endl
					<< "The id of the optimization algorithm hasn't been set."
			);
		}

		// Reset the generation counter
		iteration_ = offset;

		// Set the iteration offset
		offset_ = offset;

		// Let the audience know that the optimization process hasn't been halted yet
		halted_ = false;

		// Resize the population to the desired size and do some error checks
		adjustPopulation();

		// Set the individual's personalities (some algorithm-specific information needs to be stored
		// in individuals. Optimization algorithms need to re-implement this function to add
		// the required functionality.)
		setIndividualPersonalities();

		// Check the maximization mode of all individuals and set our own mode accordingly.
		setLocalMaxMode();

		// Emit the info header, unless we do not want any info (parameter 0).
		// Note that this call needs to come after the initialization, so we have the
		// complete set of individuals available.
		if(reportIteration_) doInfo(INFOINIT);

		// We want to know if no better values were found for a longer period of time
		bestPastFitness_ = getWorstCase();
		bestCurrentFitness_ = getWorstCase();
		stallCounter_ = 0;

		// Give derived classes the opportunity to perform any necessary preparatory work.
		init();

		// Initialize the start time with the current time.
		startTime_ = boost::posix_time::second_clock::local_time(); /// Hmmm - not necessarily thread-safe, if each population runs in its own thread ...

		// Perform any initial optimization work necessary (usually evaluation of individuals)
		optimizationInit();

		do {
			// Let all individuals know the current iteration
			markIteration();

			// Check whether a better value was found, and do the check-pointing, if necessary.
			// Uses the output of the function that contains the actual business logic of a
			// given optimization algorithm.
			checkpoint(ifProgress(bestCurrentFitness_ = cycleLogic()));

			// Let all individuals know about the best fitness known so far
			markBestFitness();

			// Let all individuals know about the number of failed optimization attempts in a row
			markNStalls();

			// We want to provide feedback to the user in regular intervals.
			// Set the reportGeneration_ variable to 0 in order not to emit
			// any information at all.
			if(reportIteration_ && (iteration_%reportIteration_ == 0)) doInfo(INFOPROCESSING);

			// update the iteration_ counter
			iteration_++;
		}
		while(!(halted_ = halt()));

		// Perform any remaining optimization work (usually evaluation of individuals)
		optimizationFinalize();

		// Give derived classes the opportunity to perform any remaining clean-up work
		finalize();

		// Finalize the info output
		if(reportIteration_) doInfo(INFOEND);

		// Remove information particular to evolutionary algorithms from the individuals
		resetIndividualPersonalities();
	}

	/**************************************************************************************/
	/**
	 * A little convenience function that helps to avoid having to specify explicit scopes
	 */
	virtual void optimize() {
		GOptimizableI::optimize();
	}

	/**************************************************************************************/
	/**
	 * Emits information specific to this class. The function can be overloaded
	 * in derived classes and it indeed makes sense to emit much more information
	 * than is done in this simple implementation.
	 *
	 * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
	 */
	virtual void doInfo(const infoMode& im) {
#ifdef DEBUG
		if(!optimizationMonitor_ptr_) {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::doInfo():" << std::endl
					<< "optimizationMonitor_ptr_ is empty when it shouldn't be."
			);
		}
#endif /* DEBUG */

		(this->optimizationMonitor_ptr_)->informationFunction(im, this);
	}

	/**************************************************************************************/
	/**
	 * Registers an optimizationMonitor object (or a derivative) with this object. Note
	 * that this class will take ownership of the optimization monitor by cloning it.
	 * You can thus assign the same boost::shared_ptr<GOptimizationAlgorithmT<ind_type> >
	 * to different objects.
	 *
	 * @param om_ptr A shared pointer to a specific optimization monitor
	 */
	void registerOptimizationMonitor(boost::shared_ptr<GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT> om_ptr) {
#ifdef DEBUG
		if(!om_ptr) {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::registerOptimizationMonitor():" << std::endl
					<< "om_ptr is empty when it shouldn't be."
			);
		}
#endif /* DEBUG */

		this->optimizationMonitor_ptr_ = om_ptr->GObject::clone<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT>();
	}

	/**************************************************************************************/
	/**
	 * Retrieves the default population size
	 *
	 * @return The default population size
	 */
	std::size_t getDefaultPopulationSize() const {
		return defaultPopulationSize_;
	}

	/**************************************************************************************/
	/**
	 * Retrieve the current population size
	 *
	 * @return The current population size
	 */
	std::size_t getPopulationSize() const {
		return this->size();
	}

	/**************************************************************************************/
	/**
	 * Set the number of iterations after which the optimization should
	 * be stopped
	 *
	 * @param maxIteration The number of iterations after which the optimization should terminate
	 */
	void setMaxIteration(boost::uint32_t maxIteration) {
		maxIteration_ = maxIteration;
	}

	/**************************************************************************************/
	/**
	 * Retrieve the number of iterations after which optimization should
	 * be stopped
	 *
	 * @return The number of iterations after which the optimization should terminate
	 */
	boost::uint32_t getMaxIteration() const {
		return maxIteration_;
	}

	/**************************************************************************************/
	/**
	 * Sets the maximum number of generations allowed without improvement of the best
	 * individual. Set to 0 in order for this stop criterion to be disabled.
	 *
	 * @param The maximum number of allowed generations
	 */
	void setMaxStallIteration(boost::uint32_t maxStallIteration) {
		maxStallIteration_ = maxStallIteration;
	}

	/**************************************************************************************/
	/**
	 * Retrieves the maximum number of generations allowed in an optimization run without
	 * improvement of the best individual.
	 *
	 * @return The maximum number of generations
	 */
	boost::uint32_t getMaxStallIteration() const {
		return maxStallIteration_;
	}

	/**************************************************************************************/
	/**
	 * Sets the maximum allowed processing time
	 *
	 * @param maxDuration The maximum allowed processing time
	 */
	void setMaxTime(boost::posix_time::time_duration maxDuration) {
		using namespace boost::posix_time;

		// Only allow "real" values
		if(maxDuration.is_special() || maxDuration.is_negative()) {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::setMaxTime() :" << std::endl
					<< "Invalid maxDuration."
			);
		}

		maxDuration_ = maxDuration;
	}

	/**************************************************************************************/
	/**
	 * Retrieves the value of the maxDuration_ parameter.
	 *
	 * @return The maximum allowed processing time
	 */
	boost::posix_time::time_duration getMaxTime() const {
		return maxDuration_;
	}

	/**************************************************************************************/
	/**
	 *  Sets a quality threshold beyond which optimization is expected to stop
	 *
	 *  @param qualityThreshold A threshold beyond which optimization should stop
	 *  @param hasQualityThreshold Allows to (de-)activtae the quality threshold
	 */
	void setQualityThreshold(double qualityThreshold, bool hasQualityThreshold = true) {
		qualityThreshold_ = qualityThreshold;
		hasQualityThreshold_=true;
	}

	/**************************************************************************************/
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

	/**************************************************************************************/
	/**
	 * Removes the quality threshold
	 *
	 * TODO: rename to resetQualityThreshold()
	 */
	void unsetQualityThreshold() {
		hasQualityThreshold_ = false;
	}

	/**************************************************************************************/
	/**
	 * Checks whether a quality threshold has been set
	 *
	 * @return A boolean indicating whether a quality threshold has been set
	 */
	bool hasQualityThreshold() const {
		return hasQualityThreshold_;
	}

	/**************************************************************************************/
	/**
	 * Retrieve the current iteration of the optimization run
	 *
	 * @return The current iteration of the optimization run
	 */
	boost::uint32_t getIteration() const {
		return iteration_;
	}

	/**************************************************************************************/
	/**
	 * Returns the current offset used to calculate the current iteration
	 *
	 * @return The current iteration offset
	 */
	boost::uint32_t getIterationOffset() const {
		return offset_;
	}

	/**************************************************************************************/
	/**
	 * Sets the number of iterations after which the algorithm should
	 * report about its inner state.
	 *
	 * @param iter The number of iterations after which information should be emitted
	 */
	void setReportIteration(boost::uint32_t iter) {
		reportIteration_ = iter;
	}

	/**************************************************************************************/
	/**
	 * Returns the number of iterations after which the algorithm should
	 * report about its inner state.
	 *
	 * @return The number of iterations after which information is emitted
	 */
	boost::uint32_t getReportIteration() const {
		return reportIteration_;
	}

	/**************************************************************************************/
	/**
	 * Retrieves the current number of failed optimization attempts
	 *
	 * @return The current number of failed optimization attempts
	 */
	boost::uint32_t getStallCounter() const {
		return stallCounter_;
	}

	/**************************************************************************************/
	/**
	 * Retrieve the best value found in the entire optimization run so far
	 *
	 * @return The best fitness found so far
	 */
	double getBestFitness() const {
		return bestPastFitness_;
	}

	/**************************************************************************************/
	/**
	 * Retrieves the best value found in the current iteration
	 *
	 * @return The best value found in the current iteration
	 */
	double getBestCurrentFitness() const {
		return bestCurrentFitness_;
	}

	/**************************************************************************************/
	/**
	 * Specifies whether information about termination reasons should be emitted
	 *
	 * @param etr A boolean which specifies whether reasons for the termination of the optimization run should be emitted
	 */
	void setEmitTerminationReason(bool emitTerminatioReason) {
		emitTerminationReason_ = emitTerminatioReason;
	}

	/**************************************************************************************/
	/**
	 * Retrieves information on whether information about termination reasons should be emitted
	 *
	 * @return A boolean which specifies whether reasons for the termination of the optimization run will be emitted
	 */
	bool getEmitTerminationReason() const {
		return emitTerminationReason_;
	}

	/**************************************************************************************/
	/**
	 * This function converts an individual at a given position to the derived
	 * type and returns it. In DEBUG mode, the function will check whether the
	 * requested position exists.
	 *
	 * Note that this function will only be accessible to the compiler if ind_type
	 * is a derivative of GIndividual, thanks to the magic of Boost's enable_if and Type
	 * Traits libraries.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GIndividual object, as required by the user
	 */
	template <typename target_type>
	inline boost::shared_ptr<target_type> individual_cast(
			 const std::size_t& pos
		   , typename boost::enable_if<boost::is_base_of<GIndividual, target_type> >::type* dummy = 0
	) {
#ifdef DEBUG
		if(pos >= this->size()) {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::individual_cast<>() : Error" << std::endl
					<< "Tried to access position " << pos << " which is >= array size " << this->size()
			);
		}

		boost::shared_ptr<target_type> p = boost::dynamic_pointer_cast<target_type>(this->at(pos));

		if(p) return p;
		else {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::individual_cast<>() : Conversion error"
			);
		}
#else
		return boost::static_pointer_cast<target_type>((*this)[pos]);
#endif /* DEBUG */
	}

	/**************************************************************************************/
	/**
	 * This function is e.g. called from GSerialEA::adjustPopulation(). It
	 * currently only triggers actions for GParameterSet-derivatives. Optimization algorithms
	 * are unaffected. It might be useful to implement actions here as well, though, in order
	 * to make better use of Multi-Populations in Evolutionary Algorithms.
	 */
	void randomInit() { /* nothing */ }

	/**************************************************************************************/
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

	/**************************************************************************************/
	/**
	 * Gives access to the current optimization monitor
	 *
	 * @return A boost::shared_ptr to the current optimization monitor
	 */
	boost::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT> getOptimizationMonitor() {
		return this->optimizationMonitor_ptr_;
	}

	/**************************************************************************************/
	/**
	 * Adds local configuration options to a GParserBuilder object
	 *
	 * @param gpb The GParserBuilder object to which configuration options should be added
	 * @param showOrigin Makes the function indicate the origin of parameters in comments
	 */
	virtual void addConfigurationOptions (
		Gem::Common::GParserBuilder& gpb
		, const bool& showOrigin
	) {
		std::string comment;
		std::string comment1;
		std::string comment2;

		// Add local data
		comment = ""; // Reset the comment string
		comment += "The maximum allowed number of iterations;";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<boost::uint32_t>(
			"maxIteration" // The name of the variable
			, DEFAULTMAXIT // The default value
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setMaxIteration
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "The maximum allowed number of iterations without improvement";
		comment += "0 means: no constraint.;";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<boost::uint32_t>(
			"maxStallIteration" // The name of the variable
			, DEFAULTMAXSTALLIT // The default value
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setMaxStallIteration
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "The number of iterations after which a report should be issued;";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<boost::uint32_t>(
			"reportIteration" // The name of the variable
			, DEFAULTREPORTITER // The default value
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setReportIteration
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "The number of iterations after which a checkpoint should be written.;";
		comment += "-1 means: Write a checkpoint file whenever an improvement was encountered;";
		comment += " 0 means: Never emit checkpoint files.;";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<boost::int32_t>(
			"cpInterval" // The name of the variable
			, DEFAULTCHECKPOINTIT // The default value
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setCheckpointInterval
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
			, comment
		);

		comment1 = ""; // Reset the comment string
		comment1 += "The directory where checkpoint files should be stored.;";
		if(showOrigin) comment1 += "[GOptimizationAlgorithmT<ind_type>]";
		comment2 = ""; // Reset the comment string
		comment2 += "The significant part of the checkpoint file name.;";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<std::string, std::string>(
			"cpDirectory"  // The name of the first variable
			, "cpBaseName" // The name of the second variable
			, DEFAULTCPDIR // Default value for the first variable
			, DEFAULTCPBASENAME // Default value for the second variable
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setCheckpointBaseName
				, this
				, _1
				, _2
			  )
			, Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
			, comment1
			, comment2
		);

		comment = ""; // Reset the comment string
		comment += "Determines whether check-pointing should be done in;";
		comment += "text- (0), XML- (1), or binary-mode (2);";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<Gem::Common::serializationMode>(
			"cpSerMode" // The name of the variable
			, DEFAULTCPSERMODE // The default value
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setCheckpointSerializationMode
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
			, comment
		);

		comment1 = ""; // Reset the comment string
		comment1 += "A threshold beyond which optimization is expected to stop;";
		comment1 += "Note that in order to activate this threshold, you also need to;";
		comment1 += "set \"hasQualityThreshold\" to 1.;";
		if(showOrigin) comment1 += "[GOptimizationAlgorithmT<ind_type>]";
		comment2 = ""; // Reset the comment string
		comment2 += "Activates (1) or de-activates (0) the qualit threshold;";
		if(showOrigin) comment2 += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<double, bool>(
			"qualityThreshold" // The name of the variable
			, "qualityThresholdActive"
			, DEFAULTQUALITYTHRESHOLD // The default value
			, false
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setQualityThreshold
				, this
				, _1
				, _2
			  )
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment1
			, comment2
		);

		comment = ""; // Reset the comment string
		comment += "The maximum allowed time-frame for the optimization;";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<boost::posix_time::time_duration>(
			"maxDuration" // The name of the variable
			, boost::posix_time::duration_from_string(DEFAULTDURATION) // The default value
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setMaxTime
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_ESSENTIAL // Alternative: VAR_IS_SECONDARY
			, comment
		);

		comment = ""; // Reset the comment string
		comment += "Triggers emission (1) or omission (0) of information about reasons for termination;";
		if(showOrigin) comment += "[GOptimizationAlgorithmT<ind_type>]";
		gpb.registerFileParameter<bool>(
			"emitTerminationReason" // The name of the variable
			, false // The default value
			, boost::bind(
				&GOptimizationAlgorithmT<ind_type>::setEmitTerminationReason
				, this
				, _1
			  )
			, Gem::Common::VAR_IS_SECONDARY // Alternative: VAR_IS_ESSENTIAL
			, comment
		);

		// Call our parent class'es function
		GMutableSetT<ind_type>::addConfigurationOptions(gpb, showOrigin);
	}


protected:
	/**************************************************************************************/
	/**
	 * Loads the data of another GObject
	 *
	 * @param cp Another GOptimizationAlgorithm object, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp)
	{
		const GOptimizationAlgorithmT<ind_type> *p_load = GObject::gobject_conversion<GOptimizationAlgorithmT<ind_type> >(cp);

		// Load the parent class'es data
		GMutableSetT<ind_type>::load_(cp);

		// and then our local data
		iteration_ = p_load->iteration_;
		offset_ = p_load->offset_;
		maxIteration_ = p_load->maxIteration_;
		maxStallIteration_ = p_load->maxStallIteration_;
		reportIteration_ = p_load->reportIteration_;
		defaultPopulationSize_ = p_load->defaultPopulationSize_;
		bestPastFitness_ = p_load->bestPastFitness_;
		bestCurrentFitness_ = p_load->bestCurrentFitness_;
		stallCounter_ = p_load->stallCounter_;
		cpInterval_ = p_load->cpInterval_;
		cpBaseName_ = p_load->cpBaseName_;
		cpDirectory_ = p_load->cpDirectory_;
		cpSerMode_ = p_load->cpSerMode_;
		qualityThreshold_ = p_load->qualityThreshold_;
		hasQualityThreshold_ = p_load->hasQualityThreshold_;
		maxDuration_ = p_load->maxDuration_;
		emitTerminationReason_ = p_load->emitTerminationReason_;
		halted_ = p_load->halted_;
		this->optimizationMonitor_ptr_ = p_load->optimizationMonitor_ptr_->GObject::clone<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT>();
	}

	/**************************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const = 0;

	/**************************************************************************************/
	/** @brief Allows derived classes to set the personality type of the individuals */
	virtual void setIndividualPersonalities() = 0;

	/**************************************************************************************/
	/**
	 * Resets the individual's personality types
	 */
	void resetIndividualPersonalities() {
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) (*it)->resetPersonality();
	}

	/**************************************************************************************/
	/** @brief Saves the state of the class to disc */
	virtual void saveCheckpoint() const = 0;

	/**************************************************************************************/
	/** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
	virtual double cycleLogic() = 0;

	/**************************************************************************************/
	/**
	 * Sets the default size of the population
	 *
	 * @param popSize The desired size of the population
	 */
	virtual void setDefaultPopulationSize(const std::size_t& defPopSize) {
		defaultPopulationSize_ = defPopSize;
	}

	/**************************************************************************************/
	/**
	 * It is possible for derived classes to specify in overloaded versions of this
	 * function under which conditions the optimization should be stopped. The
	 * function is called from GOptimizationAlgorithmT<ind_type>::halt .
	 *
	 * @return boolean indicating that a stop condition was reached
	 */
	virtual bool customHalt() const {
		/* nothing - specify your own criteria in derived classes. Make sure
		 * to emit a suitable message if execution was halted due to a
		 * custom criterion */
		return false;
	}

	/**************************************************************************************/
	/**
	 * Fitness calculation for a population means optimization. The fitness is then determined
	 * by the best individual which, after the end of the optimization cycle, can be found in
	 * the first position of the array. Note that this function will only take into account the
	 * fitness of the first registered evaluation criterion in an individual.
	 *
	 * @param The id of an evaluation criterion (will be ignored by this function)
	 * @return The fitness of the best individual in the population
	 */
	virtual double fitnessCalculation() {
		bool dirty = false;

		GOptimizableI::optimize();

		double val = this->at(0)->getCachedFitness(dirty);
		// is this the current fitness ? We should at this stage never
		// run across an unevaluated individual.
		if(dirty) {
			raiseException(
					"In GOptimizationAlgorithmT<ind_type>::fitnessCalculation():" << std::endl
					<< "Came across dirty individual"
			);
		}
		return val;
	}

	/**************************************************************************************/
	/**
	 * Allows derived classes to reset the stall counter.
	 */
	void resetStallCounter() {
		stallCounter_ = 0;
	}

	/**************************************************************************************/
	/**
	 * Helps to determine whether a given value is strictly better (i.e. better than equal)
	 * than another one. As "better" means something different for maximization and minimization,
	 * this function helps to make the code easier to understand.
	 *
	 * @param newValue The new value
	 * @param oldValue The old value
	 * @return true if newValue is better than oldValue, otherwise false.
	 */
	bool isBetter(double newValue, const double& oldValue) const {
		if(this->getMaxMode()) {
			if(newValue > oldValue) return true;
			else return false;
		}
		else { // minimization
			if(newValue < oldValue) return true;
			else return false;
		}
	}

	/**************************************************************************************/
	/**
	 * Helps to determine whether a given value is strictly worse (i.e. worse than equal)
	 * than another one. As "worse" means something different for maximization and minimization,
	 * this function helps to make the code easier to understand.
	 *
	 * @param newValue The new value
	 * @param oldValue The old value
	 * @return true of newValue is worse than oldValue, otherwise false.
	 */
	bool isWorse(double newValue, const double& oldValue) const {
		if(this->getMaxMode()) {
			if(newValue < oldValue) return true;
			else return false;
		}
		else { // minimization
			if(newValue > oldValue) return true;
			else return false;
		}
	}

	/**************************************************************************************/
	/**
	 * Helper function that emits the worst case value depending on whether maximization
	 * or minimization is performed.
	 *
	 * @return The worst case value, depending on maximization or minimization
	 */
	double getWorstCase() const {
		return (this->getMaxMode()?-DBL_MAX:DBL_MAX);
	}

	/**************************************************************************************/
	/**
	 * Allows to perform initialization work before the optimization cycle starts. This
	 * function will usually be overloaded by derived functions, which should however,
	 * as one of their first actions, call this function. It is not recommended  to perform
	 * any "real" optimization work here, such as evaluation of individuals. Use the
	 * optimizationInit() function instead.
	 */
	virtual void init() {
		// Tell all individuals in this collection to update their random number generators
		// with the one contained in GMutableSetT. Note: This will only have an effect on
		// GParameterSet objects, as GIndividual contains an empty function.
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->updateRNGs();
		}
	}

	/**************************************************************************************/
	/**
	 * Allows to perform any remaining work after the optimization cycle has finished.
	 * This function will usually be overloaded by derived functions, which shoudl however
	 * call this function as one of their last actions. It is not recommended  to perform
	 * any "real" optimization work here, such as evaluation of individuals. Use the
	 * optimizationFinalize() function instead.
	 */
	virtual void finalize()	{
		// Tell all individuals in this collection to tell all GParameterBase derivatives
		// to again use their local generators.
		typename GOptimizationAlgorithmT<ind_type>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->restoreRNGs();
		}
	}

	/**************************************************************************************/
	/**
	 * This function performs any initial optimization work (such as the evaluation of a
	 * single individual).
	 */
	virtual void optimizationInit()
	{ /* nothing */ }

	/**************************************************************************************/
	/**
	 * This function performs any final optimization work (such as the evaluation of a
	 * single individual).
	 */
	virtual void optimizationFinalize()
	{ /* nothing */ }

	/**************************************************************************************/
	/** @brief Resizes the population to the desired level and does some error checks */
	virtual void adjustPopulation() = 0;

	/**************************************************************************************/
	/**
	 * Lets individuals know about the current iteration of the optimization
	 * cycle.
	 */
	virtual void markIteration() {
		typename std::vector<boost::shared_ptr<ind_type> >::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setAssignedIteration(iteration_);
		}
	}

	/**************************************************************************************/
	/**
	 * A little helper function that determines whether we are currently inside of the first
	 * iteration
	 *
	 * @return A boolean indicating whether we are inside of the first iteration
	 */
	bool inFirstIteration() const {
		return iteration_ == offset_;
	}

	/**************************************************************************************/
	/**
	 * A little helper function that determines whether we are after the first iteration
	 *
	 * @return A boolean indicating whether we are after the first iteration
	 */
	bool afterFirstIteration() const {
		return iteration_ > offset_;
	}

private:
	/**************************************************************************************/
	/**
	 * This function returns true once a given time (set with
	 * GOptimizationAlgorithm<ind_type>::setMaxTime()) has passed.
	 * It is used in the GOptimizationAlgorithmT<ind_type>::halt() function.
	 *
	 * @return A boolean indicating whether a given amount of time has passed
	 */
	bool timedHalt() const {
		using namespace boost::posix_time;
		ptime currentTime = microsec_clock::local_time();
		if((currentTime - startTime_) >= maxDuration_) {
			if(emitTerminationReason_) {
				std::cerr << "Terminating optimization run because maximum time frame has been exceeded." << std::endl;
			}

			return true;
		} else {
			return false;
		}
	}

	/**************************************************************************************/
	/**
	 * This function returns true once the quality is below or above a given threshold
	 * (depending on whether we maximize or minimize).
	 *
	 * @return A boolean indicating whether the quality is above or below a given threshold
	 */
	bool qualityHalt() const {
		if(isBetter(bestPastFitness_, qualityThreshold_)) {
			if(emitTerminationReason_) {
				std::cerr << "Terminating optimization run because quality threshold has been reached." << std::endl;
			}

			return true;
		} else {
			return false;
		}
	}

	/**************************************************************************************/
	/**
	 * This function returns true once a given number of stalls has been exceeded in a row
	 *
	 * @return A boolean indicating whether the optimization has stalled too often in a row
	 */
	bool stallHalt() const {
		if(stallCounter_ > maxStallIteration_) {
			if(emitTerminationReason_) {
				std::cout << "Terminating optimization run because maximum number of stalls has been exceeded." << std::endl;
			}

			return true;
		} else {
			return false;
		}
	}

	/**************************************************************************************/
	/**
	 * This function returns true once a maximum number of iterations has been exceeded
	 *
	 * @return A boolean indicating whether the maximum number of iterations has been exceeded
	 */
	bool iterationHalt() const {
		if(iteration_ >= (maxIteration_ + offset_)) {
			if(emitTerminationReason_) {
				std::cout << "Terminating optimization run because iteration threshold has been reached." << std::endl;
			}

			return true;
		} else {
			return false;
		}
	}

	/**************************************************************************************/
	/**
	 * A wrapper for the customHalt() function that allows us to emit the termination reason
	 *
	 * @return A boolean indicating whether a custom halt criterion has been reached
	 */
	bool customHalt_() const {
		if(customHalt()) {
			if(emitTerminationReason_) {
				std::cerr << "Terminating optimization run because custom halt criterion has triggered." << std::endl;
			}

			return true;
		} else {
			return false;
		}

	}

	/**************************************************************************************/
	/**
	 * This function checks whether a halt criterion has been reached. The most
	 * common criterion is the maximum number of iterations. Set the maxIteration_
	 * counter to 0 if you want to disable this criterion.
	 *
	 * @return A boolean indicating whether a halt criterion has been reached
	 */
	bool halt() const
	{
		// Have we exceeded the maximum number of iterations and
		// do we indeed intend to stop in this case ?
		if(maxIteration_ && iterationHalt()) return true;

		// Has the optimization stalled too often ?
		if(maxStallIteration_ && stallHalt()) return true;

		// Do we have a scheduled halt time ? The comparatively expensive
		// timedHalt() calculation is only called if maxDuration_
		// is at least one microsecond.
		if(maxDuration_.total_microseconds() && timedHalt()) return true;

		// Are we supposed to stop when the quality has exceeded a threshold ?
		if(hasQualityThreshold_ && qualityHalt()) return true;

		// Has the user specified an additional stop criterion ?
		if(customHalt_()) return true;

		// Fine, we can continue.
		return false;
	}

	/**************************************************************************************/
	/**
	 * Retrieves the individual's maximization mode and sets our own mode accordingly. This
	 * function effectively steers whether the entire algorithm will maximize or minimize the
	 * evaluation function.
	 */
	void setLocalMaxMode() {
		// Do some error checking
		if(this->empty()) {
			raiseException(
					"In GOptimizationAlgorithmT::setLocalMaxMode():" << std::endl
					<< "There should at least be one individual present at this stage." << std::endl
					<< "Found none."
			);
		}

		bool localMaxMode = this->at(0)->getMaxMode();
		for(std::size_t i=1; i<this->size(); i++) {
			if(this->at(i)->getMaxMode() != localMaxMode) {
				raiseException(
						"In GOptimizationAlgorithmT::setLocalMaxMode():" << std::endl
						<< "Found individual with maximization mode " << this->at(i)->getMaxMode() << " in position " << i << std::endl
						<< "where " << localMaxMode << " was expected."
				);
			}
		}

		this->setMaxMode_(localMaxMode);
	}

	/**************************************************************************************/
	/**
	 * Marks the globally best known fitness in all individuals
	 */
	void markBestFitness() {
		typename std::vector<boost::shared_ptr<ind_type> >::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setBestKnownFitness(bestPastFitness_);
		}
	}

	/**************************************************************************************/
	/**
	 * Marks the number of stalled optimization attempts in all individuals
	 */
	void markNStalls() {
		typename std::vector<boost::shared_ptr<ind_type> >::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) {
			(*it)->setNStalls(stallCounter_);
		}
	}

	/**************************************************************************************/
	boost::uint32_t iteration_; ///< The current iteration
	boost::uint32_t offset_; ///< An iteration offset which can be used, if the optimization starts from a checkpoint file
	boost::uint32_t maxIteration_; ///< The maximum number of iterations
	boost::uint32_t maxStallIteration_; ///< The maximum number of generations without improvement, after which optimization is stopped
	boost::uint32_t reportIteration_; ///< The number of generations after which a report should be issued
	std::size_t defaultPopulationSize_; ///< The nominal size of the population
	double bestPastFitness_; ///< Records the best fitness found in past generations
	double bestCurrentFitness_; ///< Records the best fitness found in the current iteration
	boost::uint32_t stallCounter_; ///< Counts the number of iterations without improvement
	boost::int32_t cpInterval_; ///< Number of iterations after which a checkpoint should be written. -1 means: Write whenever an improvement was encountered
	std::string cpBaseName_; ///< The base name of the checkpoint file
	std::string cpDirectory_; ///< The directory where checkpoint files should be stored
	Gem::Common::serializationMode cpSerMode_; ///< Determines whether check-pointing should be done in text-, XML, or binary mode
	double qualityThreshold_; ///< A threshold beyond which optimization is expected to stop
	bool hasQualityThreshold_; ///< Specifies whether a qualityThreshold has been set
	boost::posix_time::time_duration maxDuration_; ///< Maximum time frame for the optimization
	mutable boost::posix_time::ptime startTime_; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
	bool emitTerminationReason_; ///< Specifies whether information about reasons for termination should be emitted
	bool halted_; ///< Set to true when halt() has returned "true"
	boost::shared_ptr<typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT> optimizationMonitor_ptr_;

#ifdef GENEVATESTING
public:
	/**************************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() {
		bool result = false;

		// Call the parent class'es function
		if(GMutableSetT<ind_type>::modify_GUnitTests()) result = true;

		return result;
	}

	/**************************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() {
		// Call the parent class'es function
		GMutableSetT<ind_type>::specificTestsNoFailureExpected_GUnitTests();
	}

	/**************************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() {
		// Call the parent class'es function
		GMutableSetT<ind_type>::specificTestsFailuresExpected_GUnitTests();
	}

	/**************************************************************************************/
#endif /* GENEVATESTING */


public:
	/**************************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////////
	/**************************************************************************************/
	/**
	 * This nested class defines the interface of optimization monitors, as used
	 * in the Geneva library. It also provides users with some basic information.
	 */
	class GOptimizationMonitorT
		: public GObject
	{
	    ///////////////////////////////////////////////////////////////////////
	    friend class boost::serialization::access;

	    template<typename Archive>
	    void serialize(Archive & ar, const unsigned int){
	      using boost::serialization::make_nvp;

	      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
	      	 & BOOST_SERIALIZATION_NVP(quiet_)
	      	 & BOOST_SERIALIZATION_NVP(resultFile_);
	    }
	    ///////////////////////////////////////////////////////////////////////

	public:
	    /**********************************************************************************/
	    /**
	     * The default constructor
	     */
	    GOptimizationMonitorT()
	    	: quiet_(false)
	    	, resultFile_(DEFAULTRESULTFILEOM)
	    { /* nothing */ }

	    /**********************************************************************************/
	    /**
	     * The copy constructor
	     *
	     * @param cp A copy of another GOptimizationMonitorT object
	     */
	    GOptimizationMonitorT(const GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT& cp)
	    	: GObject(cp)
	    	, quiet_(cp.quiet_)
	    	, resultFile_(cp.resultFile_)
	    { /* nothing */ }

	    /**********************************************************************************/
	    /**
	     * The destructor
	     */
	    virtual ~GOptimizationMonitorT()
	    { /* nothing */ }

	    /**********************************************************************************/
	    /**
	     * The standard assignment operator.
	     *
	     * @param cp Another GOptimizationMonitorT<ind_type> object
	     * @return A constant reference to this object
	     */
	    const GOptimizationMonitorT& operator=(const GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT& cp) {
	    	GOptimizationMonitorT::load_(&cp);
	    	return *this;
	    }

	    /**********************************************************************************/
	    /**
	     * Checks for equality with another GParameter Base object
	     *
	     * @param  cp A constant reference to another GOptimizationMonitorT object
	     * @return A boolean indicating whether both objects are equal
	     */
	    virtual bool operator==(const GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT& cp) const {
	    	using namespace Gem::Common;
	    	// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
	    	return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT::operator==","cp", CE_SILENT);
	    }

	    /**********************************************************************************/
	    /**
	     * Checks for inequality with another GOptimizationMonitorT object
	     *
	     * @param  cp A constant reference to another GOptimizationMonitorT object
	     * @return A boolean indicating whether both objects are inequal
	     */
	    virtual bool operator!=(const GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT& cp) const {
	    	using namespace Gem::Common;
	    	// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
	    	return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT::operator!=","cp", CE_SILENT);
	    }

	    /**********************************************************************************/
	    /**
	     * Checks whether a given expectation for the relationship between this object and another object
	     * is fulfilled.
	     *
	     * @param cp A constant reference to another object, camouflaged as a GObject
	     * @param e The expected outcome of the comparison
	     * @param limit The maximum deviation for floating point values (important for similarity checks)
	     * @param caller An identifier for the calling entity
	     * @param y_name An identifier for the object that should be compared to this one
	     * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	     * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	     */
	    virtual boost::optional<std::string> checkRelationshipWith(
	    		const GObject& cp
	    		, const Gem::Common::expectation& e
	    		, const double& limit
	    		, const std::string& caller
	    		, const std::string& y_name
	    		, const bool& withMessages
	    ) const {
	        using namespace Gem::Common;

	    	// Check that we are indeed dealing with a GOptimizationMonitorT reference
	    	const GOptimizationMonitorT *p_load = GObject::gobject_conversion<GOptimizationMonitorT>(&cp);

	    	// Will hold possible deviations from the expectation, including explanations
	        std::vector<boost::optional<std::string> > deviations;

	    	// Check our parent class'es data ...
	        deviations.push_back(GObject::checkRelationshipWith(cp, e, limit, caller, y_name, withMessages));

	    	// ... and then our local data
			EXPECTATIONCHECK(quiet_);
			EXPECTATIONCHECK(resultFile_);

	    	return evaluateDiscrepancies("GOptimizationMonitorT", caller, deviations, e);
	    }

	    /**********************************************************************************/
	    /**
	     * The actual information function
	     *
	     * @param im The mode in which the information function is called
	     * @param goa A pointer to the current optimization algorithm for which information should be emitted
	     */
	    void informationFunction(const infoMode& im, GOptimizationAlgorithmT<ind_type> * const goa) {
	    	if(quiet_) return;

	    	switch(im) {
	    	case Gem::Geneva::INFOINIT:
	    	{
	    		// Make sure we have an output path
	    		summary_.open(resultFile_.c_str());
	    		if(!summary_) {
	    			raiseException(
	    					"In GOptimizationMonitorT<T>::informationFunction():" << std::endl
	    					<< "Could not open output file \"" << resultFile_ << "\""
	    			);
	    		}

	    		// Emit the header and perform any necessary initialization work
	    		summary_ << this->firstInformation(goa) << std::flush;
	    	}
	    	break;

	    	case Gem::Geneva::INFOPROCESSING:
	    	{
	    		// Regular information emitted during each iteration
	    		summary_ << this->cycleInformation(goa) << std::flush;
	    	}
	    	break;

	    	case Gem::Geneva::INFOEND:
	    	{
	    		// Emit any remaining information
	    		summary_ << this->lastInformation(goa) << std::flush;

	    		// Clean up
	    		summary_.close();
	    	}
	    	break;
	    	};
	    }

	    /**********************************************************************************/
	    /**
	     * Prevents any information from being emitted by this object
	     */
	    void preventInformationEmission() {
	    	quiet_ = true;
	    }

	    /**********************************************************************************/
	    /**
	     * Allows this object to emit information
	     */
	    void allowInformationEmission() {
	    	quiet_ = false;
	    }

	    /**********************************************************************************/
	    /**
	     * Allows to check whether the emission of information is prevented
	     *
	     * @return A boolean which indicates whether information emission is prevented
	     */
	    bool informationEmissionPrevented() const {
	    	return quiet_;
	    }

	    /**********************************************************************************/
	    /**
	     * Allows to specify a different name for the result file
	     *
	     * @param resultFile The desired name of the result file
	     */
	    void setResultFileName(const std::string& resultFile) {
	    	resultFile_ = resultFile;
	    }

	    /**********************************************************************************/
	    /**
	     * Allows to retrieve the current value of the result file name
	     *
	     * @return The current name of the result file
	     */
	    std::string getResultFileName() const {
	    	return resultFile_;
	    }

	protected:
	    /**********************************************************************************/
	    /**
	     * A function that is called once before the optimization starts
	     *
	     * @param goa A pointer to the current optimization algorithm for which information should be emitted
	     * @return A string containing information to written to the output file (if any)
	     */
	    virtual std::string firstInformation(GOptimizationAlgorithmT<ind_type> * const goa) {
	    	std::ostringstream result;
	    	result << "Starting an optimization run with algorithm \"" << goa->getAlgorithmName() << "\"" << std::endl;
	    	return result.str();
	    }

	    /**********************************************************************************/
	    /**
	     * A function that is called during each optimization cycle. It is possible to
	     * extract quite comprehensive information in each iteration. For examples, see
	     * the standard overloads provided for the various optimization algorithms.
	     *
	     * @param goa A pointer to the current optimization algorithm for which information should be emitted
	     * @return A string containing information to written to the output file (if any)
	     */
	    virtual std::string cycleInformation(GOptimizationAlgorithmT<ind_type> * const goa) {
	    	std::ostringstream result;
	    	result << std::setprecision(15) << goa->getIteration() << ": " << goa->getBestCurrentFitness() << std::endl;
	    	return result.str();
	    }

	    /**********************************************************************************/
	    /**
	     * A function that is called once at the end of the optimization cycle
	     *
	     * @param goa A pointer to the current optimization algorithm for which information should be emitted
		 * @return A string containing information to written to the output file (if any)
	     */
	    virtual std::string lastInformation(GOptimizationAlgorithmT<ind_type> * const goa) {
	    	std::ostringstream result;
	    	result << "End of optimization reached in algorithm \""<< goa->getAlgorithmName() << "\"" << std::endl;
	    	return result.str();
	    }

	    /**********************************************************************************/
	    /**
	     * Loads the data of another object
	     *
	     * cp A pointer to another GOptimizationMonitorT object, camouflaged as a GObject
	     */
	    virtual void load_(const GObject* cp) {
	    	const GOptimizationMonitorT *p_load = GObject::gobject_conversion<GOptimizationMonitorT>(cp);

	    	// Load the parent classes' data ...
	    	GObject::load_(cp);

	    	// ... and then our local data
	    	quiet_ = p_load->quiet_;
	    	resultFile_ = p_load->resultFile_;
	    }

	    /**********************************************************************************/
	    /**
	     * Creates a deep clone of this object
	     */
		virtual GObject* clone_() const {
			return new typename GOptimizationAlgorithmT<ind_type>::GOptimizationMonitorT(*this);
		}

	private:
		/**********************************************************************************/
		bool quiet_; ///< Specifies whether any information should be emitted at all
		std::string resultFile_; ///< Specifies where result information should be sent to
		std::ofstream summary_; ///< The stream to which information is written (not serialized)

#ifdef GENEVATESTING
	public:
		/**********************************************************************************/
		/**
		 * Applies modifications to this object. This is needed for testing purposes
		 */
		virtual bool modify_GUnitTests() {
			bool result = false;

			// Call the parent class'es function
			if(GObject::modify_GUnitTests()) result = true;

			return result;
		}

		/**********************************************************************************/
		/**
		 * Performs self tests that are expected to succeed. This is needed for testing purposes
		 */
		virtual void specificTestsNoFailureExpected_GUnitTests() {
			// Call the parent class'es function
			GObject::specificTestsNoFailureExpected_GUnitTests();
		}

		/**********************************************************************************/
		/**
		 * Performs self tests that are expected to fail. This is needed for testing purposes
		 */
		virtual void specificTestsFailuresExpected_GUnitTests() {
			// Call the parent class'es function
			GObject::specificTestsFailuresExpected_GUnitTests();
		}

		/**********************************************************************************/
#endif /* GENEVATESTING */
	};

	/**************************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////////
	/**************************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename ind_type>
    struct is_abstract<Gem::Geneva::GOptimizationAlgorithmT<ind_type> > : public boost::true_type {};
    template<typename ind_type>
    struct is_abstract< const Gem::Geneva::GOptimizationAlgorithmT<ind_type> > : public boost::true_type {};
  }
}

/******************************************************************************************/

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GIndividual>::GOptimizationMonitorT)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::GOptimizationMonitorT)
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GBrokerConnectorT<Gem::Geneva::GIndividual>)

/******************************************************************************************/

#endif /* GOPTIMIZATIONALGORITHMT_HPP_ */
