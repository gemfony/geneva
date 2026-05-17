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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Standard header files go here
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Boost header files go here
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "geneva/par/GBooleanAdaptor.hpp"
#include "geneva/par/GBooleanCollection.hpp"
#include "geneva/par/GConstrainedDoubleCollection.hpp"
#include "geneva/par/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/par/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/par/GDoubleBiGaussAdaptor.hpp"
#include "geneva/par/GDoubleCollection.hpp"
#include "geneva/par/GDoubleGaussAdaptor.hpp"
#include "geneva/par/GDoubleObjectCollection.hpp"
#include "geneva/par/GInt32FlipAdaptor.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva/par/GParameterSetMultiConstraint.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
// A number of default settings for the factory
const double GEEI_DEF_ADPROB = 1.0;
const double GEEI_DEF_ADAPTADPROB = 0.1;
const double GEEI_DEF_MINADPROB = 0.05;

const double GEEI_DEF_MAXADPROB = 1.;
const std::uint32_t GEEI_DEF_ADAPTIONTHRESHOLD = 1;
const bool GEEI_DEF_USEBIGAUSSIAN = false;
const double GEEI_DEF_SIGMA1 = 0.025;
const double GEEI_DEF_SIGMASIGMA1 = 0.2;
const double GEEI_DEF_MINSIGMA1 = 0.001;
const double GEEI_DEF_MAXSIGMA1 = 1;
const double GEEI_DEF_SIGMA2 = 0.025;
const double GEEI_DEF_SIGMASIGMA2 = 0.2;
const double GEEI_DEF_MINSIGMA2 = 0.001;
const double GEEI_DEF_MAXSIGMA2 = 1;
const double GEEI_DEF_DELTA = 0.2;
const double GEEI_DEF_SIGMADELTA = 0.2;
const double GEEI_DEF_MINDELTA = 0.001;
const double GEEI_DEF_MAXDELTA = 1.;
const std::size_t GEEI_DEF_PARDIM = 2;
const double GEEI_DEF_MINVAR = -10.;
const double GEEI_DEF_MAXVAR = 10.;
const bool GEEI_DEF_USECONSTRAINEDDOUBLECOLLECTION = false;
const std::string GEEI_DEF_PROGNAME = "./evaluator/evaluator.py";
const std::string GEEI_DEF_CUSTOMOPTIONS = "empty";
const std::string GEEI_DEF_PARFILEBASENAME = "parameter_file";
const std::size_t GEEI_DEF_NRESULTS = 1;
const std::string GEEI_DEF_STARTMODE = "random";
const std::string GEEI_DEF_DATATYPE = "setup_data";
const std::string GEEI_DEF_RUNID = "empty";
const bool GEEI_DEF_REMOVETEMPORARIES = "true";

/******************************************************************************/
// Forward declaration so we can expose the factory type to the public
// from within the GExternalEvaluatorIndividual
class GExternalEvaluatorIndividualFactory;

/******************************************************************************/
/**
 * This individual calls an external program to evaluate a given set of parameters.
 * Data exchange happens partially through the GNumericParameterT class. The
 * structure of the individual is determined from information given by the external
 * program. Currently double-, bool- and std::int32_t values are supported.
 *
 * External programs should understand at least the following command line
 * arguments with obvious meanings
 *
 * --init
 * --setup --init_values=[min/max/random] --output="setupFile.xml"
 * --evaluate --input="paramsFile.xml"   --output="result_file.xml"
 * --archive  --input="archiveFile.xml"
 * --finalize
 *
 * The xml parameter files are created using boost::property_tree and its write_xml
 * utility. Hence the external program needs to understand the XML format.
 */
class GExternalEvaluatorIndividual
  : public gpar::GParameterSet { // NOLINT(cppcoreguidelines-special-member-functions)
    ///////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gpar::GParameterSet) &
            BOOST_SERIALIZATION_NVP(program_name_) & BOOST_SERIALIZATION_NVP(custom_options_) &
            BOOST_SERIALIZATION_NVP(parameter_file_base_name_) &
            BOOST_SERIALIZATION_NVP(n_results_) &
            BOOST_SERIALIZATION_NVP(remove_exec_temporaries_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    using FACTORYTYPE = GExternalEvaluatorIndividualFactory;

    /** @brief The default constructor */
    GExternalEvaluatorIndividual();
    /** @brief A standard copy constructor */
    GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual &);

    /** @brief The standard destructor */
    ~GExternalEvaluatorIndividual() override;

    /** @brief Sets the name of the external evaluation program */
    void setProgramName(const std::string &);
    /** @brief Retrieves the name of the external evaluation program */
    std::string getProgramName() const;

    /** @brief Sets any custom options that need to be passed to the external evaluation program */
    void setCustomOptions(const std::string &);
    /** @brief Retrieves any custom options that need to be passed to the external evaluation program */
    std::string getCustomOptions() const;

    /** @brief Sets the base name of the data exchange file */
    void setExchangeBaseName(const std::string &);
    /** @brief Retrieves the current value of the parameterFileBaseName_ variable */
    std::string getExchangeBaseName() const;

    /** @brief Sets the number of results to be expected from the external evaluation program */
    void setNExpectedResults(const std::size_t &);
    /** @brief Retrieves the number of results to be expected from the external evaluation program */
    std::size_t getNExpectedResults() const;

    /** @brief Allows to set the data type of this individual */
    void setDataType(std::string);
    /** @brief Allows to retrieve the data type of this individual */
    std::string getDataType() const;

    /** @brief Allows to assign a run-id to this individual */
    void setRunId(std::string);
    /** @brief Allows to retrieve the run-id assigned to this individual */
    std::string getRunId() const;

    /** @brief Allows to specify whether temporary files should be removed */
    void setRemoveExecTemporaries(bool);
    /** @brief Allows to check whether temporaries should be removed */
    bool getRemoveExecTemporaries() const;

protected:
    /***************************************************************************/
    /** @brief Loads the data of another GExternalEvaluatorIndividual */
    void load_(const GObject *) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GExternalEvaluatorIndividual>(
        GExternalEvaluatorIndividual const &,
        GExternalEvaluatorIndividual const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The actual fitness calculation takes place here */
    double fitnessCalculation() final;

private:
    /***************************************************************************/

    /** @brief Creates a deep clone of this object */
    GObject *clone_() const final;

    /***************************************************************************/

    std::string program_name_; ///< The name of the external program to be executed
    std::string
        custom_options_; ///< Any custom options that need to be provided to the external program
    std::string parameter_file_base_name_; ///< The base name to be assigned to the parameter_file
    std::size_t n_results_; ///< The number of results to be expected from the evaluation function
    std::string runID_;      ///< Identifies this run with a unique id
    bool remove_exec_temporaries_; ///< Indicates whether temporary files should be removed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GExternalEvaluatorIndividual objects
 */
class GExternalEvaluatorIndividualFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GFactoryT<gpar::GParameterSet> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive &ar, const unsigned int) {
        using namespace Gem::Common;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GFactoryT<gpar::GParameterSet>) &
            BOOST_SERIALIZATION_NVP(adProb_) & BOOST_SERIALIZATION_NVP(adaptAdProb_) &
            BOOST_SERIALIZATION_NVP(minAdProb_) & BOOST_SERIALIZATION_NVP(maxAdProb_) &
            BOOST_SERIALIZATION_NVP(adaptionThreshold_) &
            BOOST_SERIALIZATION_NVP(useBiGaussian_) & BOOST_SERIALIZATION_NVP(sigma1_) &
            BOOST_SERIALIZATION_NVP(sigmaSigma1_) & BOOST_SERIALIZATION_NVP(minSigma1_) &
            BOOST_SERIALIZATION_NVP(maxSigma1_) & BOOST_SERIALIZATION_NVP(sigma2_) &
            BOOST_SERIALIZATION_NVP(sigmaSigma2_) & BOOST_SERIALIZATION_NVP(minSigma2_) &
            BOOST_SERIALIZATION_NVP(maxSigma2_) & BOOST_SERIALIZATION_NVP(delta_) &
            BOOST_SERIALIZATION_NVP(sigmaDelta_) & BOOST_SERIALIZATION_NVP(minDelta_) &
            BOOST_SERIALIZATION_NVP(maxDelta_) & BOOST_SERIALIZATION_NVP(programName_) &
            BOOST_SERIALIZATION_NVP(customOptions_) &
            BOOST_SERIALIZATION_NVP(parameterFileBaseName_) &
            BOOST_SERIALIZATION_NVP(initValues_) &
            BOOST_SERIALIZATION_NVP(removeExecTemporaries_) &
            BOOST_SERIALIZATION_NVP(externalEvaluatorQueried_) & BOOST_SERIALIZATION_NVP(ptr_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    explicit GExternalEvaluatorIndividualFactory(std::filesystem::path const &);
    /** @brief The copy constructor */
    
    GExternalEvaluatorIndividualFactory(const GExternalEvaluatorIndividualFactory &);

    /** @brief The destructor */
    ~GExternalEvaluatorIndividualFactory() override;

    /**************************************************************************/
    // Getters and setters

    /** @brief Allows to retrieve the adaptionThreshold_ variable */
    std::uint32_t getAdaptionThreshold() const;
    /** @brief Set the value of the adaptionThreshold_ variable */
    void setAdaptionThreshold(std::uint32_t adaption_threshold);

    /** @brief Allows to retrieve the adProb_ variable */
    double getAdProb() const;
    /** @brief Set the value of the adProb_ variable */
    void setAdProb(double ad_prob);

    /** @brief Allows to retrieve the rate of evolutionary adaption of adProb_ */
    double getAdaptAdProb() const;
    /** @brief Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature) */
    void setAdaptAdProb(double adapt_ad_prob);

    /** @brief Allows to retrieve the allowed range for adProb_ variation */
    std::tuple<double, double> getAdProbRange() const;
    /** @brief Allows to set the allowed range for adaption probability variation */
    void setAdProbRange(double min_ad_prob, double max_ad_prob);

    /** @brief Allows to retrieve the useBiGaussian_ variable */
    bool getUseBiGaussian() const;
    /** @brief Set the value of the useBiGaussian_ variable */
    void setUseBiGaussian(bool use_bi_gaussian);

    /** @brief Allows to retrieve the delta_ variable */
    double getDelta() const;
    /** @brief Set the value of the delta_ variable */
    void setDelta(double delta);
    /** @brief Allows to retrieve the minDelta_ variable */
    double getMinDelta() const;
    /** @brief Allows to retrieve the maxDelta_ variable */
    double getMaxDelta() const;
    /** @brief Allows to retrieve the allowed value range of delta */
    std::tuple<double, double> getDeltaRange() const;
    /** @brief Allows to set the allowed value range of delta */
    void setDeltaRange(std::tuple<double, double>);

    /** @brief Allows to retrieve the minSigma1_ variable */
    double getMinSigma1() const;
    /** @brief Allows to retrieve the maxSigma1_ variable */
    double getMaxSigma1() const;
    /** @brief Allows to retrieve the allowed value range of sigma1_ */
    std::tuple<double, double> getSigma1Range() const;
    /** @brief Allows to set the allowed value range of sigma1_ */
    void setSigma1Range(std::tuple<double, double>);

    /** @brief Allows to retrieve the minSigma2_ variable */
    double getMinSigma2() const;
    /** @brief Allows to retrieve the maxSigma2_ variable */
    double getMaxSigma2() const;
    /** @brief Allows to retrieve the allowed value range of sigma2_ */
    std::tuple<double, double> getSigma2Range() const;
    /** @brief Allows to set the allowed value range of sigma2_ */
    void setSigma2Range(std::tuple<double, double>);

    /** @brief Allows to retrieve the sigma1_ variable */
    double getSigma1() const;
    /** @brief Set the value of the sigma1_ variable */
    void setSigma1(double sigma1);

    /** @brief Allows to retrieve the sigma2_ variable */
    double getSigma2() const;
    /** @brief Set the value of the sigma2_ variable */
    void setSigma2(double sigma2);

    /** @brief Allows to retrieve the sigmaDelta_ variable */
    double getSigmaDelta() const;
    /** @brief Set the value of the sigmaDelta_ variable */
    void setSigmaDelta(double sigma_delta);

    /** @brief Allows to retrieve the sigmaSigma1_ variable */
    double getSigmaSigma1() const;
    /** @brief Set the value of the sigmaSigma1_ variable */
    void setSigmaSigma1(double sigma_sigma1);

    /** @brief Allows to retrieve the sigmaSigma2_ variable */
    double getSigmaSigma2() const;
    /** @brief Set the value of the sigmaSigma2_ variable */
    void setSigmaSigma2(double sigma_sigma2);

    /** @brief Allows to set the name and path of the external program */
    void setProgramName(std::string);
    /** @brief Allows to retrieve the name of the external program */
    std::string getProgramName() const;

    /** @brief Sets any custom options that need to be passed to the external evaluation program */
    void setCustomOptions(const std::string);
    /** @brief Retrieves any custom options that need to be passed to the external evaluation program */
    std::string getCustomOptions() const;

    /** @brief Allows to set the base name of the parameter file */
    void setParameterFileBaseName(std::string);
    /** @brief Allows to retrieve the base name of the parameter file */
    std::string getParameterFileBaseName() const;

    /** @brief Indicates the initialization mode */
    void setInitValues(std::string);
    /** @brief Allows to retrieve the initialization mode */
    std::string getInitValues() const;

    /** @brief Allows to specify whether temporary files should be removed */
    void setRemoveExecTemporaries(bool);
    /** @brief Allows to check whether temporaries should be removed */
    bool getRemoveExecTemporaries() const;

    // End of public getters and setters
    /**************************************************************************/

    /** @brief Submit work items to the external executable for archiving */
    void archive(
        const std::vector<std::shared_ptr<GExternalEvaluatorIndividual>

                          > &arch
    ) const;

    /** @brief Loads the data of another GFunctionIndividualFactory object */
    void load(std::shared_ptr<Gem::Common::GFactoryT<gpar::GParameterSet>>) override;

    /** @brief Creates a deep clone of this object */
    std::shared_ptr<Gem::Common::GFactoryT<gpar::GParameterSet>> clone() const override;

protected:
    /** @brief Allows to describe local configuration options in derived classes */
    void describeLocalOptions_(Gem::Common::GParserBuilder &) override;

    /** @brief Allows to act on the configuration options received from the configuration file */
    void postProcess_(std::shared_ptr<gpar::GParameterSet> &) override;

private:
    /** @brief Creates individuals of this type */
    std::shared_ptr<gpar::GParameterSet>
    getObject_(Gem::Common::GParserBuilder &, const std::size_t &) override;

    /** @brief Sets up the boost property object holding information about the individual structure */
    void setUpPropertyTree();

    /** @brief Set the value of the minDelta_ variable */
    void setMinDelta(double min_delta);

    /** @brief Set the value of the maxDelta_ variable */
    void setMaxDelta(double max_delta);

    /** @brief Set the value of the minSigma1_ variable */
    void setMinSigma1(double min_sigma1);

    /** @brief Set the value of the maxSigma1_ variable */
    void setMaxSigma1(double max_sigma1);

    /** @brief Set the value of the minSigma2_ variable */
    void setMinSigma2(double min_sigma2);

    /** @brief Set the value of the maxSigma2_ variable */
    void setMaxSigma2(double max_sigma2);

    /** @brief The default constructor; Only needed for (de-)serialization purposes, hence empty. */
    GExternalEvaluatorIndividualFactory();

    Gem::Common::GOneTimeRefParameterT<double> adProb_;
    Gem::Common::GOneTimeRefParameterT<double> adaptAdProb_;
    Gem::Common::GOneTimeRefParameterT<double> minAdProb_;
    Gem::Common::GOneTimeRefParameterT<double> maxAdProb_;
    Gem::Common::GOneTimeRefParameterT<std::uint32_t> adaptionThreshold_;
    Gem::Common::GOneTimeRefParameterT<bool> useBiGaussian_;
    Gem::Common::GOneTimeRefParameterT<double> sigma1_;
    Gem::Common::GOneTimeRefParameterT<double> sigmaSigma1_;
    Gem::Common::GOneTimeRefParameterT<double> minSigma1_;
    Gem::Common::GOneTimeRefParameterT<double> maxSigma1_;
    Gem::Common::GOneTimeRefParameterT<double> sigma2_;
    Gem::Common::GOneTimeRefParameterT<double> sigmaSigma2_;
    Gem::Common::GOneTimeRefParameterT<double> minSigma2_;
    Gem::Common::GOneTimeRefParameterT<double> maxSigma2_;
    Gem::Common::GOneTimeRefParameterT<double> delta_;
    Gem::Common::GOneTimeRefParameterT<double> sigmaDelta_;
    Gem::Common::GOneTimeRefParameterT<double> minDelta_;
    Gem::Common::GOneTimeRefParameterT<double> maxDelta_;

    Gem::Common::GOneTimeRefParameterT<std::string> programName_;
    Gem::Common::GOneTimeRefParameterT<std::string> customOptions_;
    Gem::Common::GOneTimeRefParameterT<std::string> parameterFileBaseName_;
    Gem::Common::GOneTimeRefParameterT<std::string> initValues_;

    Gem::Common::GOneTimeRefParameterT<bool> removeExecTemporaries_;

    bool
        externalEvaluatorQueried_; ///< Specifies whether the external evaluator program has already been queried for setup information
    pt::ptree
        ptr_; ///< Holds setup information for individuals, as provided by the external evaluator program
};

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GExternalEvaluatorIndividual)        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GExternalEvaluatorIndividualFactory) // NOLINT
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
