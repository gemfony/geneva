/********************************************************************************
*
* This file is part of the Geneva library collection. The following license
* applies to this file:
*
* ------------------------------------------------------------------------------
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* ------------------------------------------------------------------------------
*
* Note that other files in the Geneva library collection may use a different
* license. Please see the licensing information in each file.
*
********************************************************************************
*
* Geneva was started by Dr. Rüdiger Berlich and was later maintained together
* with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
* information on Gemfony scientific, see http://www.gemfomy.eu .
*
* The majority of files in Geneva was released under the Apache license v2.0
* in February 2020.
*
* See the NOTICE file in the top-level directory of the Geneva library
* collection for a list of contributors and copyright information.
*
********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <filesystem>
#include <fstream>
#include <type_traits>
#include <chrono>
#include <utility>

// Boost header files go here

// Geneva headers go here
#include "../common/GCommonEnums.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GLogger.hpp"
#include "common/GPlotDesigner.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GParameterPropertyParser.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"

namespace Gem::Geneva::Error {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This struct holds all parameter- and evaluation-data associated with an
 * individual. It does intentionally not hold any positional data (i.e.
 * which position it had in a population, whether it was a parent or a child
 * (in the case of evolutionary algorithms), or which iteration it belonged to.
 * As these structs will be collected in an array for each iteration, and these
 * arrays in turn become part of a larger array, such data can be derived from
 * the output itself.
 */
struct individual_data {
private:
  ///////////////////////////////////////////////////////////////////////
  friend class boost::serialization::access;

  template<typename Archive>
  void serialize(Archive & ar, const unsigned int){
    using boost::serialization::make_nvp;

    ar
    & BOOST_SERIALIZATION_NVP(m_double_parameters)
    & BOOST_SERIALIZATION_NVP(m_float_parameters)
    & BOOST_SERIALIZATION_NVP(m_int_parameters)
    & BOOST_SERIALIZATION_NVP(m_bool_parameters)
    & BOOST_SERIALIZATION_NVP(m_evaluations)
    & BOOST_SERIALIZATION_NVP(m_is_processed)
    & BOOST_SERIALIZATION_NVP(m_has_errors);
  }
  ///////////////////////////////////////////////////////////////////////

public:
  /**
   * Initialisation of the data from a given individual
   * @param ind The individual to be used as the basis
   */
  individual_data(Gem::Geneva::GParameterSet const& ind) {
     // Retrieve all data
     ind.streamline<double>(m_double_parameters, Gem::Geneva::activityMode::ALLPARAMETERS);
     ind.streamline<float>(m_float_parameters, Gem::Geneva::activityMode::ALLPARAMETERS);
     ind.streamline<std::int32_t>(m_int_parameters, Gem::Geneva::activityMode::ALLPARAMETERS);
     ind.streamline<bool>(m_bool_parameters, Gem::Geneva::activityMode::ALLPARAMETERS);

     // Retrieve the evaluations
     m_evaluations = ind.getStoredResults();

     // Retrieve the processing status
     m_is_processed = ind.is_processed();
     m_has_errors = ind.has_errors();
  }

  individual_data(individual_data&& ind) {
      m_double_parameters = std::move(ind.m_double_parameters);
      m_float_parameters = std::move(ind.m_float_parameters);
      m_int_parameters = std::move(ind.m_int_parameters);
      m_bool_parameters = std::move(ind.m_bool_parameters);
      m_evaluations = std::move(ind.m_evaluations);
      m_is_processed = ind.m_is_processed; ind.m_is_processed = false;
      m_has_errors = ind.m_has_errors; ind.m_has_errors = false;
  }

  individual_data(individual_data const&) = delete;
  individual_data& operator=(individual_data const&) = delete;
  individual_data& operator=(individual_data&&) = delete;

  //=====================================================================
  // Data

  std::vector<double> m_double_parameters;
  std::vector<float> m_float_parameters;
  std::vector<std::int32_t> m_int_parameters;
  std::vector<bool> m_bool_parameters;

  std::vector<parameterset_processing_result> m_evaluations; ///< one or more evaluations (raw and transformed), with the first one representing the main evaluation

  bool m_is_processed{false}; ///< will be set to "true" if the individual has not been evaluationed yet
  bool m_has_errors{false}; ///< will be set to "true" if errors were flagged for this individual

private:
  individual_data() = default; ///< Only needed for (de-)serialization purposes
};

/**
 * This class holds all individual data of a given iteration.
 */
 class iteration_data {
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
     using boost::serialization::make_nvp;

     ar
         & BOOST_SERIALIZATION_NVP(m_iteration_individuals)
         & BOOST_SERIALIZATION_NVP(m_iteration);
   }
   ///////////////////////////////////////////////////////////////////////

 public:
   explicit iteration_data(std::uint32_t iteration)
       : m_iteration(iteration)
   { /* nothing */ }

   iteration_data(std::vector<Gem::Geneva::GParameterSet> const& inds, std::uint32_t iteration)
       : m_iteration(iteration)
   {
      for(const auto& ind: inds) {
        m_iteration_individuals.emplace_back(individual_data(ind));
      }
   }

   iteration_data(iteration_data && it_data) {
     m_iteration_individuals = std::move(it_data.m_iteration_individuals);
     m_iteration = it_data.m_iteration;
     it_data.m_iteration = 0;
     it_data.m_pos = 0;
   }

   iteration_data(iteration_data const&) = delete;
   iteration_data& operator=(iteration_data const&) = delete;
   iteration_data& operator=(iteration_data&&) = delete;

   void add(individual_data&& ind) {
     m_iteration_individuals.emplace_back(std::move(ind));
   }

   /**
    * Retrieves the number of data sets in our collection
    */
   std::size_t nDataSets() const {
     return m_iteration_individuals.size();
   }

   /**
    * Resets the individual counter
    */
   void resetCounter() { m_pos = 0; }

   /**
    * Return the next individual struct in the list. Note that the
    * function will throw in case we try to access individuals beyond
    * the end of the list.
    *
    * @return The next individual struct in the list
    */
   const auto& getNextInd() const {
     return m_iteration_individuals.at(m_pos++);
   }

   /**
    * Returns the current iteration
    *
    * @return The current iteration
    */
   std::uint32_t getIteration() const {
     return m_iteration;
   }

 private:
   iteration_data() = default; ///< Only needed for (de-)serialization purposes

   std::vector<individual_data> m_iteration_individuals; ///< a collection of all data of all individuals of an iteration
   std::uint32_t m_iteration{0}; ///< The iteration the data belongs to

   mutable std::size_t m_pos{0}; ///< Position of the next get
 };

 /**
  * This class holds all parameter- and evaluation-data of the individuals of a predefined
  * number of iterations. It has the ability to flush out the data in regular intervals, if
  * desired by the user. An interval of 0 means: only flush out once upon destruction.
  * A positive number means that only a maximum of iterations is stored, before it is
  * flushed out to a file. No data for older iterations will be retained in this case.
  * Output-files are given a consecutive number, added to a base-name.
  */
 class run_data_out {
 public:
   /**
    * The default constructor
    */
   run_data_out() = delete;

   /**
    * Sets the base filename and the number of iterations after which
    * data should be flushed out to disk
    *
    * @param dataFile
    * @param flush_iterations
    */
   run_data_out(std::filesystem::path const& dataFile, std::uint32_t flush_iterations) {

   }

   /**
    * The destructor. Writes out all stored iteration data to disk.
    */
   ~run_data_out() {

   }

   /**
    * Sets the base name for the iteration data to be flushed out
    * @param base_filename
    */
   void setBaseFilename(std::filesystem::path const& base_filename) { m_base_filename = base_filename; }

   /**
    * Returns the base filename
    *
    * @return The base filename
    */
   const std::filesystem::path& getBaseFilename() const { return m_base_filename; }

   /**
    * Returns the current file counter
    * @return The current file counter
    */
   std::size_t getFileCounter() const { return m_file_counter; }

   /**
    * Add the data of an iteration to this object.
    *
    * @param it_data The data belonging to a given iteration
    */
   void add(iteration_data&& it_data) {
     m_iteration_data.emplace_back(std::move(it_data));
   }

 private:
   /**
    * Flushes the iteration data to the disk
    */
   void flush_() {

   }

   std::vector<iteration_data> m_iteration_data; ///< a collection of all data of all individuals of the last m_flush_iterations iterations

   std::filesystem::path m_base_filename{"iteration_data"};
   std::size_t m_file_counter{0}; ///< Used for the name modifier of flush files
   std::uint32_t m_flush_iterations{0}; ///< The intervals in which iteration data should be written to files

   Gem::Common::serializationMode m_serializationMode; ///< The serialization mode used to write out data
 };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log all candidate solutions found to files, including the parameter
 * values. NOTE that the file may become very large! R
 */
class GErrorDataFileLogger
    : public GBasePluggableOM
{
  ///////////////////////////////////////////////////////////////////////
  friend class boost::serialization::access;

  template<typename Archive>
  void serialize(Archive & ar, const unsigned int){
    using boost::serialization::make_nvp;

    ar
        & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
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

  /** @brief The default constructor */
  G_API_GENEVA GErrorDataFileLogger() = default;
  /** @brief Initialization with a file name */
  explicit G_API_GENEVA GErrorDataFileLogger(std::string fileName);
  /** @brief Initialization with a file name and boundaries */
  G_API_GENEVA GErrorDataFileLogger(
      std::string  fileName
      , std::vector<double>  boundaries
  );
  /** @brief The copy constructor */
  G_API_GENEVA GErrorDataFileLogger(const GErrorDataFileLogger& cp) = default;
  /** @brief The destructor */
  G_API_GENEVA  ~GErrorDataFileLogger() override = default;

  /** @brief Sets the file name */
  G_API_GENEVA void setFileName(const std::string& fileName);
  /** @brief Retrieves the current file name */
  G_API_GENEVA [[nodiscard]] std::string getFileName() const;

  /** @brief Sets the boundaries */
  G_API_GENEVA void setBoundaries(const std::vector<double>& boundaries);
  /** @brief Allows to retrieve the boundaries */
  G_API_GENEVA [[nodiscard]] std::vector<double> getBoundaries() const;
  /** @brief Allows to check whether boundaries are active */
  G_API_GENEVA [[nodiscard]] bool boundariesActive() const;
  /** @brief Allows to inactivate boundaries */
  G_API_GENEVA void setBoundariesInactive();

  /** @brief  Allows to specify whether explanations should be printed for parameter- and fitness values. */
  G_API_GENEVA void setPrintWithNameAndType(bool withNameAndType = true);
  /** @brief Allows to check whether explanations should be printed for parameter-and fitness values */
  G_API_GENEVA [[nodiscard]] bool getPrintWithNameAndType() const;

  /** @brief Allows to specify whether commas should be printed in-between values */
  G_API_GENEVA void setPrintWithCommas(bool withCommas = true);
  /** @brief Allows to check whether commas should be printed in-between values */
  G_API_GENEVA [[nodiscard]] bool getPrintWithCommas() const;

  /** @brief Allows to specify whether the true (instead of the transformed) fitness should be shown */
  G_API_GENEVA void setUseTrueFitness(bool useRawFitness = true);
  /** @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown */
  G_API_GENEVA [[nodiscard]] bool getUseTrueFitness() const;

  /** @brief Allows to specify whether the validity of a solution should be shown */
  G_API_GENEVA void setShowValidity(bool showValidity = true);
  /** @brief Allows to check whether the validity of a solution will be shown */
  G_API_GENEVA  [[nodiscard]] bool getShowValidity() const;

  /** @brief Allows to specifiy whether the initial population should be printed. */
  G_API_GENEVA void setPrintInitial(bool printInitial = true);
  /** @brief Allows to check whether the initial population should be printed. */
  G_API_GENEVA [[nodiscard]] bool getPrintInitial() const;

  /** @brief Allows to specifiy whether a comment line should be inserted between iterations */
  G_API_GENEVA void setShowIterationBoundaries(bool showIterationBoundaries = true);
  /** @brief Allows to check whether a comment line should be inserted between iterations */
  G_API_GENEVA [[nodiscard]] bool getShowIterationBoundaries() const;

protected:
  /************************************************************************/

  /** @brief Loads the data of another object */
  G_API_GENEVA  void load_(const GObject* cp) override;

  /** @brief Allow access to this classes compare_ function */
  friend void Gem::Common::compare_base_t<GErrorDataFileLogger>(
      GErrorDataFileLogger const &
      , GErrorDataFileLogger const &
      , Gem::Common::GToken &
  );

  /** @brief Searches for compliance with expectations with respect to another object of the same type */
  G_API_GENEVA  void compare_(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
  ) const override;

  /** @brief Applies modifications to this object */
  G_API_GENEVA  bool modify_GUnitTests_() override;
  /** @brief Performs self tests that are expected to succeed */
  G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests_() override;
  /** @brief Performs self tests that are expected to fail */
  G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests_() override;

private:
  /***************************************************************************/
  /** @brief Emits a name for this class / object */
  G_API_GENEVA  [[nodiscard]] std::string name_() const override;
  /** @brief Creates a deep clone of this object */
  G_API_GENEVA  [[nodiscard]] GObject* clone_() const override;

  /** @brief Allows to emit information in different stages of the information cycle */
  G_API_GENEVA  void informationFunction_(
      infoMode
      , G_OptimizationAlgorithm_Base const * const goa
      ) override;

  /** @brief Does the actual printing */
  G_API_GENEVA void printPopulation(
      const std::string& iterationDescription
      , G_OptimizationAlgorithm_Base const * const goa
  );

  /***************************************************************************/
  // Data

  std::string m_fileName = "CompleteSolutionLog.txt"; ///< The name of the file to which solutions should be stored
  std::vector<double> m_boundaries; ///< Value boundaries used to filter logged solutions
  bool m_boundariesActive = false; ///< Set to true if boundaries have been set
  bool m_withNameAndType = false; ///< When set to true, explanations for values are printed
  bool m_withCommas = false; ///< When set to true, commas will be printed in-between values
  bool m_useRawFitness = true; ///< Indicates whether true- or transformed fitness should be output
  bool m_showValidity = true; ///< Indicates whether the validity of a solution should be shown
  bool m_printInitial = false; ///< Indicates whether the initial population should be printed
  bool m_showIterationBoundaries = false; ///< Indicates whether a comment indicating the end of an iteration should be printed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Error */