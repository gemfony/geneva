/**
 * @file G_OA_BaseT.hpp
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
#include <type_traits>
#include <utility>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifndef GOPTIMIZATIONALGORITHMT_HPP_
#define GOPTIMIZATIONALGORITHMT_HPP_

// Geneva headers go here
#include "common/GStdPtrVectorInterfaceT.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GSerializationHelperFunctionsT.hpp"
#include "common/GPlotDesigner.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/G_Interface_Optimizer.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GPersonalityTraits.hpp"
#include "geneva/GParameterSetFixedSizePriorityQueue.hpp"

namespace Gem {
namespace Geneva {

/*******************************************************************************/
/////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************/
/**
 * The base class of all pluggable optimization monitors
 */
template <typename oa_type>
class GBasePluggableOMT : public GObject
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
		 & BOOST_SERIALIZATION_NVP(m_useRawEvaluation);
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
		 const GBasePluggableOMT<oa_type>& cp
	 )
		 : m_useRawEvaluation(cp.m_useRawEvaluation)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The Destructor
	  */
	 virtual ~GBasePluggableOMT()
	 { /* nothing */ }

	 /************************************************************************/
	 /**
	  * Checks for equality with another GBasePluggableOMT<oa_type> object
	  *
	  * @param  cp A constant reference to another GBasePluggableOMT<oa_type> object
	  * @return A boolean indicating whether both objects are equal
	  */
	 virtual bool operator==(const GBasePluggableOMT<oa_type>& cp) const {
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
	  * Checks for inequality with another GBasePluggableOMT<oa_type> object
	  *
	  * @param  cp A constant reference to another GBasePluggableOMT<oa_type> object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 virtual bool operator!=(const GBasePluggableOMT<oa_type>& cp) const {
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

		 // Check that we are dealing with a GBasePluggableOMT<oa_type> reference independent of this object and convert the pointer
		 const GBasePluggableOMT<oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GBasePluggableOMT<oa_type>>(cp, this);

		 GToken token("GBasePluggableOMT<oa_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GObject>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_useRawEvaluation, p_load->m_useRawEvaluation), token);

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
		 , oa_type * const goa
	 ) BASE = 0;

	 /***************************************************************************/
	 /**
	  * Allows to set the m_useRawEvaluation variable
	  */
	 void setUseRawEvaluation(bool useRaw) {
		 m_useRawEvaluation = useRaw;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the value of the m_useRawEvaluation variable
	  */
	 bool getUseRawEvaluation() const {
		 return m_useRawEvaluation;
	 }

protected:
	 /************************************************************************/
	 /**
	  * Loads the data of another object
	  *
	  * cp A pointer to another GBasePluggableOMT<oa_type> object, camouflaged as a GObject
	  */
	 virtual void load_(const GObject* cp) override {
		 // Check that we are dealing with a GBasePluggableOMT<oa_type> reference independent of this object and convert the pointer
		 const GBasePluggableOMT<oa_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GBasePluggableOMT<oa_type>>(cp, this);

		 // Load the parent classes' data ...
		 GObject::load_(cp);

		 // ... and then our local data
		 m_useRawEvaluation = p_load->m_useRawEvaluation;
	 }

	 /************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 virtual GObject* clone_() const override = 0;

	 /***************************************************************************/
	 bool m_useRawEvaluation = false; ///< Specifies whether the true (unmodified) evaluation should be used

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
		 condnotset("GBasePluggableOMT<oa_type>", "GEM_TESTING");
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
		 condnotset("GBasePluggableOMT<oa_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		 condnotset("GBasePluggableOMT<oa_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements basic operations found in iteration-based optimization algorithms.
 * E.g., one might want to stop the optimization after a given number of cycles, or after
 * a given amount of time. The class also defines the interface functions common to these
 * algorithms, such as a general call to "optimize()".
 */
class G_OA_BaseT
	: public GObject
  	, public Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>
   , public G_Interface_Optimizer
{
private:
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
		 & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>>(*this))
		 & make_nvp("G_Interface_Optimizer", boost::serialization::base_object<G_Interface_Optimizer>(*this))
		 & BOOST_SERIALIZATION_NVP(m_iteration)
		 & BOOST_SERIALIZATION_NVP(m_offset)
		 & BOOST_SERIALIZATION_NVP(m_maxIteration)
		 & BOOST_SERIALIZATION_NVP(m_minIteration)
		 & BOOST_SERIALIZATION_NVP(m_maxStallIteration)
		 & BOOST_SERIALIZATION_NVP(m_reportIteration)
		 & BOOST_SERIALIZATION_NVP(m_nRecordbestGlobalIndividuals)
		 & BOOST_SERIALIZATION_NVP(m_bestGlobalIndividuals_pq)
		 & BOOST_SERIALIZATION_NVP(m_defaultPopulationSize)
		 & BOOST_SERIALIZATION_NVP(m_bestKnownPrimaryFitness)
		 & BOOST_SERIALIZATION_NVP(m_bestCurrentPrimaryFitness)
		 & BOOST_SERIALIZATION_NVP(m_stallCounter)
		 & BOOST_SERIALIZATION_NVP(m_stallCounterThreshold)
		 & BOOST_SERIALIZATION_NVP(m_cp_interval)
		 & BOOST_SERIALIZATION_NVP(m_cp_base_name)
		 & BOOST_SERIALIZATION_NVP(m_cp_directory)
		 & BOOST_SERIALIZATION_NVP(m_cp_last)
		 & BOOST_SERIALIZATION_NVP(m_cp_remove)
		 & BOOST_SERIALIZATION_NVP(m_cp_serialization_mode)
		 & BOOST_SERIALIZATION_NVP(m_qualityThreshold)
		 & BOOST_SERIALIZATION_NVP(m_hasQualityThreshold)
		 & BOOST_SERIALIZATION_NVP(m_maxDuration)
		 & BOOST_SERIALIZATION_NVP(m_minDuration)
		 & BOOST_SERIALIZATION_NVP(m_terminationFile)
		 & BOOST_SERIALIZATION_NVP(m_terminateOnFileModification)
		 & BOOST_SERIALIZATION_NVP(m_emitTerminationReason)
		 & BOOST_SERIALIZATION_NVP(m_halted)
		 & BOOST_SERIALIZATION_NVP(m_worstKnownValids_vec)
		 & BOOST_SERIALIZATION_NVP(m_pluggable_monitors_vec)
		 & BOOST_SERIALIZATION_NVP(m_executor_ptr)
		 & BOOST_SERIALIZATION_NVP(m_default_execMode)
		 & BOOST_SERIALIZATION_NVP(m_default_executor_config);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The default constructor. Note that most variables are initialized in the class body.
	  */
	 G_OA_BaseT() = default;

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  *
	  * @param cp A constant reference to another G_OA_BaseT object
	  */
	 G_OA_BaseT(const G_OA_BaseT& cp)
		 : GObject(cp)
			, Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>(cp)
			, m_iteration(cp.m_iteration)
			, m_offset(DEFAULTOFFSET)
			, m_minIteration(cp.m_minIteration)
			, m_maxIteration(cp.m_maxIteration)
			, m_maxStallIteration(cp.m_maxStallIteration)
			, m_reportIteration(cp.m_reportIteration)
			, m_nRecordbestGlobalIndividuals(cp.m_nRecordbestGlobalIndividuals)
			, m_bestGlobalIndividuals_pq(cp.m_bestGlobalIndividuals_pq)
			, m_bestIterationIndividuals_pq(cp.m_bestIterationIndividuals_pq)
			, m_defaultPopulationSize(cp.m_defaultPopulationSize)
			, m_bestKnownPrimaryFitness(cp.m_bestKnownPrimaryFitness)
			, m_bestCurrentPrimaryFitness(cp.m_bestCurrentPrimaryFitness)
			, m_stallCounter(cp.m_stallCounter)
			, m_stallCounterThreshold(cp.m_stallCounterThreshold)
			, m_cp_interval(cp.m_cp_interval)
			, m_cp_base_name(cp.m_cp_base_name)
			, m_cp_directory(cp.m_cp_directory)
		 	, m_cp_last(cp.m_cp_last)
			, m_cp_remove(cp.m_cp_remove)
			, m_cp_serialization_mode(cp.m_cp_serialization_mode)
			, m_qualityThreshold(cp.m_qualityThreshold)
			, m_hasQualityThreshold(cp.m_hasQualityThreshold)
			, m_maxDuration(cp.m_maxDuration)
			, m_minDuration(cp.m_minDuration)
			, m_terminationFile(cp.m_terminationFile)
			, m_terminateOnFileModification(cp.m_terminateOnFileModification)
			, m_emitTerminationReason(cp.m_emitTerminationReason)
			, m_worstKnownValids_vec(cp.m_worstKnownValids_vec)
	 	   , m_default_execMode(cp.m_default_execMode)
	 	   , m_default_executor_config(cp.m_default_executor_config)
	 {
		 // Copy atomics over
		 m_halted.store(cp.m_halted.load());

		 // Copy the executor over
		 Gem::Common::copyCloneableSmartPointer(cp.m_executor_ptr, m_executor_ptr);

		 // Copy the pluggable optimization monitors over (if any)
		 Gem::Common::copyCloneableSmartPointerContainer(cp.m_pluggable_monitors_vec, m_pluggable_monitors_vec);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~G_OA_BaseT()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A standard assignment operator
	  */
	 const G_OA_BaseT& operator=(const G_OA_BaseT& cp) {
		 this->load_(&cp);
		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Checks for equality with another G_OA_BaseT object
	  *
	  * @param  cp A constant reference to another G_OA_BaseT object
	  * @return A boolean indicating whether both objects are equal
	  */
	 bool operator==(const G_OA_BaseT& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Checks for inequality with another G_OA_BaseT object
	  *
	  * @param  cp A constant reference to another G_OA_BaseT object
	  * @return A boolean indicating whether both objects are inequal
	  */
	 bool operator!=(const G_OA_BaseT& cp) const {
		 using namespace Gem::Common;
		 try {
			 this->compare(cp, expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			 return true;
		 } catch(g_expectation_violation&) {
			 return false;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Performs the necessary administratory work of doing check-pointing. Special
	  * work necessary for a given optimization algorithm may be performed in the
	  * virtual function saveCheckpoint(), which is called by this function.
	  *
	  * @param better A boolean which indicates whether a better result was found
	  */
	 void checkpoint(const bool& is_better) const {
		 bool do_save = false;

		 // Determine a suitable name for the checkpoint file
		 bf::path output_file;
		 output_file = getCheckpointPath() / bf::path(
			 "checkpoint-" + this->getAlgorithmPersonalityType() + "-" +
			 (this->halted() ? "final" : boost::lexical_cast<std::string>(getIteration())) + "-" +
			 boost::lexical_cast<std::string>(std::get<G_TRANSFORMED_FITNESS>(getBestKnownPrimaryFitness())) + "-" +
			 getCheckpointBaseName()
		 );

		 // Save checkpoints if required by the user
		 if(m_cp_interval < 0 && is_better) {
			 do_save = true;
		 } // Only save when a better solution was found
		 else if(m_cp_interval > 0 && m_iteration%m_cp_interval == 0) {
			 do_save = true;
		 } // Save in regular intervals
		 else if(this->halted()) {
			 do_save = true;
		 } // Save the final result


		 if(do_save) {
			 saveCheckpoint(output_file);

			 // Remove the last checkoint file if requested by the user
			 if(m_cp_remove && m_cp_last != "empty") {
				 if(boost::filesystem::exists(boost::filesystem::path(m_cp_last))) {
					 boost::filesystem::remove(boost::filesystem::path(m_cp_last));
				 }
			 }

			 // Record the name of the last known checkpoint file
			 m_cp_last = output_file.string();
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Loads the state of the class from disc
	  */
	 virtual void loadCheckpoint(const bf::path& cpFile) BASE {
		 // Extract the name of the optimization algorithm used for this file
		 std::string opt_desc = this->extractOptAlgFromPath(cpFile);

		 // Make sure it fits our own algorithm
		 if(opt_desc != this->getAlgorithmPersonalityType()) {
			 glogger
			 << "In G_OA_BaseT<>::loadCheckpoint(): Error!" << std::endl
		    << "Checkpoint file " << cpFile << std::endl
			 << "seems to belong to another algorithm. Expected " << this->getAlgorithmPersonalityType() << std::endl
			 << "but got " << opt_desc << std::endl
			 << GEXCEPTION;
		 }

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
	  * A negative value will result in automatic checkpointing, whenever a better solution
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
	 std::uint32_t getCheckpointInterval() const {
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
				 << "In G_OA_BaseT::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
				 << "Error: Invalid cpBaseName: " << cpBaseName << std::endl
				 << GEXCEPTION;
		 }

		 if(cpDirectory == "empty" || cpDirectory.empty()) {
			 glogger
				 << "In G_OA_BaseT::setCheckpointBaseName(const std::string&, const std::string&):" << std::endl
				 << "Error: Invalid cpDirectory: " << cpDirectory << std::endl
				 << GEXCEPTION;
		 }

		 m_cp_base_name = cpBaseName;

		 // Check that the provided directory exists
		 if(!boost::filesystem::exists(cpDirectory)) {
			 glogger
				 << "In G_OA_BaseT::setCheckpointBaseName(): Warning!" << std::endl
				 << "Directory " << cpDirectory << " does not exist and will be created automatically." << std::endl
				 << GWARNING;

			 if(!boost::filesystem::create_directory(cpDirectory)) {
				 glogger
					 << "In G_OA_BaseT::setCheckpointBaseName(): Error!" << std::endl
					 << "Could not create directory " << cpDirectory << std::endl
					 << GEXCEPTION;
			 }
		 } else if(!boost::filesystem::is_directory(cpDirectory)) {
			 glogger
				 << "In G_OA_BaseT::setCheckpointBaseName(): Error!" << std::endl
				 << cpDirectory << " exists but is no directory." << std::endl
				 << GEXCEPTION;
		 }

		 // Add a trailing slash to the directory name, if necessary
		 // TODO: THIS IS NOT PORTABLE TO WINDOWS!
		 if(cpDirectory[cpDirectory.size() - 1] != '/') m_cp_directory = cpDirectory + '/';
		 else m_cp_directory = cpDirectory;
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
	 std::string getCheckpointDirectory() const {
		 return m_cp_directory;
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
	  * should be removed or kept
	  */
	 void setRemoveCheckpointFiles(bool cp_remove) {
		 m_cp_remove = cp_remove;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether checkpoint files will be removed
	  */
	 bool checkpointFilesAreRemoved() const {
		 return m_cp_remove;
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

		 // Check that we are dealing with a G_OA_BaseT reference independent of this object and convert the pointer
		 const G_OA_BaseT *p_load = Gem::Common::g_convert_and_compare<GObject, G_OA_BaseT>(cp, this);

		 GToken token("G_OA_BaseT", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GObject>(IDENTITY(*this, *p_load), token);

		 // ... and then the local data
		 compare_t(IDENTITY(this->data,  p_load->data), token); // This allows us to compare the parent class without directly referring to it.
		 compare_t(IDENTITY(m_iteration, p_load->m_iteration), token);
		 compare_t(IDENTITY(m_offset, p_load->m_offset), token);
		 compare_t(IDENTITY(m_maxIteration, p_load->m_maxIteration), token);
		 compare_t(IDENTITY(m_minIteration, p_load->m_minIteration), token);
		 compare_t(IDENTITY(m_maxStallIteration, p_load->m_maxStallIteration), token);
		 compare_t(IDENTITY(m_reportIteration, p_load->m_reportIteration), token);
		 compare_t(IDENTITY(m_nRecordbestGlobalIndividuals, p_load->m_nRecordbestGlobalIndividuals), token);
		 compare_t(IDENTITY(m_bestGlobalIndividuals_pq, p_load->m_bestGlobalIndividuals_pq), token);
		 compare_t(IDENTITY(m_bestIterationIndividuals_pq, p_load->m_bestIterationIndividuals_pq), token);
		 compare_t(IDENTITY(m_defaultPopulationSize, p_load->m_defaultPopulationSize), token);
		 compare_t(IDENTITY(m_bestKnownPrimaryFitness, p_load->m_bestKnownPrimaryFitness), token);
		 compare_t(IDENTITY(m_bestCurrentPrimaryFitness, p_load->m_bestCurrentPrimaryFitness), token);
		 compare_t(IDENTITY(m_stallCounter, p_load->m_stallCounter), token);
		 compare_t(IDENTITY(m_stallCounterThreshold, p_load->m_stallCounterThreshold), token);
		 compare_t(IDENTITY(m_cp_interval, p_load->m_cp_interval), token);
		 compare_t(IDENTITY(m_cp_base_name, p_load->m_cp_base_name), token);
		 compare_t(IDENTITY(m_cp_directory, p_load->m_cp_directory), token);
		 compare_t(IDENTITY(m_cp_last, p_load->m_cp_last), token);
		 compare_t(IDENTITY(m_cp_remove, p_load->m_cp_remove), token);
		 compare_t(IDENTITY(m_cp_serialization_mode, p_load->m_cp_serialization_mode), token);
		 compare_t(IDENTITY(m_qualityThreshold, p_load->m_qualityThreshold), token);
		 compare_t(IDENTITY(m_hasQualityThreshold, p_load->m_hasQualityThreshold), token);
		 compare_t(IDENTITY(m_maxDuration.count(), p_load->m_maxDuration.count()), token); // Cannot directly compare std::chrono::duration<double>
		 compare_t(IDENTITY(m_minDuration.count(), p_load->m_minDuration.count()), token); // Cannot directly compare std::chrono::duration<double>
		 compare_t(IDENTITY(m_terminationFile, p_load->m_terminationFile), token);
		 compare_t(IDENTITY(m_terminateOnFileModification, p_load->m_terminateOnFileModification), token);
		 compare_t(IDENTITY(m_emitTerminationReason, p_load->m_emitTerminationReason), token);
		 compare_t(IDENTITY(m_halted, p_load->m_halted), token);
		 compare_t(IDENTITY(m_worstKnownValids_vec, p_load->m_worstKnownValids_vec), token);
		 compare_t(IDENTITY(m_pluggable_monitors_vec, p_load->m_pluggable_monitors_vec), token);
		 compare_t(IDENTITY(m_executor_ptr, p_load->m_executor_ptr), token);
		 compare_t(IDENTITY(m_default_execMode, p_load->m_default_execMode), token);
		 compare_t(IDENTITY(m_default_executor_config, p_load->m_default_executor_config), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Resets the class to the state before the optimize call. This will in
	  * particular erase all individuals stored in this class and clear the list
	  * of best individuals. Please note that a subsequent call to optimize will
	  * result in an error, unless you add new individuals. The purpose of this
	  * function is allow repeated optimization with the same settings, but different
	  * starting points. Actual implementations of optimization algorithms derived
	  * from this class may have to perform additional work by overloading (and
	  * calling) this function.
	  */
	 virtual void resetToOptimizationStart() BASE {
		 this->clear(); // Remove all individuals found in this population

		 m_iteration = 0; // The current iteration
		 m_bestGlobalIndividuals_pq.clear(); // A priority queue with the best individuals found so far
		 m_bestIterationIndividuals_pq.clear(); // A priority queue with the best individuals of a given iteration

		 m_bestKnownPrimaryFitness = std::tuple<double,double>(0.,0.); // Records the best primary fitness found so far
		 m_bestCurrentPrimaryFitness = std::tuple<double,double>(0.,0.); // Records the best fitness found in the current iteration

		 m_stallCounter = 0; // Counts the number of iterations without improvement

		 m_halted = true; // Also means: No optimization is currently running

		 m_worstKnownValids_vec.clear(); // Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: tranformed)
	 }

	 /***************************************************************************/
	 /**
	  * Adds a new executor to the class, replacing the default executor. The
	  * executor is responsible for evaluating the individuals.
	  *
	  * @param executor_ptr A pointer to an executor
	  * @param executorConfigFile The name of a file used to configure the executor
	  */
	 void registerExecutor(
		 std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> executor_ptr
	 	 , const std::string& executorConfigFile
	 ) {
		 if(!executor_ptr) {
			 glogger
			 << "In G_OA_BaseT::registerExecutor(): Warning!" << std::endl
	       << "Tried to register empty executor-pointer. We will leave the existing" << std::endl
			 << "executor in place" << std::endl
			 << GWARNING;

			 return;
		 }

		 if(!m_halted) {
			 glogger
			 << "In G_OA_BaseT::registerExecutor(): Warning!" << std::endl
			 << "Tried to register an executor while the optimization is already running" << std::endl
			 << "The new executor will be ignored." << std::endl
			 << GWARNING;

			 return;
		 }

		 // Register the new executor
		 m_executor_ptr = executor_ptr;

		 // Give the executor a chance to configure itself from
		 // user-defined configuration options
		 Gem::Common::GParserBuilder gpb;
		 m_executor_ptr->addConfigurationOptions(gpb);
		 if (!gpb.parseConfigFile(executorConfigFile)) {
			 glogger
				 << "In G_OA_BaseT::registerExecutor(): Error!" << std::endl
				 << "Could not parse configuration file " << executorConfigFile << std::endl
				 << GEXCEPTION;
		 }

		 // TODO: Check that the new executor has the desired configuration
	 }

	 /***************************************************************************/
	 /**
	  * Adds a new executor to the class, using the chosen execution mode
	  *
	  * @param e The execution mode
	  * @param executorConfigFile The name of a file used to configure the executor
	  */
	 void registerExecutor(
		 execMode e
		 , const std::string& executorConfigFile
	 ) {
		 auto executor_ptr = this->createExecutor(e);
		 this->registerExecutor(executor_ptr, executorConfigFile);
	 }

	 /***************************************************************************/
	 /**
 	  * Gives access to the current executor, converted to a given target type.
 	  * The executor is internally stored via its base class, so we need to
 	  * convert it to its final type in order to configure it via its API. The
 	  * function is only accessible when converting to a derived class of GBaseExecutorT.
 	  * You need to take care yourself that the stored class matches the one you
 	  * are converting to. The function will throw (via dynamic_pointer_cast), if
 	  * this is not the case.
 	  */
	 template <typename target_type>
	 std::shared_ptr<target_type> getExecutor(
		 typename std::enable_if<std::is_base_of<Gem::Courtier::GBaseExecutorT<GParameterSet>, target_type>::value>::type *dummy = nullptr
	 ) {
		 return std::dynamic_pointer_cast<target_type>(m_executor_ptr);
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
	 virtual void optimize(const std::uint32_t& offset) final {
		 // Reset the generation counter
		 m_iteration = offset;

		 // Set the iteration offset
		 m_offset = offset;

		 // Store any *clean* individuals that have been added to this algorithm
		 // in the priority queue. This happens so that best individuals from a
		 // previous "chained" optimization run aren't lost.
		 addCleanStoredBests(m_bestGlobalIndividuals_pq);

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
		 if(m_reportIteration) informationUpdate(infoMode::INFOINIT);

		 // We want to know if no better values were found for a longer period of time
		 double worstCase = this->at(0)->getWorstCase();
		 m_bestKnownPrimaryFitness   = std::make_tuple(worstCase, worstCase);
		 m_bestCurrentPrimaryFitness = std::make_tuple(worstCase, worstCase);

		 m_stallCounter = 0;

		 // Give derived classes the opportunity to perform any other necessary preparatory work.
		 init();

		 // Let the algorithm know that the optimization process hasn't been halted yet.
		 m_halted = false; // general halt criterion

		 // Initialize the start time with the current time.
		 m_startTime = std::chrono::system_clock::now();

		 do {
			 // Let all individuals know the current iteration
			 markIteration();

			 // Update fitness values and the stall counter
			 updateStallCounter((m_bestCurrentPrimaryFitness=cycleLogic()));

			 // Add the best individuals to the m_bestGlobalIndividuals_pq
			 // and m_bestIterationIndividuals_pq vectors
			 updateGlobalBestsPQ(m_bestGlobalIndividuals_pq);
			 updateIterationBestsPQ(m_bestIterationIndividuals_pq);

			 // Check whether a better value was found, and do the check-pointing, if necessary and requested.
			 checkpoint(progress());

			 // Let all individuals know about the best fitness known so far
			 markBestFitness();

			 // Let individuals know about the stalls encountered so far
			 markNStalls();

			 // Give derived classes an opportunity to act on stalls. NOTE that no action
			 // may be taken that affects the "dirty" state of individuals
			 if(m_stallCounterThreshold && stallCounterThresholdExceeded()) {
				 actOnStalls();
			 }

			 // We want to provide feedback to the user in regular intervals.
			 // Set the reportGeneration_ variable to 0 in order not to emit
			 // any information at all.
			 if(m_reportIteration && (m_iteration%m_reportIteration == 0)) {
				 informationUpdate(infoMode::INFOPROCESSING);
			 }

			 // update the m_iteration counter
			 m_iteration++;
		 }
		 while(!(m_halted = halt()));

		 // Give derived classes the opportunity to perform any remaining clean-up work
		 finalize();

		 // Finalize the info output
		 if(m_reportIteration) informationUpdate(infoMode::INFOEND);

		 // Remove information particular to the optimization algorithms from the individuals
		 resetIndividualPersonalities();
	 }

	 /***************************************************************************/
	 /**
	  * A little convenience function that helps to avoid having to specify explicit scopes
	  */
	 virtual void optimize() final {
		 G_Interface_Optimizer::optimize();
	 }

	 /***************************************************************************/
	 /**
	  * Emits information specific to this class (basic information in each iteration
	  * plus some user-defined information via pluggable optimization monitors)
	  *
	  * @param im The information mode (INFOINIT, INFOPROCESSING or INFOEND)
	  */
	 void informationUpdate(const infoMode& im) {
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

			 default:
			 {
				 glogger
					 << "G_OA_BaseT<>::informationUpdate(" << im << "): Received invalid infoMode " << std::endl
					 << GEXCEPTION;
			 }
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
		 return (0==m_stallCounter);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a pluggable optimization monitor. Note that this
	  * function does NOT take ownership of the optimization monitor.
	  */
	 void registerPluggableOM(
		 std::shared_ptr<GBasePluggableOMT<G_OA_BaseT>> pluggableOM
	 ) {
		 if(pluggableOM) {
			 m_pluggable_monitors_vec.push_back(pluggableOM);
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
		 return m_defaultPopulationSize;
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
		 if(m_maxIteration > 0 && m_maxIteration <= m_minIteration) {
			 glogger
				 << "In G_OA_BaseT<>::setMaxIteration(): Error!" << std::endl
				 << "Maximum number of iterations " << 	m_maxIteration << " is <= the minimum number " << m_minIteration << std::endl
				 << GEXCEPTION;
		 }

		 m_maxIteration = maxIteration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of iterations after which optimization should
	  * be stopped
	  *
	  * @return The number of iterations after which the optimization should terminate
	  */
	 std::uint32_t getMaxIteration() const {
		 return m_maxIteration;
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
		 if(m_maxIteration > 0 && m_maxIteration <= m_minIteration) {
			 glogger
				 << "In G_OA_BaseT<>::setMinIteration(): Error!" << std::endl
				 << "Maximum number of iterations " << 	m_maxIteration << " is <= the minimum number " << m_minIteration << std::endl
				 << GEXCEPTION;
		 }

		 m_minIteration = minIteration;
	 }

	 /***************************************************************************/
	 /**
	  * This function retrieves the value of the minIteration_ variable
	  */
	 std::uint32_t getMinIteration() const {
		 return m_minIteration;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum number of generations allowed without improvement of the best
	  * individual. Set to 0 in order for this stop criterion to be disabled.
	  *
	  * @param The maximum number of allowed generations
	  */
	 void setMaxStallIteration(std::uint32_t maxStallIteration) {
		 m_maxStallIteration = maxStallIteration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the maximum number of generations allowed in an optimization run without
	  * improvement of the best individual.
	  *
	  * @return The maximum number of generations
	  */
	 std::uint32_t getMaxStallIteration() const {
		 return m_maxStallIteration;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the maximum allowed processing time
	  *
	  * @param maxDuration The maximum allowed processing time
	  */
	 void setMaxTime(std::chrono::duration<double> maxDuration) {
		 if(!Gem::Common::isClose<double>(maxDuration.count(), 0.) && maxDuration < m_minDuration) {
			 glogger
				 << "In G_OA_BaseT<>::setMaxTime(): Error!" << std::endl
				 << "Desired maxDuration (" << maxDuration.count() << " is smaller than m_minDuration(" << m_minDuration.count() << ")" << std::endl
				 << GEXCEPTION;
		 }

		 m_maxDuration = maxDuration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the value of the maxDuration_ parameter.
	  *
	  * @return The maximum allowed processing time
	  */
	 std::chrono::duration<double> getMaxTime() const {
		 return m_maxDuration;
	 }

	 /***************************************************************************/
	 /**
	 * Sets the minimum required processing time. NOTE: Always set the maximum duration
	 * before the minumum duration.
	 *
	 * @param minDuration The minimum allowed processing time
	 */
	 void setMinTime(std::chrono::duration<double> minDuration) {
		 if(!Gem::Common::isClose<double>(m_maxDuration.count(),0.) && m_maxDuration < minDuration) {
			 glogger
				 << "In G_OA_BaseT<>::setMinTime(): Error!" << std::endl
				 << "Desired maxDuration (" << m_maxDuration.count() << " is smaller than m_minDuration(" << minDuration.count() << ")" << std::endl
				 << GEXCEPTION;
		 }

		 m_minDuration = minDuration;
	 }

	 /***************************************************************************/
	 /**
	 * Retrieves the value of the minDuration_ parameter.
	 *
	 * @return The minimum required processing time
	 */
	 std::chrono::duration<double> getMinTime() const {
		 return m_minDuration;
	 }

	 /***************************************************************************/
	 /**
	  *  Sets a quality threshold beyond which optimization is expected to stop
	  *
	  *  @param qualityThreshold A threshold beyond which optimization should stop
	  *  @param hasQualityThreshold Allows to (de-)activate the quality threshold
	  */
	 void setQualityThreshold(double qualityThreshold, bool hasQualityThreshold) {
		 m_qualityThreshold = qualityThreshold;
		 m_hasQualityThreshold=hasQualityThreshold;
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
		 hasQualityThreshold = m_hasQualityThreshold;
		 return m_qualityThreshold;
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
		 m_terminationFile = terminationFile;
		 m_terminateOnFileModification = terminateOnFileModification;
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
		 terminateOnFileModification = m_terminateOnFileModification;
		 return m_terminationFile;
	 }

	 /***************************************************************************/
	 /**
	  * Removes the quality threshold
	  */
	 void resetQualityThreshold() {
		 m_hasQualityThreshold = false;
	 }

	 /***************************************************************************/
	 /**
	  * Checks whether a quality threshold has been set
	  *
	  * @return A boolean indicating whether a quality threshold has been set
	  */
	 bool hasQualityThreshold() const {
		 return m_hasQualityThreshold;
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
		 m_reportIteration = iter;
	 }

	 /***************************************************************************/
	 /**
	  * Returns the number of iterations after which the algorithm should
	  * report about its inner state.
	  *
	  * @return The number of iterations after which information is emitted
	  */
	 std::uint32_t getReportIteration() const {
		 return m_reportIteration;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current number of failed optimization attempts
	  *
	  * @return The current number of failed optimization attempts
	  */
	 std::uint32_t getStallCounter() const {
		 return m_stallCounter;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the number of iterations without improvement, after which
	  * individuals are asked to update their internal data structures
	  */
	 void setStallCounterThreshold(std::uint32_t stallCounterThreshold) {
		 m_stallCounterThreshold = stallCounterThreshold;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the number of iterations without improvement, after which
	  * individuals are asked to update their internal data structures
	  */
	 std::uint32_t getStallCounterThreshold() const {
		 return m_stallCounterThreshold;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the best value found in the entire optimization run so far
	  *
	  * @return The best raw and transformed fitness found so far
	  */
	 std::tuple<double, double> getBestKnownPrimaryFitness() const {
		 return (m_bestGlobalIndividuals_pq.best())->getFitnessTuple();

		 // return m_bestKnownPrimaryFitness;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best value found in the current iteration
	  *
	  * @return The best raw and transformed fitness found in the current iteration
	  */
	 std::tuple<double, double> getBestCurrentPrimaryFitness() const {
		 return m_bestCurrentPrimaryFitness;
	 }

	 /***************************************************************************/
	 /**
	  * Specifies whether information about termination reasons should be emitted
	  *
	  * @param etr A boolean which specifies whether reasons for the termination of the optimization run should be emitted
	  */
	 void setEmitTerminationReason(bool emitTerminatioReason = true) {
		 m_emitTerminationReason = emitTerminatioReason;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves information on whether information about termination reasons should be emitted
	  *
	  * @return A boolean which specifies whether reasons for the termination of the optimization run will be emitted
	  */
	 bool getEmitTerminationReason() const {
		 return m_emitTerminationReason;
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
	 ) {
#ifdef DEBUG
		 if(pos >= this->size()) {
			 glogger
				 << "In G_OA_BaseT::individual_cast<>() : Error" << std::endl
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
	 virtual std::size_t getNProcessableItems() const BASE {
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
		 // Call our parent class'es function
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

		 gpb.registerFileParameter<bool>(
			 "cpOverwrite" // The name of the variable
			 , true // The default value -- we always remove old checkpoints
			 , [this](bool cp_overwrite){ this->setRemoveCheckpointFiles(cp_overwrite); }
		 )
			 << "When set to \"true\", old checkpoint files will not be kept";

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

		 gpb.registerFileParameter<execMode, std::string>(
			 "defaultExecMode" // The name of the variable
			 , "defaultExecConfig"
			 , this->m_default_execMode // The default value
			 , this->m_default_executor_config
			 , [this](execMode e, std::string config){ this->m_default_execMode = e; this->m_default_executor_config = config; }
			 , "defaultExecutor"
		 )
			 << "The default executor type to be used for this algorithm." << std::endl
			 << "0: serial" << std::endl
		 	 << "1: multi-threaded" << std::endl
		    << "2: brokered" << std::endl << Gem::Common::nextComment()
			 << "The configuration file for the default executor. Note that it needs to fit the executor type.";

		 execMode m_default_execMode = execMode::BROKER; ///< The default execution mode. Unless explicitöy requested by the user, we always go through the broker
		 std::string m_default_executor_config = "./config/GBrokerExecutor.json"; ///< The default configuration file for the broker executor
	 }

	 /***************************************************************************/
	 /**
	  * Adds the individuals of this iteration to a priority queue. The
 	  * queue will be sorted by the first evaluation criterion of the individuals
 	  * and may either have a limited or unlimited size, depending on user-
 	  * settings
	  */
	 virtual void updateGlobalBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {
		 const bool CLONE = true;
		 const bool DONOTREPLACE = false;

#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In G_OA_BaseT::updateGlobalBestsPQ() :" << std::endl
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
	  * Adds the individuals of this iteration to a priority queue. The
	  * queue will be sorted by the first evaluation criterion of the individuals
	  * and may either have a limited or unlimited size, depending on user-
	  * settings
	  */
	 virtual void updateIterationBestsPQ(GParameterSetFixedSizePriorityQueue& bestIndividuals) BASE {
		 const bool CLONE = true;
		 const bool REPLACE = true;

#ifdef DEBUG
		 if(this->empty()) {
			 glogger
				 << "In G_OA_BaseT::updateIterationBestsPQ() :" << std::endl
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
	 void addCleanStoredBests(GParameterSetFixedSizePriorityQueue& bestIndividuals) {
		 const bool CLONE = true;

		 // We simply add all *clean* individuals to the queue -- only the best ones will actually be added
		 // (and cloned) Unless we have asked for the queue to have an unlimited size, the queue will be
		 // resized as required by its maximum allowed size.
		 for(auto ind_ptr: *this) {
			 if(ind_ptr->isClean()) {
				 bestIndividuals.add(ind_ptr, CLONE);
			 }
		 }
	 }

	 /***************************************************************************/

	 /** @brief Emits a name for this class / object; this can be a long name with spaces */
	 virtual std::string name() const override = 0;

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

	 /***************************************************************************/
	 /**
	  * Checks whether a checkpoint-file has the same "personality" as our
	  * own algorithm
	  */
	 bool cp_personality_fits(const boost::filesystem::path& p) const {
		 // Extract the name of the optimization algorithm used for this file
		 std::string opt_desc = this->extractOptAlgFromPath(p);

		 // Make sure it fits our own algorithm
		 if(opt_desc != this->getAlgorithmPersonalityType()) {
			 return false;
		 } else {
			 return true;
		 }
	 }

protected:
	 /***************************************************************************/
	 // Some data

	 Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> m_gr; ///< A random number generator. Note that the actual calculation is done in a random number proxy / factory
	 std::uniform_real_distribution<double> m_uniform_real_distribution; ///< Access to uniformly distributed double random values

	 /***************************************************************************/
	 /**
	  * Loads the data of another GOptimizationAlgorithm object
	  *
	  * @param cp Another GOptimizationAlgorithm object, camouflaged as a GObject
	  */
	 virtual void load_(const GObject* cp) override {
		 // Check that we are dealing with a G_OA_BaseT reference independent of this object and convert the pointer
		 const G_OA_BaseT *p_load = Gem::Common::g_convert_and_compare<GObject, G_OA_BaseT>(cp, this);

		 // Load the parent class'es data
		 GObject::load_(cp);
		 Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::operator=(*p_load);

		 // and then our local data
		 m_iteration = p_load->m_iteration;
		 m_offset = p_load->m_offset;
		 m_maxIteration = p_load->m_maxIteration;
		 m_minIteration = p_load->m_minIteration;
		 m_maxStallIteration = p_load->m_maxStallIteration;
		 m_reportIteration = p_load->m_reportIteration;
		 m_nRecordbestGlobalIndividuals = p_load->m_nRecordbestGlobalIndividuals;
		 m_bestGlobalIndividuals_pq = p_load->m_bestGlobalIndividuals_pq;
		 m_bestIterationIndividuals_pq = p_load->m_bestIterationIndividuals_pq;
		 m_defaultPopulationSize = p_load->m_defaultPopulationSize;
		 m_bestKnownPrimaryFitness = p_load->m_bestKnownPrimaryFitness;
		 m_bestCurrentPrimaryFitness = p_load->m_bestCurrentPrimaryFitness;
		 m_stallCounter = p_load->m_stallCounter;
		 m_stallCounterThreshold = p_load->m_stallCounterThreshold;
		 m_cp_interval = p_load->m_cp_interval;
		 m_cp_base_name = p_load->m_cp_base_name;
		 m_cp_directory = p_load->m_cp_directory;
		 m_cp_last = p_load->m_cp_last;
		 m_cp_remove = p_load->m_cp_remove;
		 m_cp_serialization_mode = p_load->m_cp_serialization_mode;
		 m_qualityThreshold = p_load->m_qualityThreshold;
		 m_hasQualityThreshold = p_load->m_hasQualityThreshold;
		 m_terminationFile = p_load->m_terminationFile;
		 m_terminateOnFileModification = p_load->m_terminateOnFileModification;
		 m_maxDuration = p_load->m_maxDuration;
		 m_minDuration = p_load->m_minDuration;
		 m_emitTerminationReason = p_load->m_emitTerminationReason;
		 m_halted.store(p_load->m_halted.load());
		 m_worstKnownValids_vec = p_load->m_worstKnownValids_vec;
		 Gem::Common::copyCloneableSmartPointerContainer(p_load->m_pluggable_monitors_vec, m_pluggable_monitors_vec);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_executor_ptr, m_executor_ptr);
		 m_default_execMode = p_load->m_default_execMode;
		 m_default_executor_config = p_load->m_default_executor_config;
	 }

	 /***************************************************************************/
	 /**
	  * Delegation of work to be performed to the private executor object
	  */
	 bool workOn(
		 std::vector<std::shared_ptr<GParameterSet>>& workItems
		 , std::vector<bool>& workItemPos
		 , bool resubmitUnprocessed = false
		 , const std::string &caller = std::string()
	 ) {
		 return m_executor_ptr->workOn(
			workItems
			, workItemPos
		 	, resubmitUnprocessed
		 	, caller
		 );
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a vector of old work items after job submission
	  */
	 std::vector<std::shared_ptr<GParameterSet>> getOldWorkItems() {
		 return m_executor_ptr->getOldWorkItems();
	 }

	 /***************************************************************************/
	 /**
	  * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	  * Make the vector wrapper purely virtual allows the compiler to perform
	  * further optimizations.
	  */
	 virtual void dummyFunction() override { /* nothing */ }

	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 virtual GObject* clone_() const override = 0;

	 /***************************************************************************/
	 /**
	  * Saves the state of the class to disc
	  */
	 virtual void saveCheckpoint(bf::path outputFile) const {
		 this->toFile(outputFile, this->getCheckpointSerializationMode());
	 }

	 /***************************************************************************/
	 /**
	  * Extracts the short name of the optimization algorithm (example:
	  * "PERSONALITY_EA") from a path which complies to the following
	  * scheme: /some/path/word1-PERSONALITY_EA-some-other-information .
	  * This is mainly used for checkpointing and associated cross-checks.
	  */
	 std::string extractOptAlgFromPath(const boost::filesystem::path& p) const {
		 // Extract the filename
		 std::string filename = p.filename().string();

		 // Divide the name into tokens
		 std::vector<std::string> tokens = Gem::Common::splitString(filename, "-");

		 // Check that the size is at least 2 (i.e. the PERSONALITY_X-part may exist)
		 if(tokens.size() < 2) {
			 glogger
			 << "In G_OA_BaseT<>::extractOptAlgFromPath(): Error!" << std::endl
		    << "Found file name " << filename << " that does not comply to rules." << std::endl
			 << "Expected \"/some/path/word1-PERSONALITY_EA-some-other-information \"" << std::endl
		    << GEXCEPTION;
		 }

		 // Let the audience know
		 return tokens[1];
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual found up to now (which is usually the best individual
	  * in the priority queue).
	  */
	 virtual std::shared_ptr<GParameterSet> customGetBestGlobalIndividual() override {
#ifdef DEBUG
		 std::shared_ptr<GParameterSet> p = m_bestGlobalIndividuals_pq.best();
		 if(p) return p;
		 else {
			 glogger
				 << "In G_OA_BaseT<T>::customGetBestGlobalIndividual(): Error!" << std::endl
				 << "Best individual seems to be empty" << std::endl
				 << GEXCEPTION;

			 // Make the compiler happy
			 return std::shared_ptr<GParameterSet>();
		 }
#else
		 return m_bestGlobalIndividuals_pq.best();
#endif
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found (equal to the content of
	  * the priority queue)
	  */
	 virtual std::vector<std::shared_ptr<GParameterSet>> customGetBestGlobalIndividuals() override {
		 return m_bestGlobalIndividuals_pq.toVector();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the best individual found in the iteration (which is the best individual
	  * in the priority queue).
	  */
	 virtual std::shared_ptr<GParameterSet> customGetBestIterationIndividual() override {
#ifdef DEBUG
		 std::shared_ptr<GParameterSet> p = m_bestIterationIndividuals_pq.best();
		 if(p) return p;
		 else {
			 glogger
				 << "In G_OA_BaseT<T>::customGetBestIterationIndividual(): Error!" << std::endl
				 << "Best individual seems to be empty" << std::endl
				 << GEXCEPTION;

			 // Make the compiler happy
			 return std::shared_ptr<GParameterSet>();
		 }
#else
		 return m_bestIterationIndividuals_pq.best();
#endif
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves a list of the best individuals found in the iteration (equal to the content of
	  * the priority queue)
	  */
	 virtual std::vector<std::shared_ptr<GParameterSet>> customGetBestIterationIndividuals() override {
		 return m_bestIterationIndividuals_pq.toVector();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the personality type of the individuals
	  */
	 void setIndividualPersonalities() {
		 for(auto ind_ptr: *this) { ind_ptr->setPersonality(this->getPersonalityTraits()); }
	 }

	 /***************************************************************************/
	 /**
	  * Resets the individual's personality types
	  */
	 void resetIndividualPersonalities() {
		 for(auto ind_ptr: *this) { ind_ptr->resetPersonality(); }
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
		 m_defaultPopulationSize = defPopSize;
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
				 << "In G_OA_BaseT<>::setNRecordBestIndividuals(): Error!" << std::endl
				 << "Invalid number of individuals to be recorded: " << nRecordBestIndividuals << std::endl
				 << GEXCEPTION;
		 }

		 m_nRecordbestGlobalIndividuals = nRecordBestIndividuals;
		 m_bestGlobalIndividuals_pq.setMaxSize(m_nRecordbestGlobalIndividuals);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of best individuals to be recorded in each iteration
	  *
	  * @return The number of best individuals to be recorded in each iteration
	  */
	 std::size_t getNRecordBestIndividuals() const {
		 return m_nRecordbestGlobalIndividuals;
	 }

	 /***************************************************************************/
	 /**
	  * It is possible for derived classes to specify in overloaded versions of this
	  * function under which conditions the optimization should be stopped. The
	  * function is called from G_OA_BaseT::halt .
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
		 m_stallCounter = 0;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform initialization work before the optimization cycle starts. This
	  * function will usually be overloaded by derived functions, which should however,
	  * as their first action, call this function.
	  */
	 virtual void init() BASE {
		 // Add an executor, if none has been registered
		 if(!m_executor_ptr) {
			 auto executor_ptr = this->createExecutor(m_default_execMode);

#ifdef DEBUG
			 if(!executor_ptr) {
				 glogger
					 << "In G_OA_BaseT<>::init(): Error!" << std::endl
				    << "Did not receive a valid executor" << std::endl
					 << GEXCEPTION;
			 }
#endif

			 glogger
			 << "In G_OA_BaseT<>::init(): No explicit executor was registered. Using default" << std::endl
			 << "\"" << executor_ptr->name() << "\" with config \"" << this->m_default_executor_config << "\" instead" << std::endl
			 << GLOGGING;

			 this->registerExecutor(
				 executor_ptr
				 , this->m_default_executor_config
			 );
		 }

		 // Initialize the executor
		 m_executor_ptr->init();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to perform any remaining work after the optimization cycle has finished.
	  * This function will usually be overloaded by derived functions, which should however
	  * call this function as their last action.
	  */
	 virtual void finalize() BASE {
		 // Finalize the broker connector
		 m_executor_ptr->finalize();
	 }

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
	 void markIteration() {
		 for(auto ind: *this) { ind->setAssignedIteration(m_iteration); }
	 }

	 /***************************************************************************/
	 /**
	  * Updates the worst known valid evaluations up to the current iteration
	  * and stores the fitness-values internally. Note: The first tuple-value
	  * in the vector signifies the untransformed (but possible == MIN/MAX_DOUBLE)
	  * evaluation, the second value the potentially transformed value.
	  */
	 void updateWorstKnownValid() {
		 std::size_t nFitnessCriteria = (*(this->begin()))->getNumberOfFitnessCriteria();

		 // Is this the first call ? Fill m_worstKnownValids_vec with data
		 if(inFirstIteration()) {
			 for(auto ind_ptr: *this) { ind_ptr->populateWorstKnownValid(); }

			 // Initialize our own, local m_worstKnownValids_vec
			 m_worstKnownValids_vec = (*(this->begin()))->getWorstKnownValids();
		 }

		 std::size_t ind_cnt = 0;
		 for(auto ind_ptr: *this) {
#ifdef DEBUG
			 if(ind_ptr->getNumberOfFitnessCriteria() != nFitnessCriteria) {
				 glogger
					 << "In G_OA_BaseT<>::updateWorstKnownValid(): Error!" << std::endl
					 << "Got " << ind_ptr->getNumberOfFitnessCriteria() << " fitness criteria in individual " << ind_cnt << std::endl
					 << "but expected " << nFitnessCriteria << " criteria" << std::endl
					 << GEXCEPTION;
			 }

			 if(!m_worstKnownValids_vec.empty() && m_worstKnownValids_vec.size() != nFitnessCriteria) {
				 glogger
					 << "In G_OA_BaseT<>::updateWorstKnownValid(): Error!" << std::endl
					 << "Got invalid number of evaluation criteria in m_worstKnownValids_vec:" << std::endl
					 << "Got " << m_worstKnownValids_vec.size() << " but expected " << nFitnessCriteria << std::endl
					 << GEXCEPTION;
			 }
#endif /* DEBUG */

			 if(ind_ptr->isClean() && ind_ptr->isValid()) { // Is this an individual which has been evaluated and fulfills all constraints ?
				 for(std::size_t id=0; id<nFitnessCriteria; id++) {
					 ind_ptr->challengeWorstValidFitness(m_worstKnownValids_vec.at(id), id);
				 }
			 }

			 ind_cnt++;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Let the individuals know about the worst known valid solution so far
	  */
	 void markWorstKnownValid() {
		 this->updateWorstKnownValid();
		 for(auto ind_ptr: *this) { ind_ptr->setWorstKnownValid(m_worstKnownValids_vec); }
	 }

	 /***************************************************************************/
	 /**
	  * Triggers an update of the individual's evaluation (e.g. in order to
	  * act on the information regarding best or worst evaluations found
	  */
	 void triggerEvaluationUpdate() {
		 for(auto ind_ptr: *this) { ind_ptr->postEvaluationUpdate();}
	 }

	 /***************************************************************************/
	 /**
	  * Work to be performed right after the individuals were evaluated. NOTE:
	  * this setup is sub-optimal, as this function isn't called from within
	  * G_OA_BaseT directly, but only from derived classes. This happens
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
		 for(auto ind_ptr: *this) { ind_ptr->setNStalls(m_stallCounter); }
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

private:
	 /***************************************************************************/
	 /**
	  * Update the stall counter. We use the transformed fitness for comparison
	  * here, so we can usually deal with finite values (due to the transformation
	  * in the case of a constraint violation).
	  */
	 void updateStallCounter(const std::tuple<double, double>& bestEval) {
		 if(this->at(0)->isBetter(std::get<G_TRANSFORMED_FITNESS>(bestEval), std::get<G_TRANSFORMED_FITNESS>(m_bestKnownPrimaryFitness))) {
			 m_bestKnownPrimaryFitness = bestEval;
			 m_stallCounter = 0;
		 } else {
			 m_stallCounter++;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * This function returns true once a given time (set with
	  * GOptimizationAlgorithm<GParameterSet>::setMaxTime()) has passed.
	  * It is used in the G_OA_BaseT::halt() function.
	  *
	  * @return A boolean indicating whether a given amount of time has passed
	  */
	 bool timedHalt(const std::chrono::system_clock::time_point& currentTime) const {
		 if((currentTime - m_startTime) >= m_maxDuration) {
			 if(m_emitTerminationReason) {
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
		 if((currentTime - m_startTime) <= m_minDuration) {
			 return false;
		 } else {
			 return true;
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
		 if(this->at(0)->isBetter(std::get<G_RAW_FITNESS>(m_bestKnownPrimaryFitness), m_qualityThreshold)) {
			 if(m_emitTerminationReason) {
				 glogger
					 << "Terminating optimization run because" << std::endl
					 << "quality threshold " << m_qualityThreshold << " has been exceeded." << std::endl
					 << "Best untransformed quality found was " << std::get<G_RAW_FITNESS>(m_bestKnownPrimaryFitness) << std::endl
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
		 if(m_stallCounter > m_maxStallIteration) {
			 if(m_emitTerminationReason) {
				 std::cout
					 << "Terminating optimization run because" << std::endl
					 << "maximum number of stalls " << m_maxStallIteration << " has been exceeded." << std::endl
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
		 if(m_iteration >= (m_maxIteration + m_offset)) {
			 if(m_emitTerminationReason) {
				 std::cout
					 << "Terminating optimization run because" << std::endl
					 << "iteration threshold " << m_maxIteration << " has been exceeded." << std::endl;
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
		 if(m_iteration <= m_minIteration) {
			 return false;
		 }
		 return true;
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
		 bf::path p(m_terminationFile);

		 // Return if the file doesn't exist
		 if(!bf::exists(p)) {
			 return false;
		 }

		 // Determine the modification time of the file
		 std::time_t t = boost::filesystem::last_write_time(p);
		 std::chrono::system_clock::time_point modTime = std::chrono::system_clock::from_time_t(t);

		 // Check if the file was modified after the start of the optimization run
		 if(modTime > m_startTime) {
			 if(m_emitTerminationReason) {
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
			 if(m_emitTerminationReason) {
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
		 if(m_terminateOnFileModification && touchHalt()) return true;

		 //------------------------------------------------------------------------
		 // With the exception of the above criteria, no other halt criterion will
		 // have an effect unless some minimum criteria have been met. E.g., if the
		 // minimum number of iterations, as defined by the user, hasn't been passwd,
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
		 if(maxIterationHaltset() && iterationHalt()) return true;

		 // Has the optimization stalled too often ?
		 if(stallHaltSet() && stallHalt()) return true;

		 // Do we have a scheduled halt time ? The comparatively expensive
		 // timedHalt() calculation is only called if m_maxDuration
		 // is at least one microsecond.
		 if(maxDurationHaltSet() && timedHalt(currentTime)) return true;

		 // Are we supposed to stop when the quality has exceeded a threshold ?
		 if(qualityThresholdHaltSet() && qualityHalt()) return true;

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
		 if(0 == m_maxIteration) return false;
		 else return true;
	 }

	 /***************************************************************************/
	 /**
	  * Check whether a halt criterion based on the number of stalls has been set
	  *
	  * @return A boolean indicating whether a halt criterion based on the number of stalls has been set
	  */
	 bool stallHaltSet() const {
		 if(0 == m_maxStallIteration) return false;
		 else return true;
	 }

	 /***************************************************************************/
	 /**
	  * Check whether the maxDuration-halt criterion has been set
	  *
	  * @return A boolean indication whether the max-duration halt criterion has been set
	  */
	 bool maxDurationHaltSet() const {
		 if(0. == m_maxDuration.count()) return false;
		 else return true;
	 }

	 /***************************************************************************/
	 /**
	  * Check whether the quality-threshold halt-criterion has been set
	  *
	  * @return A boolean indicating whether the quality-threshold halt-criterion has been set
	  */
	 bool qualityThresholdHaltSet() const {
		 return m_hasQualityThreshold;
	 }

	 /***************************************************************************/
	 /**
	  * Marks the globally best known fitness in all individuals
	  */
	 void markBestFitness() {
		 for(auto ind_ptr: *this) { ind_ptr->setBestKnownPrimaryFitness(this->getBestKnownPrimaryFitness()); }
	 }

	 /***************************************************************************/
	 /**
	  * Indicates whether the stallCounterThreshold_ has been exceeded
	  */
	 bool stallCounterThresholdExceeded() const {
		 return (m_stallCounter > m_stallCounterThreshold);
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an executor for the given execution mode
	  */
	 std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> createExecutor(const execMode& e) {
		 std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> executor_ptr;

		 switch(e) {
			 case execMode::SERIAL:
				 executor_ptr = std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>(new Gem::Courtier::GSerialExecutorT<GParameterSet>());
				 break;

			 case execMode::MULTITHREADED:
				 executor_ptr = std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>(new Gem::Courtier::GMTExecutorT<GParameterSet>());
				 break;

			 case execMode::BROKER:
				 std::cout << "Creating broker executor" << std::endl;
				 executor_ptr = std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>>(new Gem::Courtier::GBrokerExecutorT<GParameterSet>());
				 break;
		 }

		 return executor_ptr;
	 }

	 /***************************************************************************/

	 std::uint32_t m_iteration = 0; ///< The current iteration
	 std::uint32_t m_offset = DEFAULTOFFSET; ///< An iteration offset which can be used, if the optimization starts from a checkpoint file
	 std::uint32_t m_minIteration = DEFAULTMINIT; ///< The minimum number of iterations
	 std::uint32_t m_maxIteration = DEFAULTMAXIT; ///< The maximum number of iterations
	 std::uint32_t m_maxStallIteration = DEFAULTMAXSTALLIT; ///< The maximum number of generations without improvement, after which optimization is stopped
	 std::uint32_t m_reportIteration = DEFAULTREPORTITER; ///< The number of generations after which a report should be issued

	 std::size_t m_nRecordbestGlobalIndividuals = DEFNRECORDBESTINDIVIDUALS; ///< Indicates the number of best individuals to be recorded/updated in each iteration
	 GParameterSetFixedSizePriorityQueue m_bestGlobalIndividuals_pq {m_nRecordbestGlobalIndividuals, Gem::Common::LOWERISBETTER}; ///< A priority queue with the best individuals found so far
	 GParameterSetFixedSizePriorityQueue m_bestIterationIndividuals_pq {0, Gem::Common::LOWERISBETTER}; ///< A priority queue with the best individuals of a given iteration; unlimited size so all individuals of an iteration fit in

	 std::size_t m_defaultPopulationSize = DEFAULTPOPULATIONSIZE; ///< The nominal size of the population
	 std::tuple<double, double> m_bestKnownPrimaryFitness = std::tuple<double,double>(0.,0.); ///< Records the best primary fitness found so far
	 std::tuple<double, double> m_bestCurrentPrimaryFitness = std::tuple<double,double>(0.,0.); ///< Records the best fitness found in the current iteration

	 std::uint32_t m_stallCounter = 0; ///< Counts the number of iterations without improvement
	 std::uint32_t m_stallCounterThreshold = DEFAULTSTALLCOUNTERTHRESHOLD; ///< The number of stalls after which individuals are asked to update their internal data structures

	 std::int32_t m_cp_interval = DEFAULTCHECKPOINTIT; ///< Number of iterations after which a checkpoint should be written. -1 means: Write whenever an improvement was encountered
	 std::string m_cp_base_name = DEFAULTCPBASENAME; ///< The base name of the checkpoint file
	 std::string m_cp_directory = DEFAULTCPDIR; ///< The directory where checkpoint files should be stored
	 mutable std::string m_cp_last = "empty"; ///< The name of the last saved checkpoint
	 bool m_cp_remove = true; ///< Whether checkpoint files should be overwritten or kept
	 Gem::Common::serializationMode m_cp_serialization_mode = DEFAULTCPSERMODE; ///< Determines whether check-pointing should be done in text-, XML, or binary mode
	 double m_qualityThreshold = DEFAULTQUALITYTHRESHOLD; ///< A threshold beyond which optimization is expected to stop
	 bool m_hasQualityThreshold = false; ///< Specifies whether a qualityThreshold has been set
	 std::chrono::duration<double> m_maxDuration = Gem::Common::duration_from_string(DEFAULTDURATION); ///< Maximum time-frame for the optimization
	 std::chrono::duration<double> m_minDuration = Gem::Common::duration_from_string(DEFAULTMINDURATION); ///< Minimum time-frame for the optimization
	 mutable std::chrono::system_clock::time_point m_startTime; ///< Used to store the start time of the optimization. Declared mutable so the halt criteria can be const
	 std::string m_terminationFile = DEFAULTTERMINATIONFILE; ///< The name of a file which, when modified after the start of the optimization run, will cause termination of the run
	 bool m_terminateOnFileModification = false;
	 bool m_emitTerminationReason = DEFAULTEMITTERMINATIONREASON; ///< Specifies whether information about reasons for termination should be emitted
	 std::atomic<bool> m_halted { true }; ///< Set to true when halt() has returned "true"
	 std::vector<std::tuple<double, double>> m_worstKnownValids_vec; ///< Stores the worst known valid evaluations up to the current iteration (first entry: raw, second: tranformed)
	 std::vector<std::shared_ptr<GBasePluggableOMT<G_OA_BaseT>>> m_pluggable_monitors_vec; ///< A collection of monitors

	 std::shared_ptr<Gem::Courtier::GBaseExecutorT<GParameterSet>> m_executor_ptr; ///< Holds the current executor for this algorithm
	 execMode m_default_execMode = execMode::BROKER; ///< The default execution mode. Unless explicitöy requested by the user, we always go through the broker
	 std::string m_default_executor_config = "./config/GBrokerExecutor.json"; ///< The default configuration file for the broker executor

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
		 if(GObject::modify_GUnitTests()) result = true;
		 if(Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::modify_GUnitTests()) result = true;

		 // Try to change the objects contained in the collection
		 for(auto o: *this) {
			 if(o->modify_GUnitTests()) result = true;
		 }

		 this->setMaxIteration(this->getMaxIteration() + 1);
		 result = true;

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("G_OA_BaseT<>::modify_GUnitTests", "GEM_TESTING");
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
		 GObject::specificTestsNoFailureExpected_GUnitTests();
		 Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("G_OA_BaseT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
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
		 GObject::specificTestsFailuresExpected_GUnitTests();
		 Gem::Common::GStdPtrVectorInterfaceT<GParameterSet, Gem::Geneva::GObject>::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 condnotset("G_OA_BaseT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
};

/*******************************************************************************/
/////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// Some serialization-related exports and declarations

BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::G_OA_BaseT)

// TODO: Export base pluggable OMT

BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GBrokerExecutorT<Gem::Geneva::GParameterSet>)
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GSerialExecutorT<Gem::Geneva::GParameterSet>)
BOOST_CLASS_EXPORT_KEY(Gem::Courtier::GMTExecutorT<Gem::Geneva::GParameterSet>)

/******************************************************************************/

#endif /* GOPTIMIZATIONALGORITHMT_HPP_ */
