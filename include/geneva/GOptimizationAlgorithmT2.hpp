/**
 * @file GOptimizationAlgorithmT2.hpp
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
#include <chrono>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifndef GOptimizationAlgorithmT2_HPP_
#define GOptimizationAlgorithmT2_HPP_

// Geneva headers go here
#include "hap/GRandomT.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GExceptions.hpp"
#include "common/GStdPtrVectorInterfaceT.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GOptimizableI.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"
#include "geneva/GParameterBase.hpp"

namespace Gem {
namespace Geneva {

// TODO: m_executor is not loaded or compared. Fix or use m_executor as a (private ?) base class instead
// TODO: make m_executor private and provide protected access functions for derived classes

/***************************************************************************/
/**
 * This class implements basic operations found in iteration-based optimization algorithms.
 * E.g., one might want to stop the optimization after a given number of cycles, or after
 * a given amount of time. The class also defines the interface functions common to these
 * algorithms, such as a general call to "optimize()". All evaluation of individuals is
 * handled inside of an executor object.
 */
template <
	typename executor_type = Gem::Courtier::GBrokerExecutorT<GParameterSet>
>
class GOptimizationAlgorithmT2
   : public GObject
	, public GOptimizableI
   , public Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>
{
public:
	 // Forward declarations, as these classes are only defined at the end of this file
	 class GOptimizationMonitorT;
	 class GBasePluggableOMT;

	 static_assert(
		 std::enable_if<std::is_base_of<Gem::Courtier::GBaseExecutorT<GParameterSet>,executor_type>::value>::type
		 , "Error: executor_type is not derived from GBaseExecutorT<GParameterSet>"
	 );

private:
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
		 & make_nvp("GOptimizableI", boost::serialization::base_object<GOptimizableI>(*this))
		 & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>>(*this))
		 & BOOST_SERIALIZATION_NVP(m_iteration)
		 & BOOST_SERIALIZATION_NVP(m_offset)
		 & BOOST_SERIALIZATION_NVP(m_max_iteration)
		 & BOOST_SERIALIZATION_NVP(m_min_iteration)
		 & BOOST_SERIALIZATION_NVP(m_max_stall_iteration)
		 & BOOST_SERIALIZATION_NVP(m_report_iteration)
		 & BOOST_SERIALIZATION_NVP(m_n_record_best_global_individuals)
		 & BOOST_SERIALIZATION_NVP(m_best_globalIndividuals)
		 & BOOST_SERIALIZATION_NVP(m_default_population_size)
		 & BOOST_SERIALIZATION_NVP(m_best_known_primary_fitness)
		 & BOOST_SERIALIZATION_NVP(m_best_current_primary_fitness)
		 & BOOST_SERIALIZATION_NVP(m_stall_counter)
		 & BOOST_SERIALIZATION_NVP(m_stall_counter_threshold)
		 & BOOST_SERIALIZATION_NVP(m_cp_interval)
		 & BOOST_SERIALIZATION_NVP(m_cp_base_name)
		 & BOOST_SERIALIZATION_NVP(m_cp_directory)
		 & BOOST_SERIALIZATION_NVP(m_cp_serialization_mode)
		 & BOOST_SERIALIZATION_NVP(m_cp_overwrite)
		 & BOOST_SERIALIZATION_NVP(m_quality_threshold)
		 & BOOST_SERIALIZATION_NVP(m_has_quality_threshold)
		 & BOOST_SERIALIZATION_NVP(m_max_duration)
		 & BOOST_SERIALIZATION_NVP(m_min_duration)
		 & BOOST_SERIALIZATION_NVP(m_termination_file)
		 & BOOST_SERIALIZATION_NVP(m_terminate_on_file_modification)
		 & BOOST_SERIALIZATION_NVP(m_emit_termination_reason)
		 & BOOST_SERIALIZATION_NVP(m_halted)
		 & BOOST_SERIALIZATION_NVP(m_worst_known_valids_vec)
		 & BOOST_SERIALIZATION_NVP(m_pluggable_monitors_vec)
		 & BOOST_SERIALIZATION_NVP(m_executor);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor. Note that most variables are initialized in the class body.
	  */
	 GOptimizationAlgorithmT2()
		 : GObject()
		 , Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>()
		 , m_best_globalIndividuals(m_n_record_best_global_individuals, Gem::Common::LOWERISBETTER)
		 , m_best_iteration_individuals(0, Gem::Common::LOWERISBETTER) // unlimited size, so all individuals of an iteration fit in
		 , m_best_known_primary_fitness(std::tuple<double,double>(0.,0.)) // will be set appropriately in the optimize() function
		 , m_best_current_primary_fitness(std::tuple<double,double>(0.,0.)) // will be set appropriately in the optimize() function
		 , m_max_duration(Gem::Common::duration_from_string(DEFAULTDURATION))
		 , m_min_duration(Gem::Common::duration_from_string(DEFAULTMINDURATION))
		 , m_worst_known_valids_vec()
		 , m_pluggable_monitors_vec()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A constant reference to another GOptimizationAlgorithmT2 object
	  */
	 GOptimizationAlgorithmT2(const GOptimizationAlgorithmT2<executor_type>& cp)
		 : GObject(cp)
		 , Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>(cp)
		 , m_iteration(cp.m_iteration)
		 , m_offset(DEFAULTOFFSET)
		 , m_min_iteration(cp.m_min_iteration)
		 , m_max_iteration(cp.m_max_iteration)
		 , m_max_stall_iteration(cp.m_max_stall_iteration)
		 , m_report_iteration(cp.m_report_iteration)
		 , m_n_record_best_global_individuals(cp.m_n_record_best_global_individuals)
		 , m_best_globalIndividuals(cp.m_best_globalIndividuals)
		 , m_best_iteration_individuals(cp.m_best_iteration_individuals)
		 , m_default_population_size(cp.m_default_population_size)
		 , m_best_known_primary_fitness(cp.m_best_known_primary_fitness)
		 , m_best_current_primary_fitness(cp.m_best_current_primary_fitness)
		 , m_stall_counter(cp.m_stall_counter)
		 , m_stall_counter_threshold(cp.m_stall_counter_threshold)
		 , m_cp_interval(cp.m_cp_interval)
		 , m_cp_base_name(cp.m_cp_base_name)
		 , m_cp_directory(cp.m_cp_directory)
		 , m_cp_overwrite(cp.m_cp_overwrite)
		 , m_cp_serialization_mode(cp.m_cp_serialization_mode)
		 , m_quality_threshold(cp.m_quality_threshold)
		 , m_has_quality_threshold(cp.m_has_quality_threshold)
		 , m_max_duration(cp.m_max_duration)
		 , m_min_duration(cp.m_min_duration)
		 , m_termination_file(cp.m_termination_file)
		 , m_terminate_on_file_modification(cp.m_terminate_on_file_modification)
		 , m_emit_termination_reason(cp.m_emit_termination_reason)
		 , m_halted(cp.m_halted)
		 , m_worst_known_valids_vec(cp.m_worst_known_valids_vec)
	 	 , m_executor(cp.m_executor)
	 {
		 // Copy the pluggable optimization monitors over (if any)
		 Gem::Common::copyCloneableSmartPointerContainer(cp.m_pluggable_monitors_vec, m_pluggable_monitors_vec);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GOptimizationAlgorithmT2()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A standard assignment operator
	  */
	 const GOptimizationAlgorithmT2<executor_type>& operator=(const GOptimizationAlgorithmT2<executor_type>& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another GOptimizationAlgorithmT2<executor_type> object
	  *
	  * @param  cp A constant reference to another GOptimizationAlgorithmT2<executor_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const GOptimizationAlgorithmT2<executor_type>& cp) const {
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
	  * Checks for inequality with another GOptimizationAlgorithmT2<executor_type> object
	  *
	  * @param  cp A constant reference to another GOptimizationAlgorithmT2<executor_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const GOptimizationAlgorithmT2<executor_type>& cp) const {
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
	  * Triggers saving of a checkpoint file dependimg on user-settings
	  *
	  * @param is_better A boolean which indicates whether a better result was found
	  */
	 void checkpoint(const bool& is_better) const {
		 // Determine a suitable name for the checkpoint file
		 bf::path output_file;
		 if(m_cp_overwrite) {
			 output_file = getCheckpointPath() /  bf::path("checkpoint_" + getCheckpointBaseName());
		 } else {
			 output_file = getCheckpointPath() / bf::path(
					(this->halted() ? "final" : boost::lexical_cast<std::string>(getIteration())) + "_" +
					boost::lexical_cast<std::string>(std::get<G_TRANSFORMED_FITNESS>(getBestKnownPrimaryFitness())) + "_" +
					getCheckpointBaseName()
			 );
		 }

		 // Save checkpoints if required by the user
		 if(m_cp_interval < 0 && is_better) {
			 saveCheckpoint(output_file);
		 } // Only save when a better solution was found
		 else if(m_cp_interval > 0 && m_iteration%m_cp_interval == 0) {
			 saveCheckpoint(output_file);
		 } // Save in regular intervals
		 else if(this->halted()) {
			 saveCheckpoint(output_file);
		 } // Save the final result
	 }

	 /***************************************************************************/
	 /**
	  * Loads the state of the class from disc
	  */
	 virtual void loadCheckpoint(const bf::path& cpFile) {
		 this->fromFile(cpFile, this->getCheckpointSerializationMode());
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether the optimization process has been halted, because the halt() function
	  * has returned "true"
	  *
	  * @return A boolean indicating whether the optimization process has been halted
	  */
	 bool halted() const {
		 return m_halted;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the number of generations after which a checkpoint should be written.
	  * A negative value will result in automatic check-pointing, whenever a better solution
	  * was found.
	  *
	  * @param cpInterval The number of generations after which a checkpoint should be written
	  */
	 void setCheckpointInterval(std::int32_t cpInterval) {
		 m_cp_interval = cpInterval;
	 }


	 /***************************************************************************/
	 /**
	  * Allows to retrieve the number of generations after which a checkpoint should be written
	  *
	  * @return The number of generations after which a checkpoint should be written
	  */
	 std::int32_t getCheckpointInterval() const {
		 return m_cp_interval;
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
				 << "In GOptimizationAlgorithmT2<executor_type>::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
				 << "Error: Invalid cpBaseName: " << cpBaseName << std::endl
				 << GEXCEPTION;
		 }

		 if(cpDirectory == "empty" || cpDirectory.empty()) {
			 glogger
				 << "In GOptimizationAlgorithmT2<executor_type>::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
				 << "Error: Invalid cpDirectory: " << cpDirectory << std::endl
				 << GEXCEPTION;
		 }

		 m_cp_base_name = cpBaseName;

		 // Check that the provided directory exists
		 if(!boost::filesystem::exists(cpDirectory)) {
			 glogger
				 << "In GOptimizationAlgorithmT2<executor_type>::setCheckpointBaseName(): Warning!" << std::endl
				 << "Directory " << cpDirectory << " does not exist and will be created automatically." << std::endl
				 << GWARNING;

			 if(!boost::filesystem::create_directory(cpDirectory)) {
				 glogger
					 << "In GOptimizationAlgorithmT2<executor_type>::setCheckpointBaseName(): Error!" << std::endl
					 << "Could not create directory " << cpDirectory << std::endl
					 << GEXCEPTION;
			 }
		 } else if(!boost::filesystem::is_directory(cpDirectory)) {
			 glogger
				 << "In GOptimizationAlgorithmT2<executor_type>::setCheckpointBaseName(): Error!" << std::endl
				 << cpDirectory << " exists but is no directory." << std::endl
				 << GEXCEPTION;
		 }

		 // Finally store the directory
		 m_cp_directory = cpDirectory;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the base name of the checkpoint file.
	  *
	  * @return The base name used for checkpoint files
	  */
	 std::string getCheckpointBaseName() const {
		 return m_cp_base_name;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the directory where checkpoint files should be stored
	  *
	  * @return The base name used for checkpoint files
	  */
	 bf::path getCheckpointPath() const {
		 return bf::path(m_cp_directory);
	 }

	 /***************************************************************************/
	 /**
	  * Determines whether checkpointing should be done in Text-, XML- or Binary-mode
	  *
	  * @param cpSerMode The desired new checkpointing serialization mode
	  */
	 void setCheckpointSerializationMode(Gem::Common::serializationMode cpSerMode) {
		 m_cp_serialization_mode = cpSerMode;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current checkpointing serialization mode
	  *
	  * @return The current checkpointing serialization mode
	  */
	 Gem::Common::serializationMode getCheckpointSerializationMode() const {
		 return m_cp_serialization_mode;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the m_cp_overwrite flag (determines whether checkpoint files
	  * should be overwritten or kept
	  */
	 void setKeepCheckpointFiles(bool cp_overwrite) {
		 m_cp_overwrite = cp_overwrite;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether checkpoint files will be overwritten
	  */
    bool checkpointFilesAreKept() const {
		 return m_cp_overwrite;
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

		 // Check that we are dealing with a GOptimizationAlgorithmT2<executor_type> reference independent of this object and convert the pointer
		 const GOptimizationAlgorithmT2<executor_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GOptimizationAlgorithmT2<executor_type>>(cp, this);

		 GToken token("GOptimizationAlgorithmT2<executor_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GObject>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(m_iteration, p_load->m_iteration), token);
		 compare_t(IDENTITY(m_offset, p_load->m_offset), token);
		 compare_t(IDENTITY(m_max_iteration, p_load->m_max_iteration), token);
		 compare_t(IDENTITY(m_min_iteration, p_load->m_min_iteration), token);
		 compare_t(IDENTITY(m_max_stall_iteration, p_load->m_max_stall_iteration), token);
		 compare_t(IDENTITY(m_report_iteration, p_load->m_report_iteration), token);
		 compare_t(IDENTITY(m_n_record_best_global_individuals, p_load->m_n_record_best_global_individuals), token);
		 compare_t(IDENTITY(m_best_globalIndividuals, p_load->m_best_globalIndividuals), token);
		 compare_t(IDENTITY(m_best_iteration_individuals, p_load->m_best_iteration_individuals), token);
		 compare_t(IDENTITY(m_default_population_size, p_load->m_default_population_size), token);
		 compare_t(IDENTITY(m_best_known_primary_fitness, p_load->m_best_known_primary_fitness), token);
		 compare_t(IDENTITY(m_best_current_primary_fitness, p_load->m_best_current_primary_fitness), token);
		 compare_t(IDENTITY(m_stall_counter, p_load->m_stall_counter), token);
		 compare_t(IDENTITY(m_stall_counter_threshold, p_load->m_stall_counter_threshold), token);
		 compare_t(IDENTITY(m_cp_interval, p_load->m_cp_interval), token);
		 compare_t(IDENTITY(m_cp_base_name, p_load->m_cp_base_name), token);
		 compare_t(IDENTITY(m_cp_directory, p_load->m_cp_directory), token);
		 compare_t(IDENTITY(m_cp_overwrite, p_load->m_cp_overwrite), token);
		 compare_t(IDENTITY(m_cp_serialization_mode, p_load->m_cp_serialization_mode), token);
		 compare_t(IDENTITY(m_quality_threshold, p_load->m_quality_threshold), token);
		 compare_t(IDENTITY(m_has_quality_threshold, p_load->m_has_quality_threshold), token);
		 compare_t(IDENTITY(m_max_duration.count(), p_load->m_max_duration.count()), token); // Cannot directly compare std::chrono::duration<double>
		 compare_t(IDENTITY(m_min_duration.count(), p_load->m_min_duration.count()), token); // Cannot directly compare std::chrono::duration<double>
		 compare_t(IDENTITY(m_termination_file, p_load->m_termination_file), token);
		 compare_t(IDENTITY(m_terminate_on_file_modification, p_load->m_terminate_on_file_modification), token);
		 compare_t(IDENTITY(m_emit_termination_reason, p_load->m_emit_termination_reason), token);
		 compare_t(IDENTITY(m_halted, p_load->m_halted), token);
		 compare_t(IDENTITY(m_worst_known_valids_vec, p_load->m_worst_known_valids_vec), token);
		 compare_t(IDENTITY(m_pluggable_monitors_vec, p_load->m_pluggable_monitors_vec), token);
		 compare_t(IDENTITY(this->data,  p_load->data), token); // Held in the parent class
		 compare_t(IDENTITY(this->m_executor, p_load->m_executor), token);

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
		 m_iteration = offset;

		 // Set the iteration offset
		 m_offset = offset;

		 // Let the algorithm know that the optimization process hasn't been halted yet
		 m_halted = false; // general halt criterion

		 // Store any *clean* individuals that have been added to this algorithm
		 // in the priority queue. This happens so that best individuals from a
		 // previous "chained" optimization run aren't lost.
		 addCleanStoredBests(m_best_globalIndividuals);

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
		 if(m_report_iteration) informationUpdate(infoMode::INFOINIT);

		 // We want to know if no better values were found for a longer period of time
		 double worstCase = this->getWorstCase();
		 m_best_known_primary_fitness   = std::make_tuple(worstCase, worstCase);
		 m_best_current_primary_fitness = std::make_tuple(worstCase, worstCase);

		 m_stall_counter = 0;

		 // Initialize the start time with the current time.
		 m_start_time = std::chrono::system_clock::now();

		 // Give derived classes the opportunity to perform any necessary preparatory work.
		 init();

		 do {
			 // Let all individuals know the current iteration
			 markIteration();

			 // Update fitness values and the stall counter
			 updateStallCounter((m_best_current_primary_fitness=cycleLogic()));

			 // Add the best individuals to the m_best_globalIndividuals
			 // and m_best_iteration_individuals vectors
			 updateGlobalBestsPQ(m_best_globalIndividuals);
			 updateIterationBestsPQ(m_best_iteration_individuals);

			 // Check whether a better value was found, and do the check-pointing, if necessary and requested.
			 checkpoint(progress());

			 // Let all individuals know about the best fitness known so far
			 markBestFitness();

			 // Let individuals know about the stalls encountered so far
			 markNStalls();

			 // Give derived classes an opportunity to act on stalls. NOTE that no action
			 // may be taken that affects the "dirty" state of individuals
			 if(m_stall_counter_threshold && stallCounterThresholdExceeded()) {
				 actOnStalls();
			 }

			 // We want to provide feedback to the user in regular intervals.
			 // Set the reportGeneration_ variable to 0 in order not to emit
			 // any information at all.
			 if(m_report_iteration && (m_iteration%m_report_iteration == 0)) {
				 informationUpdate(infoMode::INFOPROCESSING);
			 }

			 // update the m_iteration counter
			 m_iteration++;
		 }
		 while(!(m_halted = halt()));

		 // Give derived classes the opportunity to perform any remaining clean-up work
		 finalize();

		 // Finalize the info output
		 if(m_report_iteration) informationUpdate(infoMode::INFOEND);

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
	  * Emits information specific to this class (basic information in each iteration
	  * plus some user-defined information via pluggable optimization monitors)
	  *
	  * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
	  */
	 virtual void informationUpdate(const infoMode& im) BASE {
		 // Act on the information mode provided
		 switch(im) {
			 case Gem::Geneva::infoMode::INFOINIT:
				 std::cout << "Starting an optimization run with algorithm \"" << this->getAlgorithmName() << "\"" << std::endl;
				 break;

			 case Gem::Geneva::infoMode::INFOPROCESSING:
			 {
				 // We output raw values here, as this is likely what the user is interested in
				 std::cout
					 << std::setprecision(5)
					 << this->getIteration() << ": "
					 << Gem::Common::g_to_string(this->getBestCurrentPrimaryFitness())
					 << " // best past: " << Gem::Common::g_to_string(this->getBestKnownPrimaryFitness())
					 << std::endl;
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOEND:
				 std::cout << "End of optimization reached in algorithm \""<< this->getAlgorithmName() << "\"" << std::endl;
				 break;
		 };

		 // Perform any action defined by the user through pluggable monitor objects
		 for(auto pm_ptr: m_pluggable_monitors_vec) { // std::shared_ptr may be copied
			 pm_ptr->informationFunction(im, this);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether a better solution was found. If so, the stallCounter_
	  * variable will have been set to 0
	  *
	  * @return A boolean indicating whether a better solution was found
	  */
	 bool progress() const {
		 return (0==m_stall_counter);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a pluggable optimization monitor. Note that this
	  * function does NOT take ownership of the optimization monitor.
	  */
	 void registerPluggableOM(
		 std::shared_ptr<typename GOptimizationAlgorithmT2<executor_type>::GBasePluggableOMT> pluggableOM
	 ) {
		 if(pluggableOM) {
			 m_pluggable_monitors_vec.push_back(pluggableOM);
		 } else {
			 glogger
				 << "In GOptimizationMonitorT<>::registerPluggableOM(): Tried to register empty pluggable optimization monitor" << std::endl
				 << GEXCEPTION;
		 }
	 }

	 /************************************************************************/
	 /**
	  * Allows to reset the local pluggable optimization monitors
	  */
	 void resetPluggableOM() {
		 m_pluggable_monitors_vec.clear();
	 }

	 /******************************************************************************/
	 /**
 	 * Allows to check whether pluggable optimization monitors were registered
  	 */
	 bool hasPluggableOptimizationMonitors() const {
		 return !m_pluggable_monitors_vec.empty();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the default population size
	  *
	  * @return The default population size
	  */
	 std::size_t getDefaultPopulationSize() const {
		 return m_default_population_size;
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
		 // Check that the maximum number of iterations is > the minimum number
		 // The check is only valid if a maximum number of iterations has been set (i.e. is != 0)
		 if(m_max_iteration > 0 && m_max_iteration <= m_min_iteration) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::setMaxIteration(): Error!" << std::endl
				 << "Maximum number of iterations " << 	m_max_iteration << " is <= the minimum number " << m_min_iteration << std::endl
				 << GEXCEPTION;
		 }

		 m_max_iteration = maxIteration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of iterations after which optimization should
	  * be stopped
	  *
	  * @return The number of iterations after which the optimization should terminate
	  */
	 std::uint32_t getMaxIteration() const {
		 return m_max_iteration;
	 }

	 /***************************************************************************/
	 /**
 	 * This function checks whether a minimal number of iterations was reached.
  	 * No halt will be performed if this is not the case (with the exception of halts
  	 * that are triggered by user-actions, such as Ctrl-C (Sighup-Halt) and touched halt
  	 * (Geneva checks whether a file was modified after Geneva has started). Set the number
  	 * of iterations to 0 in order to disable a check for the minimal number of iterations.
	 */
	 void setMinIteration(std::uint32_t minIteration) {
		 // Check that the maximum number of iterations is > the minimum number
		 // The check is only valid if a maximum number of iterations has been set (i.e. is != 0)
		 if(m_max_iteration > 0 && m_max_iteration <= m_min_iteration) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::setMinIteration(): Error!" << std::endl
				 << "Maximum number of iterations " << 	m_max_iteration << " is <= the minimum number " << m_min_iteration << std::endl
				 << GEXCEPTION;
		 }

		 m_min_iteration = minIteration;
	 }

	 /***************************************************************************/
	 /**
	  * This function retrieves the value of the minIteration_ variable
	  */
	 std::uint32_t getMinIteration() const {
		 return m_min_iteration;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum number of generations allowed without improvement of the best
	  * individual. Set to 0 in order for this stop criterion to be disabled.
	  *
	  * @param The maximum number of allowed generations
	  */
	 void setMaxStallIteration(std::uint32_t maxStallIteration) {
		 m_max_stall_iteration = maxStallIteration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the maximum number of generations allowed in an optimization run without
	  * improvement of the best individual.
	  *
	  * @return The maximum number of generations
	  */
	 std::uint32_t getMaxStallIteration() const {
		 return m_max_stall_iteration;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum allowed processing time
	  *
	  * @param maxDuration The maximum allowed processing time
	  */
	 void setMaxTime(std::chrono::duration<double> maxDuration) {
		 if(!Gem::Common::isClose<double>(maxDuration.count(), 0.) && maxDuration < m_min_duration) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::setMaxTime(): Error!" << std::endl
				 << "Desired maxDuration (" << maxDuration.count() << " is smaller than m_min_duration(" << m_min_duration.count() << ")" << std::endl
				 << GEXCEPTION;
		 }

		 m_max_duration = maxDuration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the maxDuration_ parameter.
	  *
	  * @return The maximum allowed processing time
	  */
	 std::chrono::duration<double> getMaxTime() const {
		 return m_max_duration;
	 }

	 /***************************************************************************/
	 /**
	 * Sets the minimum required processing time. NOTE: Always set the maximum duration
	 * before the minimum duration.
	 *
	 * @param minDuration The minimum allowed processing time
	 */
	 void setMinTime(std::chrono::duration<double> minDuration) {
		 if(!Gem::Common::isClose<double>(m_max_duration.count(),0.) && m_max_duration < minDuration) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::setMinTime(): Error!" << std::endl
				 << "Desired maxDuration (" << m_max_duration.count() << " is smaller than m_min_duration(" << minDuration.count() << ")" << std::endl
				 << GEXCEPTION;
		 }

		 m_min_duration = minDuration;
	 }

	 /***************************************************************************/
	 /**
	 * Retrieves the value of the minDuration_ parameter.
	 *
	 * @return The minimum required processing time
	 */
	 std::chrono::duration<double> getMinTime() const {
		 return m_min_duration;
	 }

	 /***************************************************************************/
	 /**
	  *  Sets a quality threshold beyond which optimization is expected to stop
	  *
	  *  @param qualityThreshold A threshold beyond which optimization should stop
	  *  @param hasQualityThreshold Allows to (de-)activate the quality threshold
	  */
	 void setQualityThreshold(double qualityThreshold, bool hasQualityThreshold) {
		 m_quality_threshold = qualityThreshold;
		 m_has_quality_threshold=hasQualityThreshold;
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
		 hasQualityThreshold = m_has_quality_threshold;
		 return m_quality_threshold;
	 }

	 /***************************************************************************/
	 /**
	  *  Sets the name of a "termination file" (optimization is supposed to
	  *  stop when the modification time of this file is more recent than the
	  *  start of the optimization run.
	  *
	  *  @param terminationFile The name of a file used to initiate termination
	  *  @param hasQualityThreshold Allows to (de-)activate "touched termination"
	  */
	 void setTerminationFile(std::string terminationFile, bool terminateOnFileModification) {
		 m_termination_file = terminationFile;
		 m_terminate_on_file_modification = terminateOnFileModification;
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
		 terminateOnFileModification = m_terminate_on_file_modification;
		 return m_termination_file;
	 }

	 /***************************************************************************/
	 /**
	  * Removes the quality threshold
	  */
	 void resetQualityThreshold() {
		 m_has_quality_threshold = false;
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether a quality threshold has been set
	  *
	  * @return A boolean indicating whether a quality threshold has been set
	  */
	 bool hasQualityThreshold() const {
		 return m_has_quality_threshold;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the current iteration of the optimization run
	  *
	  * @return The current iteration of the optimization run
	  */
	 virtual std::uint32_t getIteration() const override {
		 return m_iteration;
	 }

	 /***************************************************************************/
	 /**
	  * Returns the current offset used to calculate the current iteration. This
	  * is identical to the iteration the optimization starts with.
	  *
	  * @return The current iteration offset
	  */
	 std::uint32_t getStartIteration() const {
		 return m_offset;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the number of iterations after which the algorithm should
	  * report about its inner state.
	  *
	  * @param iter The number of iterations after which information should be emitted
	  */
	 void setReportIteration(std::uint32_t iter) {
		 m_report_iteration = iter;
	 }

	 /***************************************************************************/
	 /**
	  * Returns the number of iterations after which the algorithm should
	  * report about its inner state.
	  *
	  * @return The number of iterations after which information is emitted
	  */
	 std::uint32_t getReportIteration() const {
		 return m_report_iteration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current number of failed optimization attempts
	  *
	  * @return The current number of failed optimization attempts
	  */
	 std::uint32_t getStallCounter() const {
		 return m_stall_counter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the number of iterations without improvement, after which
	  * individuals are asked to update their internal data structures
	  */
	 void setStallCounterThreshold(std::uint32_t stallCounterThreshold) {
		 m_stall_counter_threshold = stallCounterThreshold;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the number of iterations without improvement, after which
	  * individuals are asked to update their internal data structures
	  */
	 std::uint32_t getStallCounterThreshold() const {
		 return m_stall_counter_threshold;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the best value found in the entire optimization run so far
	  *
	  * @return The best raw and transformed fitness found so far
	  */
	 std::tuple<double, double> getBestKnownPrimaryFitness() const {
		 return (m_best_globalIndividuals.best())->getFitnessTuple();

		 // return m_best_known_primary_fitness;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best value found in the current iteration
	  *
	  * @return The best raw and transformed fitness found in the current iteration
	  */
	 std::tuple<double, double> getBestCurrentPrimaryFitness() const {
		 return m_best_current_primary_fitness;
	 }

	 /***************************************************************************/
	 /**
	  * Specifies whether information about termination reasons should be emitted
	  *
	  * @param etr A boolean which specifies whether reasons for the termination of the optimization run should be emitted
	  */
	 void setEmitTerminationReason(bool emitTerminationReason = true) {
		 m_emit_termination_reason = emitTerminationReason;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves information on whether information about termination reasons should be emitted
	  *
	  * @return A boolean which specifies whether reasons for the termination of the optimization run will be emitted
	  */
	 bool getEmitTerminationReason() const {
		 return m_emit_termination_reason;
	 }

	 /***************************************************************************/
	 /**
	  * This function converts an individual at a given position to the derived
	  * type and returns it. In DEBUG mode, the function will check whether the
	  * requested position exists.
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
				 << "In GOptimizationAlgorithmT2<executor_type>::individual_cast<>() : Error" << std::endl
				 << "Tried to access position " << pos << " which is >= array size " << this->size() << std::endl
				 << GEXCEPTION;

			 // Make the compiler happy
			 return std::shared_ptr<target_type>();
		 }
#endif /* DEBUG */

		 // Does error checks on the conversion internally
		 return Gem::Common::convertSmartPointer<GParameterSet, target_type>(this->at(pos));
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
	  * Adds local configuration options to a GParserBuilder object
	  *
	  * @param gpb The GParserBuilder object to which configuration options should be added
	  */
	 virtual void addConfigurationOptions (
		 Gem::Common::GParserBuilder& gpb
	 ) override {
		 // Call our parent classes' functions
		 GObject::addConfigurationOptions(gpb);

		 // Add local data
		 gpb.registerFileParameter<std::uint32_t>(
			 "maxIteration" // The name of the variable
			 , DEFAULTMAXIT // The default value
			 , [this](std::uint32_t maxIt){ this->setMaxIteration(maxIt); }
		 )
			 << "The maximum allowed number of iterations";

		 gpb.registerFileParameter<std::uint32_t>(
			 "minIteration" // The name of the variable
			 , DEFAULTMINIT // The default value
			 , [this](std::uint32_t minIt){ this->setMinIteration(minIt); }
		 )
			 << "The minimum allowed number of iterations";

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
			 << "optimization run, instructs Geneva to terminate optimization." << std::endl
			 << "This can be used to \"touch a file\" after the start of an optimization" << std::endl
			 << "run, which will lead to the termination of the run after the current iteration." << Gem::Common::nextComment()
			 << "Activates (1) or de-activates (0) the \"touched termination\"";

		 gpb.registerFileParameter<std::uint32_t>(
			 "individualUpdateStallCounterThreshold" // The name of the variable
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

		 gpb.registerFileParameter<std::int32_t>(
			 "cp_pverwrite" // The name of the variable
			 , false // The default value
			 , [this](bool cp_overwrite){ this->setKeepCheckpointFiles(cp_overwrite); }
		 )
			 << "Determines whether checkpoint files should be overwritten or kept" << std::endl;

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

		 gpb.registerFileParameter<std::string>(
			 "maxDuration" // The name of the variable
			 , DEFAULTDURATION // The default value
			 , [this](std::string mt_str){ this->setMaxTime(Gem::Common::duration_from_string(mt_str)); }
		 )
			 << "The maximum allowed time-frame for the optimization" << std::endl
			 << "in the format hours:minutes:seconds";

		 gpb.registerFileParameter<std::string>(
			 "minDuration" // The name of the variable
			 , DEFAULTMINDURATION // The default value
			 , [this](std::string mt_str){ this->setMinTime(Gem::Common::duration_from_string(mt_str)); }
		 )
			 << "The minimum required time-frame for the optimization" << std::endl
			 << "in the format hours:minutes:seconds";

		 gpb.registerFileParameter<bool>(
			 "emitTerminationReason" // The name of the variable
			 , DEFAULTEMITTERMINATIONREASON // The default value
			 , [this](bool etr){ this->setEmitTerminationReason(etr); }
		 )
			 << "Triggers emission (1) or omission (0) of information about reasons for termination";
	 }

	 /***************************************************************************/
	 /**
	  * Adds the best individuals of each iteration to a priority queue. The
	  * queue will be sorted by the first evaluation criterion of the individuals
	  * and may either have a limited or unlimited size.
	  */
	 virtual void updateGlobalBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {
		 const bool CLONE = true;
		 const bool DONOTREPLACE = false;

#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In GOptimizationAlgorithmT2<executor_type>::updateGlobalBestsPQ() :" << std::endl
				 << "Tried to retrieve the best individuals even though the population is empty." << std::endl
				 << GEXCEPTION;
		 }
#endif /* DEBUG */

		 // We simply add all individuals to the queue -- only the best ones will actually be added (and cloned)
		 // Unless we have asked for the queue to have an unlimited size, the queue will be resized as required
		 // by its maximum allowed size.
		 bestIndividuals.add(this->data, CLONE, DONOTREPLACE);
	 }

	 /***************************************************************************/
	 /**
	  * Adds the best individuals of each iteration to a priority queue, which is cleared
	  * along the way, so results are only relevant for a given iteration.
	  */
	 virtual void updateIterationBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {
		 const bool CLONE = true;
		 const bool REPLACE = true;

#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In GOptimizationAlgorithmT2<executor_type>::updateIterationBestsPQ() :" << std::endl
				 << "Tried to retrieve the best individuals even though the population is empty." << std::endl
				 << GEXCEPTION;
		 }
#endif /* DEBUG */

		 // We simply add all individuals to the queue. They will automatically be sorted.
		 bestIndividuals.add(this->data, CLONE, REPLACE);
	 }

	 /***************************************************************************/
	 /**
	  * If individuals have been stored in this population, they are added to the
	  * priority queue. This happens before the optimization cycle starts, so that
	  * best individuals from a previous "chained" optimization run aren't lost.
	  * Only those individuals are stored in the priority queue that do not have the
	  * "dirty flag" set.
	  */
	 virtual void addCleanStoredBests(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {	const bool CLONE = true;

		 // We simply add all *clean* individuals to the queue -- only the best ones will actually be added
		 // (and cloned) Unless we have asked for the queue to have an unlimited size, the queue will be
		 // resized as required by its maximum allowed size.
		 for(auto item: this->data) {
			 if(item->isClean()) {
				 bestIndividuals.add(item, CLONE);
			 }
		 }
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
	 virtual bool isBetter(double newValue, const double& oldValue) const {
#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::isBetter(): Error!" << std::endl
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
	 virtual bool isWorse(double newValue, const double& oldValue) const {
#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::isWorse(): Error!" << std::endl
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
	 virtual double getWorstCase() const {
#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::getWorstCase(): Error!" << std::endl
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
	 virtual double getBestCase() const {
#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In GOptimizationAlgorithmT2<>::getBestCase(): Error!" << std::endl
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
		 return m_iteration == m_offset;
	 }

	 /***************************************************************************/
	 /**
	  * A little helper function that determines whether we are after the first iteration
	  *
	  * @return A boolean indicating whether we are after the first iteration
	  */
	 bool afterFirstIteration() const {
		 return m_iteration > m_offset;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads the data of another GOptimizationAlgorithm object
	  *
	  * @param cp Another GOptimizationAlgorithm object, camouflaged as a GObject
	  */
	 virtual void load_(const GObject* cp) override {
		 // Check that we are dealing with a GOptimizationAlgorithmT2<executor_type> reference independent of this object and convert the pointer
		 const GOptimizationAlgorithmT2<executor_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GOptimizationAlgorithmT2<executor_type>>(cp, this);

		 // Load the parent class'es data
		 GObject::load_(cp);
		 Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::operator=(*p_load);

		 // and then our local data
		 m_iteration = p_load->m_iteration;
		 m_offset = p_load->m_offset;
		 m_max_iteration = p_load->m_max_iteration;
		 m_min_iteration = p_load->m_min_iteration;
		 m_max_stall_iteration = p_load->m_max_stall_iteration;
		 m_report_iteration = p_load->m_report_iteration;
		 m_n_record_best_global_individuals = p_load->m_n_record_best_global_individuals;
		 m_best_globalIndividuals = p_load->m_best_globalIndividuals;
		 m_best_iteration_individuals = p_load->m_best_iteration_individuals;
		 m_default_population_size = p_load->m_default_population_size;
		 m_best_known_primary_fitness = p_load->m_best_known_primary_fitness;
		 m_best_current_primary_fitness = p_load->m_best_current_primary_fitness;
		 m_stall_counter = p_load->m_stall_counter;
		 m_stall_counter_threshold = p_load->m_stall_counter_threshold;
		 m_cp_interval = p_load->m_cp_interval;
		 m_cp_base_name = p_load->m_cp_base_name;
		 m_cp_directory = p_load->m_cp_directory;
		 m_cp_overwrite = p_load->m_cp_overwrite;
		 m_cp_serialization_mode = p_load->m_cp_serialization_mode;
		 m_quality_threshold = p_load->m_quality_threshold;
		 m_has_quality_threshold = p_load->m_has_quality_threshold;
		 m_termination_file = p_load->m_termination_file;
		 m_terminate_on_file_modification = p_load->m_terminate_on_file_modification;
		 m_max_duration = p_load->m_max_duration;
		 m_min_duration = p_load->m_min_duration;
		 m_emit_termination_reason = p_load->m_emit_termination_reason;
		 m_halted = p_load->m_halted;
		 m_worst_known_valids_vec = p_load->m_worst_known_valids_vec;
		 Gem::Common::copyCloneableSmartPointerContainer(p_load->m_pluggable_monitors_vec, m_pluggable_monitors_vec);
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
		 std::shared_ptr<GParameterSet> p = m_best_globalIndividuals.best();
		 if(p) return p;
		 else {
			 glogger
				 << "In GOptimizationAlgorithmT2<executor_type>::customGetBestGlobalIndividual(): Error!" << std::endl
				 << "Best individual seems to be empty" << std::endl
				 << GEXCEPTION;

			 // Make the compiler happy
			 return std::shared_ptr<GParameterSet>();
		 }
#else
		 return m_best_globalIndividuals.best();
#endif
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found (equal to the content of
	  * the priority queue)
	  */
	 virtual std::vector<std::shared_ptr<GParameterSet>> customGetBestGlobalIndividuals() override {
		 return m_best_globalIndividuals.toVector();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual found in the iteration (which is the best individual
	  * in the priority queue).
	  */
	 virtual std::shared_ptr<GParameterSet> customGetBestIterationIndividual() override {
#ifdef DEBUG
		 std::shared_ptr<GParameterSet> p = m_best_iteration_individuals.best();
		 if(p) return p;
		 else {
			 glogger
				 << "In GOptimizationAlgorithmT2<executor_type>::customGetBestIterationIndividual(): Error!" << std::endl
				 << "Best individual seems to be empty" << std::endl
				 << GEXCEPTION;

			 // Make the compiler happy
			 return std::shared_ptr<GParameterSet>();
		 }
#else
		 return m_best_iteration_individuals.best();
#endif
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found in the iteration (equal to the content of
	  * the priority queue)
	  */
	 virtual std::vector<std::shared_ptr<GParameterSet>> customGetBestIterationIndividuals() override {
		 return m_best_iteration_individuals.toVector();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the personality type of the individuals
	  */
	 virtual void setIndividualPersonalities() {
		 for(auto item: this->data) {
			 item->setPersonality(this->getPersonalityTraits());
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Resets the individual's personality types
	  */
	 void resetIndividualPersonalities() {
		 for(auto item: this->data) {
			 item->resetPersonality();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Saves the state of the class to disc
	  */
	 virtual void saveCheckpoint(bf::path outputFile) const {
		 this->toFile(outputFile, this->getCheckpointSerializationMode());
	 }

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
		 m_default_population_size = defPopSize;
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
				 << "In GOptimizationAlgorithmT2<>::setNRecordBestIndividuals(): Error!" << std::endl
				 << "Invalid number of individuals to be recorded: " << nRecordBestIndividuals << std::endl
				 << GEXCEPTION;
		 }

		 m_n_record_best_global_individuals = nRecordBestIndividuals;
		 m_best_globalIndividuals.setMaxSize(m_n_record_best_global_individuals);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of best individuals to be recorded in each iteration
	  *
	  * @return The number of best individuals to be recorded in each iteration
	  */
	 std::size_t getNRecordBestIndividuals() const {
		 return m_n_record_best_global_individuals;
	 }

	 /***************************************************************************/
	 /**
	  * It is possible for derived classes to specify in overloaded versions of this
	  * function under which conditions the optimization should be stopped. The
	  * function is called from GOptimizationAlgorithmT2<executor_type>::halt .
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
	  * Allows derived classes to reset the stall counter.
	  */
	 void resetStallCounter() {
		 m_stall_counter = 0;
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
		 for(auto item: this->data) {
			 item->setAssignedIteration(m_iteration);
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
		 typename GOptimizationAlgorithmT2<executor_type>::iterator it;
		 std::size_t nFitnessCriteria = (*(this->begin()))->getNumberOfFitnessCriteria();

		 // Is this the first call ? Fill m_worst_known_valids_vec with data
		 if(inFirstIteration()) {
			 for(auto item: this->data) {
				 item->populateWorstKnownValid();
			 }

			 // Initialize our own, local m_worst_known_valids_vec
			 m_worst_known_valids_vec = (*(this->begin()))->getWorstKnownValids();
		 }

		 for(it=this->begin(); it!=this->end(); ++it) {
#ifdef DEBUG
			 if((*it)->getNumberOfFitnessCriteria() != nFitnessCriteria) {
				 glogger
					 << "In GOptimizationAlgorithmT2<>::updateWorstKnownValid(): Error!" << std::endl
					 << "Got " << (*it)->getNumberOfFitnessCriteria() << " fitness criteria in individual " << (it-this->begin()) << std::endl
					 << "but expected " << nFitnessCriteria << " criteria" << std::endl
					 << GEXCEPTION;
			 }

			 if(!m_worst_known_valids_vec.empty() && m_worst_known_valids_vec.size() != nFitnessCriteria) {
				 glogger
					 << "In GOptimizationAlgorithmT2<>::updateWorstKnownValid(): Error!" << std::endl
					 << "Got invalid number of evaluation criteria in m_worst_known_valids_vec:" << std::endl
					 << "Got " << m_worst_known_valids_vec.size() << " but expected " << nFitnessCriteria << std::endl
					 << GEXCEPTION;
			 }
#endif /* DEBUG */

			 if((*it)->isClean() && (*it)->isValid()) { // Is this an individual which has been evaluated and fulfills all constraints ?
				 for(std::size_t id=0; id<nFitnessCriteria; id++) {
					 (*it)->challengeWorstValidFitness(m_worst_known_valids_vec.at(id), id);
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
		 for(auto item: this->data) {
			 item->setWorstKnownValid(m_worst_known_valids_vec);
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Triggers an update of the individual's evaluation (e.g. in order to
	  * act on the information regarding best or worst evaluations found
	  */
	 void triggerEvaluationUpdate() {
		 for(auto item: this->data) {
			 item->postEvaluationUpdate();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Work to be performed right after the individuals were evaluated. NOTE:
	  * this setup is sub-optimal, as this function isn't called from within
	  * GOptimizationAlgorithmT2 directly, but only from derived classes. This happens
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
		 for(auto item: this->data) {
			 item->setNStalls(m_stall_counter);
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
	 /**
	  * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	  * Make the vector wrapper purely virtual allows the compiler to perform
	  * further optimizations.
	  */
	 virtual void dummyFunction() override { /* nothing */ }

	 /***************************************************************************/

	 std::uniform_real_distribution<double> m_uniform_real_distribution; ///< Access to uniformly distributed double random values
	 std::uniform_int_distribution<std::size_t> m_uniform_int_distribution; ///< Access to uniformly distributed random numbers

	 Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> m_gr; ///< A random number generator. Note that the actual calculation is done in a random number proxy / factory
	 executor_type m_executor; ///< Takes care of the evaluation of objects

private:
	 /***************************************************************************/
	 /**
	  * Update the stall counter. We use the transformed fitness for comparison
	  * here, so we can usually deal with finite values (due to the transformation
	  * in the case of a constraint violation).
	  */
	 void updateStallCounter(const std::tuple<double, double>& bestEval) {
		 if(this->isBetter(std::get<G_TRANSFORMED_FITNESS>(bestEval), std::get<G_TRANSFORMED_FITNESS>(m_best_known_primary_fitness))) {
			 m_best_known_primary_fitness = bestEval;
			 m_stall_counter = 0;
		 } else {
			 m_stall_counter++;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * This function returns true once a given time (set with
	  * GOptimizationAlgorithm<executor_type>::setMaxTime()) has passed.
	  * It is used in the GOptimizationAlgorithmT2<executor_type>::halt() function.
	  *
	  * @return A boolean indicating whether a given amount of time has passed
	  */
	 bool timedHalt(const std::chrono::system_clock::time_point& currentTime) const {
		 if((currentTime - m_start_time) >= m_max_duration) {
			 if(m_emit_termination_reason) {
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
	  * This function checks whether a minimum amount of time has passed
	  */
	 bool minTimePassed(const std::chrono::system_clock::time_point& currentTime) const {
		 return ((currentTime - m_start_time) > m_min_duration);
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
		 if(this->isBetter(std::get<G_RAW_FITNESS>(m_best_known_primary_fitness), m_quality_threshold)) {
			 if(m_emit_termination_reason) {
				 glogger
					 << "Terminating optimization run because" << std::endl
					 << "quality threshold " << m_quality_threshold << " has been exceeded." << std::endl
					 << "Best untransformed quality found was " << std::get<G_RAW_FITNESS>(m_best_known_primary_fitness) << std::endl
					 << "with termination in iteration " << m_iteration << std::endl
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
		 if(m_stall_counter > m_max_stall_iteration) {
			 if(m_emit_termination_reason) {
				 std::cout
					 << "Terminating optimization run because" << std::endl
					 << "maximum number of stalls " << m_max_stall_iteration << " has been exceeded." << std::endl
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
		 if(m_iteration >= (m_max_iteration + m_offset)) {
			 if(m_emit_termination_reason) {
				 std::cout
					 << "Terminating optimization run because" << std::endl
					 << "iteration threshold " << m_max_iteration << " has been exceeded." << std::endl;
			 }

			 return true;
		 } else {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * This function returns true when the minimum number of iterations has
	  * been passed.
	  */
	 bool minIterationPassed() const {
		 return (m_iteration > m_min_iteration);
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
		 else {
			 return false;
		 }
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
		 bf::path p(m_termination_file);

		 // Return if the file doesn't exist
		 if(!bf::exists(p)) {
			 return false;
		 }

		 // Determine the modification time of the file
		 std::time_t t = boost::filesystem::last_write_time(p);
		 std::chrono::system_clock::time_point modTime = std::chrono::system_clock::from_time_t(t);

		 // Check if the file was modified after the start of the optimization run
		 if(modTime > m_start_time) {
			 if(m_emit_termination_reason) {
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
			 if(m_emit_termination_reason) {
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
		 // Retrieve the current time, so all time-based functions act on the same basis
		 std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();

		 //------------------------------------------------------------------------
		 // The following halt criteria are triggered by the user. They override
		 // all other (automatic) criteria

		 // Have we received a SIGHUP signal ?
		 if(sigHupHalt()) return true;

		 // Are we supposed to stop when a file was modified after the start of the optimization run ?
		 if(m_terminate_on_file_modification && touchHalt()) return true;

		 //------------------------------------------------------------------------
		 // With the exception of the above criteria, no other halt criterion will
		 // have an effect unless some minimum criteria have been met. E.g., if the
		 // minimum number of iterations, as defined by the user, hasn't been passed,
		 // the optimization will continue (no matter whether e.g. the optimization
		 // has stalled for a given number of times).

		 // Has the minimum number of iterations, as defined by the user, been passed?
		 if(!minIterationPassed()) return false;

		 // Has the minimum required optimization time been passed?
		 if(!minTimePassed(currentTime)) return false;

		 //------------------------------------------------------------------------
		 // The following halt criteria are evaluated by Geneva at run-time,
		 // without any user-interaction.

		 // Have we exceeded the maximum number of iterations and
		 // do we indeed intend to stop in this case ?
		 if(maxIterationHaltSet() && iterationHalt()) return true;

		 // Has the optimization stalled too often ?
		 if(stallHaltSet() && stallHalt()) return true;

		 // Do we have a scheduled halt time ? The comparatively expensive
		 // timedHalt() calculation is only called if m_max_duration
		 // is at least one microsecond.
		 if(maxDurationHaltSet() && timedHalt(currentTime)) return true;

		 // Are we supposed to stop when the quality has exceeded a threshold ?
		 if(qualityThresholdHaltSet() && qualityHalt()) return true;

		 // Has the user specified an additional stop criterion ?
		 return customHalt_();
	 }

	 /***************************************************************************/
	 /**
	  * Check whether the max-iteration halt is set
	  *
	  * @return A boolean indicating whether the "max-iteration halt" has been set
	  */
	 bool maxIterationHaltSet() const {
		 return (m_max_iteration > 0);
	 }

	 /***************************************************************************/
	 /**
	  * Check whether a halt criterion based on the number of stalls has been set
	  *
	  * @return A boolean indicating whether a halt criterion based on the number of stalls has been set
	  */
	 bool stallHaltSet() const {
		 return (m_max_stall_iteration > 0);
	 }

	 /***************************************************************************/
	 /**
	  * Check whether the maxDuration-halt criterion has been set
	  *
	  * @return A boolean indication whether the max-duration halt criterion has been set
	  */
	 bool maxDurationHaltSet() const {
		 return (m_max_duration.count() > 0.);
	 }

	 /***************************************************************************/
	 /**
	  * Check whether the quality-threshold halt-criterion has been set
	  *
	  * @return A boolean indicating whether the quality-threshold halt-criterion has been set
	  */
	 bool qualityThresholdHaltSet() const {
		 return m_has_quality_threshold;
	 }

	 /***************************************************************************/
	 /**
	  * Marks the globally best known fitness in all individuals
	  */
	 void markBestFitness() {
		 for(auto item: this->data) {
			 item->setBestKnownPrimaryFitness(this->getBestKnownPrimaryFitness());
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Indicates whether the stallCounterThreshold_ has been exceeded
	  */
	 bool stallCounterThresholdExceeded() const {
		 return (m_stall_counter > m_stall_counter_threshold);
	 }

	 /***************************************************************************/

	 std::uint32_t m_iteration = 0; ///< The current iteration
	 std::uint32_t m_offset = DEFAULTOFFSET; ///< An iteration offset which can be used, if the optimization starts from a checkpoint file
	 std::uint32_t m_min_iteration = DEFAULTMINIT; ///< The minimum number of iterations
	 std::uint32_t m_max_iteration = DEFAULTMAXIT; ///< The maximum number of iterations
	 std::uint32_t m_max_stall_iteration = DEFAULTMAXSTALLIT; ///< The maximum number of generations without improvement, after which optimization is stopped
	 std::uint32_t m_report_iteration = DEFAULTREPORTITER; ///< The number of generations after which a report should be issued

	 std::size_t m_n_record_best_global_individuals = DEFNRECORDBESTINDIVIDUALS; ///< Indicates the number of best individuals to be recorded/updated in each iteration
	 GParameterSetFixedSizePriorityQueue m_best_globalIndividuals; ///< A priority queue with the best individuals found so far
	 GParameterSetFixedSizePriorityQueue m_best_iteration_individuals; ///< A priority queue with the best individuals of a given iteration

	 std::size_t m_default_population_size = DEFAULTPOPULATIONSIZE; ///< The nominal size of the population
	 std::tuple<double, double> m_best_known_primary_fitness; ///< Records the best primary fitness found so far
	 std::tuple<double, double> m_best_current_primary_fitness; ///< Records the best fitness found in the current iteration

	 std::uint32_t m_stall_counter = 0; ///< Counts the number of iterations without improvement
	 std::uint32_t m_stall_counter_threshold = DEFAULTSTALLCOUNTERTHRESHOLD; ///< The number of stalls after which individuals are asked to update their internal data structures

	 std::int32_t m_cp_interval = DEFAULTCHECKPOINTIT; ///< Number of iterations after which a checkpoint should be written. -1 means: Write whenever an improvement was encountered
	 std::string m_cp_base_name = DEFAULTCPBASENAME; ///< The base name of the checkpoint file
	 std::string m_cp_directory = DEFAULTCPDIR; ///< The directory where checkpoint files should be stored
	 bool m_cp_overwrite = true; ///< Whether checkpoint files should be overwritten or kept
	 Gem::Common::serializationMode m_cp_serialization_mode = DEFAULTCPSERMODE; ///< Determines whether check-pointing should be done in text-, XML, or binary mode

	 double m_quality_threshold = DEFAULTQUALITYTHRESHOLD; ///< A threshold beyond which optimization is expected to stop
	 bool m_has_quality_threshold = false; ///< Specifies whether a qualityThreshold has been set
	 std::chrono::duration<double> m_max_duration = Gem::Common::duration_from_string(DEFAULTDURATION); ///< Maximum time-frame for the optimization
	 std::chrono::duration<double> m_min_duration = Gem::Common::duration_from_string(DEFAULTMINDURATION); ///< Minimum time-frame for the optimization
	 mutable std::chrono::system_clock::time_point m_start_time; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
	 std::string m_termination_file = DEFAULTTERMINATIONFILE; ///< The name of a file which, when modified after the start of the optimization run, will cause termination of the run
	 bool m_terminate_on_file_modification = false;
	 bool m_emit_termination_reason = DEFAULTEMITTERMINATIONREASON; ///< Specifies whether information about reasons for termination should be emitted
	 bool m_halted = false; ///< Set to true when halt() has returned "true"
	 std::vector<std::tuple<double, double>> m_worst_known_valids_vec; ///< Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: transformed)
	 std::vector<std::shared_ptr<typename Gem::Geneva::GOptimizationAlgorithmT2<executor_type>::GBasePluggableOMT>> m_pluggable_monitors_vec; ///< A collection of monitors

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

		 // Call the parent classes' functions
		 if(GObject::modify_GUnitTests()) result = true;
		 if(Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::modify_GUnitTests()) result = true;

		 // Try to change the objects contained in the collection
		 for(auto item: this->data) {
			 if(item->modify_GUnitTests()) result = true;
		 }

		 this->setMaxIteration(this->getMaxIteration() + 1);
		 result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GOptimizationAlgorithmT2<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent classes' functions
		 GObject::specificTestsNoFailureExpected_GUnitTests();
		 Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GOptimizationAlgorithmT2<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 // Call the parent classes' functions
		 GObject::specificTestsFailuresExpected_GUnitTests();
		 Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("GOptimizationAlgorithmT2<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
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
			  const typename GOptimizationAlgorithmT2<executor_type>::GBasePluggableOMT& cp
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
		  virtual bool operator==(const typename GOptimizationAlgorithmT2<executor_type>::GBasePluggableOMT& cp) const {
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
		  virtual bool operator!=(const typename GOptimizationAlgorithmT2<executor_type>::GBasePluggableOMT& cp) const {
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
			  , GOptimizationAlgorithmT2<executor_type> * const goa
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

			  this->setUseRawEvaluation(!this->getUseRawEvaluation());
			  result = true;

			  return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
			  condnotset("GOptimizationAlgorithmT2<>::GBasePluggableOMT::modify_GUnitTests", "GEM_TESTING");
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
			  condnotset("GOptimizationAlgorithmT2<>::GBasePluggableOMT::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
			  condnotset("GOptimizationAlgorithmT2<>::GBasePluggableOMT::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
		  }

		  /************************************************************************/
	 };

	 /***************************************************************************/
	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {
template<typename executor_type>
struct is_abstract<Gem::Geneva::GOptimizationAlgorithmT2<executor_type>> : public boost::true_type {};
template<typename executor_type>
struct is_abstract< const Gem::Geneva::GOptimizationAlgorithmT2<executor_type>> : public boost::true_type {};
}
}

/******************************************************************************/

//BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GBrokerExecutorT<Gem::Geneva::GParameterSet>)
//BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GSerialExecutorT<Gem::Geneva::GParameterSet>)
//BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GMTExecutorT<Gem::Geneva::GParameterSet>)

/******************************************************************************/

#endif /* GOptimizationAlgorithmT2_HPP_ */
