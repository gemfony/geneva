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

#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GExternalEvaluatorIndividual)        // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GExternalEvaluatorIndividualFactory) // NOLINT
namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual()
  : program_name_(GEEI_DEF_PROGNAME)
  , custom_options_(GEEI_DEF_CUSTOMOPTIONS)
  , parameter_file_base_name_(GEEI_DEF_PARFILEBASENAME)
  , n_results_(GEEI_DEF_NRESULTS)
  , runID_(GEEI_DEF_RUNID)
  , remove_exec_temporaries_(GEEI_DEF_REMOVETEMPORARIES) { /* nothing */
}

/******************************************************************************/
/**
 * A standard copy constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual &cp)
  : GParameterSet(cp) // copies all local collections
  , program_name_(cp.program_name_)
  , custom_options_(cp.custom_options_)
  , parameter_file_base_name_(cp.parameter_file_base_name_)
  , n_results_(cp.n_results_)
  , runID_(cp.runID_)
  , remove_exec_temporaries_(cp.remove_exec_temporaries_) { /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GExternalEvaluatorIndividual::~GExternalEvaluatorIndividual() { /* nothing */
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GExternalEvaluatorIndividual::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    // Check that we are dealing with a GExternalEvaluatorIndividual reference independent of this object and convert the pointer
    const GExternalEvaluatorIndividual *p_load =
        Gem::Common::g_convert_and_compare<GObject, GExternalEvaluatorIndividual>(cp, this);

    Gem::Common::GToken token("GExternalEvaluatorIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<GParameterSet>(*this, *p_load, token);

    // ... and then the local data
    Gem::Common::compare_t(IDENTITY(program_name_, p_load->program_name_), token);
    Gem::Common::compare_t(IDENTITY(custom_options_, p_load->custom_options_), token);
    Gem::Common::compare_t(
        IDENTITY(parameter_file_base_name_, p_load->parameter_file_base_name_),
        token
    );
    Gem::Common::compare_t(IDENTITY(n_results_, p_load->n_results_), token);
    Gem::Common::compare_t(IDENTITY(runID_, p_load->runID_), token);
    Gem::Common::compare_t(
        IDENTITY(remove_exec_temporaries_, p_load->remove_exec_temporaries_),
        token
    );

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividual::setProgramName(const std::string &program_name) {
    program_name_ = program_name;
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividual::getProgramName() const {
    return program_name_;
}

/******************************************************************************/
/**
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividual::setCustomOptions(const std::string &custom_options) {
    custom_options_ = custom_options;
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividual::getCustomOptions() const {
    return custom_options_;
}

/******************************************************************************/
/**
 * Sets the base name of the data exchange file. Note that the individual might add additional
 * characters in order to distinguish between the exchange files of different individuals.
 *
 * @param parameter_file The desired new base name of the exchange file
 */
void GExternalEvaluatorIndividual::setExchangeBaseName(const std::string &parameter_file) {
    if(parameter_file.empty() || parameter_file == "empty") {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividual::setExchangeBaseName(): Error!" << '\n'
            << "Invalid file name \"" << parameter_file << "\"" << '\n'
        );
    }

    parameter_file_base_name_ = parameter_file;
}

/******************************************************************************/
/**
 * Retrieves the current value of the parameterFileBaseName_ variable.
 *
 * @return The current base name of the exchange file
 */
std::string GExternalEvaluatorIndividual::getExchangeBaseName() const {
    return parameter_file_base_name_;
}

/******************************************************************************/
/**
 * Sets the number of results to be expected from the external evaluation program
 */
void GExternalEvaluatorIndividual::setNExpectedResults(const std::size_t &n_results) {
    if(0 == n_results) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividual::setNExpectedResults(): Error!" << '\n'
            << "Got invalid number of expected results: " << n_results << '\n'
        );
    }

    n_results_ = n_results;
}

/******************************************************************************/
/**
 * Retrieves the number of results to be expected from the external evaluation program
 */
std::size_t GExternalEvaluatorIndividual::getNExpectedResults() const {
    return n_results_;
}

/******************************************************************************/
/**
 * Loads the data of another GExternalEvaluatorIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GExternalEvaluatorIndividual, camouflaged as a GObject
 */
void GExternalEvaluatorIndividual::load_(const GObject *cp) {
    // Check that we are dealing with a GExternalEvaluatorIndividual reference independent of this object and convert the pointer
    const GExternalEvaluatorIndividual *p_load =
        Gem::Common::g_convert_and_compare<GObject, GExternalEvaluatorIndividual>(cp, this);

    // First load the data of our parent class ...
    GParameterSet::load_(cp);

    // ... and then our own
    program_name_ = p_load->program_name_;
    custom_options_ = p_load->custom_options_;
    parameter_file_base_name_ = p_load->parameter_file_base_name_;
    n_results_ = p_load->n_results_;
    runID_ = p_load->runID_;
    remove_exec_temporaries_ = p_load->remove_exec_temporaries_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject *GExternalEvaluatorIndividual::clone_() const {
    return new GExternalEvaluatorIndividual(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place in an external program. Here we just
 * write a file with the required parameters to disk and execute the program.
 *
 * @return The primary value of this object
 */
double GExternalEvaluatorIndividual::fitnessCalculation() {
    // Transform this object into a boost property tree
    boost::property_tree::ptree ptr_out; // NOLINT(cppcoreguidelines-init-variables)

    std::string batch = "batch";

    // Output the header data
    ptr_out.put(batch + ".dataType", std::string("run_parameters"));
    ptr_out.put(batch + ".run_id", this->getRunId());
    ptr_out.put(batch + ".n_individuals", static_cast<std::size_t>(1));

    std::string basename = batch + ".individuals.individual0";
    this->toPropertyTree(ptr_out, basename);

    // Create a suitable extension and exchange file names for this object
    std::string extension = std::string("-") +
                            Gem::Common::to_string(this->getAssignedIteration()) + "-" +
                            Gem::Common::to_string(this);
    std::string parameterfile_name = parameter_file_base_name_ + extension + ".xml";
    std::string result_file_name = std::string("result") + extension + ".xml";
    std::string command_output_file_name = std::string("commandOutput") + extension + ".txt";

    // RAII guard: remove the three IPC temp files on scope exit, whether normal or via exception.
    // remove() is a no-op for non-existent files (e.g. result_file_name when the external
    // program never ran), so all three can be listed unconditionally.
    struct TempFileGuard {
        const std::string &param, &result, &cmdOut;
        bool active;
        ~TempFileGuard() {
            if (!active) return;
            std::filesystem::remove(param);
            std::filesystem::remove(result);
            std::filesystem::remove(cmdOut);
        }
    } temp_guard{
        parameterfile_name,
        result_file_name,
        command_output_file_name,
        remove_exec_temporaries_
    };

    // Save the parameters to a file for the external evaluation
    boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
    boost::property_tree::write_xml(parameterfile_name, ptr_out, std::locale(), settings);

    // Collect all command-line arguments
    std::vector<std::string> arguments;
    if(custom_options_ != "empty" && not custom_options_.empty()) {
        arguments.push_back(custom_options_);
    }
    arguments.emplace_back("--evaluate");
    arguments.push_back(std::string("--input=\"") + parameterfile_name + "\"");
    arguments.push_back(std::string("--output=\"") + result_file_name + "\"");

    // Perform the external evaluation
    double main_result = 0.;
    std::string command;
    int error_code = Gem::Common::runExternalCommand(
        std::filesystem::path(program_name_),
        arguments,
        std::filesystem::path(command_output_file_name),
        command
    );

    if(error_code) {                      // Something went wrong
        std::ostringstream error_message; // NOLINT(cppcoreguidelines-init-variables)

        error_message << "In GExternalEvaluatorIndividual::fitnessCalculation():" << '\n'
                      << "Execution of external command failed." << '\n'
                      << "Command: " << command << '\n'
                      << "Error code: " << error_code << '\n'
                      << "Program output:" << '\n'
                      << Gem::Common::loadTextDataFromFile(command_output_file_name) << '\n';

#ifdef DEBUG
        glogger << error_message.str() << GWARNING;
#endif

        // O.k., so the external application crashed or returned an error.
        // All we can do here is to return the worst case. As long as crashes
        // do not happen too often, this will have but little influence on
        // the optimization.
        main_result = this->getWorstCase();
        for(std::size_t res = 1; res < n_results_; res++) {
            this->setResult(res, this->getWorstCase());
        }

        // Make sure the individual can be recognized as invalid by Geneva
        this->force_set_error(error_message.str());
    }
    else { // Everything is o.k., lets retrieve the evaluation
        // Check that the result file exists
        if(not std::filesystem::exists(result_file_name)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << '\n'
                << "Result file " << result_file_name << " does not seem to exist." << '\n'
            );
        }

        // Parse the results
        boost::property_tree::ptree
            ptr_in; // A property tree object; // NOLINT(cppcoreguidelines-init-variables)
        try {
            pt::read_xml(result_file_name, ptr_in);
        }
        catch(const boost::property_tree::xml_parser::xml_parser_error &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error  " << '\n'
                << "Caught boost::property_tree::xml_parser::xml_parser_error" << '\n'
                << "for file " << e.filename() << " (line " << e.line() << ")" << '\n'
            );
        }
        catch(const std::exception &e) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error reading "
                << result_file_name << '\n'
                << "with message " << e.what() << '\n'
            );
        }

        // Check that only a single result was returned
        std::size_t n_external_individuals = ptr_in.get<std::size_t>(
            batch + ".n_individuals"
        ); // NOLINT(cppcoreguidelines-init-variables)
        if(1 != n_external_individuals) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << '\n'
                << "Number of result individuals != 1: " << n_external_individuals << '\n'
            );
        }

        // Check that the number of results provided by the result file matches the number of expected results
        std::size_t external_n_results = ptr_in.get<std::size_t>(
            "batch.individuals.individual0.n_results"
        ); // NOLINT(cppcoreguidelines-init-variables)
        if(external_n_results != n_results_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << '\n'
                << "Result file provides n_results = " << external_n_results << '\n'
                << "while we expected " << n_results_ << '\n'
            );
        }

        // Check whether the results represent useful values
        bool is_valid = ptr_in.get<bool>(
            "batch.individuals.individual0.isValid"
        );                                    // NOLINT(cppcoreguidelines-init-variables)
        if(not is_valid) {                    // Assign worst-case values to all result
            std::ostringstream error_message; // NOLINT(cppcoreguidelines-init-variables)

            error_message << "In GExternalEvaluatorIndividual::fitnessCalculation():" << '\n'
                          << "batch.individuals.individual0.isValid is \"false\"" << '\n';

#ifdef DEBUG
            glogger << error_message.str() << GWARNING;
#endif

            main_result = this->getWorstCase();
            for(std::size_t res = 1; res < n_results_; res++) {
                this->setResult(res, this->getWorstCase());
            }

            // Make sure the individual can be recognized as invalid by Geneva
            this->force_set_error(error_message.str());
        }
        else { // Extract and store all result values
            // Get the results node
            pt::ptree results_node = ptr_in.get_child("batch.individuals.individual0.results");

            double current_result = 0.;
            std::string result_string;
            for(std::size_t res = 0; res < n_results_; res++) {
                result_string = std::string("rawResult") + Gem::Common::to_string(res);

                current_result = results_node.get<double>(result_string);

                if(res == 0) {
                    main_result = current_result;
                }

                this->setResult(res, current_result);
            }
        }
    }

    // Return the master result (first result returned)
    return main_result;
}

/******************************************************************************/
/**
 * Allows to assign a run-id to this individual
 */
void GExternalEvaluatorIndividual::setRunId(std::string run_id) {
    if(run_id.empty() || "empty" == run_id) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividual::setRunId(): Error!" << '\n'
            << "Attempt to set an invalid run id: \"" << run_id << "\"" << '\n'
        );
    }

    runID_ = run_id;
}

/******************************************************************************/
/**
 * Allows to retrieve the run-id assigned to this individual
 */
std::string GExternalEvaluatorIndividual::getRunId() const {
    return runID_;
}

/******************************************************************************/
/**
 * Allows to specify whether temporary files should be removed. This is mostly
 * needed for debugging purposes.
 */
void GExternalEvaluatorIndividual::setRemoveExecTemporaries(bool remove_exec_temporaries) {
    remove_exec_temporaries_ = remove_exec_temporaries;
}

/******************************************************************************/
/**
 * Allows to check whether temporaries should be removed
 */
bool GExternalEvaluatorIndividual::getRemoveExecTemporaries() const {
    return remove_exec_temporaries_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Creates GExternalEvaluatorIndividual objects, based on an XML template
 * provided by an external program. See the description of the
 * GExternalEvaluatorIndividual class for the options this program needs
 * to understand.
 *
 * @param config_file The name of the configuration file
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory(
    std::filesystem::path const &config_file
)
  : Gem::Common::GFactoryT<GParameterSet>(config_file)
  , adProb_(GEEI_DEF_ADPROB)
  , adaptAdProb_(GEEI_DEF_ADAPTADPROB)
  , minAdProb_(GEEI_DEF_MINADPROB)
  , maxAdProb_(GEEI_DEF_MAXADPROB)
  , adaptionThreshold_(GEEI_DEF_ADAPTIONTHRESHOLD)
  , useBiGaussian_(GEEI_DEF_USEBIGAUSSIAN)
  , sigma1_(GEEI_DEF_SIGMA1)
  , sigmaSigma1_(GEEI_DEF_SIGMASIGMA1)
  , minSigma1_(GEEI_DEF_MINSIGMA1)
  , maxSigma1_(GEEI_DEF_MAXSIGMA1)
  , sigma2_(GEEI_DEF_SIGMA2)
  , sigmaSigma2_(GEEI_DEF_SIGMASIGMA2)
  , minSigma2_(GEEI_DEF_MINSIGMA2)
  , maxSigma2_(GEEI_DEF_MAXSIGMA2)
  , delta_(GEEI_DEF_DELTA)
  , sigmaDelta_(GEEI_DEF_SIGMADELTA)
  , minDelta_(GEEI_DEF_MINDELTA)
  , maxDelta_(GEEI_DEF_MAXDELTA)
  , programName_(GEEI_DEF_PROGNAME)
  , customOptions_(GEEI_DEF_CUSTOMOPTIONS)
  , parameterFileBaseName_(GEEI_DEF_PARFILEBASENAME)
  , initValues_(GEEI_DEF_STARTMODE)
  , removeExecTemporaries_(GEEI_DEF_REMOVETEMPORARIES)
  , externalEvaluatorQueried_(false) { /* nothing */
}

/******************************************************************************/
/**
 * The copy constructor
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory(
    const GExternalEvaluatorIndividualFactory &cp
)
  : Gem::Common::GFactoryT<GParameterSet>(cp)
  , adProb_(cp.adProb_)
  , adaptAdProb_(cp.adaptAdProb_)
  , minAdProb_(cp.minAdProb_)
  , maxAdProb_(cp.maxAdProb_)
  , adaptionThreshold_(cp.adaptionThreshold_)
  , useBiGaussian_(cp.useBiGaussian_)
  , sigma1_(cp.sigma1_)
  , sigmaSigma1_(cp.sigmaSigma1_)
  , minSigma1_(cp.minSigma1_)
  , maxSigma1_(cp.maxSigma1_)
  , sigma2_(cp.sigma2_)
  , sigmaSigma2_(cp.sigmaSigma2_)
  , minSigma2_(cp.minSigma2_)
  , maxSigma2_(cp.maxSigma2_)
  , delta_(cp.delta_)
  , sigmaDelta_(cp.sigmaDelta_)
  , minDelta_(cp.minDelta_)
  , maxDelta_(cp.maxDelta_)
  , programName_(cp.programName_)
  , customOptions_(cp.customOptions_)
  , parameterFileBaseName_(cp.parameterFileBaseName_)
  , initValues_(cp.initValues_)
  , removeExecTemporaries_(cp.removeExecTemporaries_)
  , externalEvaluatorQueried_(cp.externalEvaluatorQueried_)
  , ptr_(cp.ptr_) { /* nothing */
}

/******************************************************************************/
/**
 * The default constructor. Only needed for (de-)serialization purposes, hence empty.
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory()
  : Gem::Common::GFactoryT<GParameterSet>("empty")
  , adProb_(GEEI_DEF_ADPROB)
  , adaptAdProb_(GEEI_DEF_ADAPTADPROB)
  , minAdProb_(GEEI_DEF_MINADPROB)
  , maxAdProb_(GEEI_DEF_MAXADPROB)
  , adaptionThreshold_(GEEI_DEF_ADAPTIONTHRESHOLD)
  , useBiGaussian_(GEEI_DEF_USEBIGAUSSIAN)
  , sigma1_(GEEI_DEF_SIGMA1)
  , sigmaSigma1_(GEEI_DEF_SIGMASIGMA1)
  , minSigma1_(GEEI_DEF_MINSIGMA1)
  , maxSigma1_(GEEI_DEF_MAXSIGMA1)
  , sigma2_(GEEI_DEF_SIGMA2)
  , sigmaSigma2_(GEEI_DEF_SIGMASIGMA2)
  , minSigma2_(GEEI_DEF_MINSIGMA2)
  , maxSigma2_(GEEI_DEF_MAXSIGMA2)
  , delta_(GEEI_DEF_DELTA)
  , sigmaDelta_(GEEI_DEF_SIGMADELTA)
  , minDelta_(GEEI_DEF_MINDELTA)
  , maxDelta_(GEEI_DEF_MAXDELTA)
  , programName_(GEEI_DEF_PROGNAME)
  , customOptions_(GEEI_DEF_CUSTOMOPTIONS)
  , parameterFileBaseName_(GEEI_DEF_PARFILEBASENAME)
  , initValues_(GEEI_DEF_STARTMODE)
  , removeExecTemporaries_(GEEI_DEF_REMOVETEMPORARIES)
  , externalEvaluatorQueried_(false) { /* nothing */
}

/******************************************************************************/
/**
 * The destructor. Note that, if the external evaluator, when called with the
 * --finalize switch, does anything making optimization impossible, the factory
 * should not be destroyed before the end of the optimization run.
 */
GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory() {
    // Check that the file name isn't empty
    if(programName_.value().empty()) {
        glogger
            << "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): "
               "Error!"
            << '\n'
            << "Program name was empty" << '\n'
            << GTERMINATION;
    }

    // Check that the file exists
    if(not std::filesystem::exists(programName_.value())) {
        glogger
            << "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): "
               "Error!"
            << '\n'
            << "External program " << programName_.value() << " does not seem to exist"
            << '\n'
            << GTERMINATION;
    }

    // Collect all command-line arguments
    std::vector<std::string> arguments;
    if(customOptions_.value() != "empty" && not customOptions_.value().empty()) {
        arguments.push_back(customOptions_.value());
    }
    arguments.emplace_back("--finalize");

    // Ask the external evaluation program to perform any final work
    std::string command;
    int error_code = Gem::Common::runExternalCommand(
        std::filesystem::path(programName_.value()),
        arguments,
        std::filesystem::path(),
        command
    );

    // Let the audience know
    if(error_code) {
        glogger << "In GExternalEvaluatorIndividual::~GExternalEvaluatorIndividualFactory(): Error"
                << '\n'
                << "Execution of external command failed." << '\n'
                << "Command: " << command << '\n'
                << "Error code: " << error_code << '\n'
                << GTERMINATION;
    }
}

/******************************************************************************/
/**
 * Loads the data of another GFunctionIndividualFactory object
 */
void GExternalEvaluatorIndividualFactory::load(
    std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>> cp_raw_ptr
) {
    // Load our parent class'es data
    Gem::Common::GFactoryT<GParameterSet>::load(cp_raw_ptr);

    // Convert the base pointer
    std::shared_ptr<GExternalEvaluatorIndividualFactory> cp_ptr = Gem::Common::convertSmartPointer<
        Gem::Common::GFactoryT<GParameterSet>,
        GExternalEvaluatorIndividualFactory>(cp_raw_ptr);

    // And then our own
    adProb_ = cp_ptr->adProb_;
    adaptAdProb_ = cp_ptr->adaptAdProb_;
    minAdProb_ = cp_ptr->minAdProb_;
    maxAdProb_ = cp_ptr->maxAdProb_;
    adaptionThreshold_ = cp_ptr->adaptionThreshold_;
    useBiGaussian_ = cp_ptr->useBiGaussian_;
    sigma1_ = cp_ptr->sigma1_;
    sigmaSigma1_ = cp_ptr->sigmaSigma1_;
    minSigma1_ = cp_ptr->minSigma1_;
    maxSigma1_ = cp_ptr->maxSigma1_;
    sigma2_ = cp_ptr->sigma2_;
    sigmaSigma2_ = cp_ptr->sigmaSigma2_;
    minSigma2_ = cp_ptr->minSigma2_;
    maxSigma2_ = cp_ptr->maxSigma2_;
    delta_ = cp_ptr->delta_;
    sigmaDelta_ = cp_ptr->sigmaDelta_;
    minDelta_ = cp_ptr->minDelta_;
    maxDelta_ = cp_ptr->maxDelta_;
    programName_ = cp_ptr->programName_;
    customOptions_ = cp_ptr->customOptions_;
    parameterFileBaseName_ = cp_ptr->parameterFileBaseName_;
    initValues_ = cp_ptr->initValues_;
    removeExecTemporaries_ = cp_ptr->removeExecTemporaries_;
    externalEvaluatorQueried_ = cp_ptr->externalEvaluatorQueried_;
    ptr_ = cp_ptr->ptr_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
std::shared_ptr<Gem::Common::GFactoryT<GParameterSet>>
GExternalEvaluatorIndividualFactory::clone() const {
    return std::make_shared<GExternalEvaluatorIndividualFactory>(*this);
}

/******************************************************************************/
/**
 * Get the value of the adaptionThreshold_ variable
 */
std::uint32_t GExternalEvaluatorIndividualFactory::getAdaptionThreshold() const {
    return adaptionThreshold_;
}

/******************************************************************************/
/**
 * Set the value of the adaptionThreshold_ variable
 */
void GExternalEvaluatorIndividualFactory::setAdaptionThreshold(std::uint32_t adaption_threshold) {
    adaptionThreshold_ = adaption_threshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the adProb_ variable
 */
double GExternalEvaluatorIndividualFactory::getAdProb() const {
    return adProb_;
}

/******************************************************************************/
/**
 * Set the value of the adProb_ variable
 */
void GExternalEvaluatorIndividualFactory::setAdProb(double ad_prob) {
    adProb_ = ad_prob;
}

/******************************************************************************/
/**
 * Allows to retrieve the rate of evolutionary adaption of adProb_
 */
double GExternalEvaluatorIndividualFactory::getAdaptAdProb() const {
    return adaptAdProb_;
}

/******************************************************************************/
/**
 * Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature)
 */
void GExternalEvaluatorIndividualFactory::setAdaptAdProb(double adapt_ad_prob) {
#ifdef DEBUG
    if(adapt_ad_prob < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setAdaptAdProb(): Error!" << '\n'
            << "Invalid value for adapt_ad_prob given: " << adapt_ad_prob << '\n'
        );
    }
#endif /* DEBUG */

    adaptAdProb_ = adapt_ad_prob;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for adProb_ variation
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getAdProbRange() const {
    return std::tuple<double, double>{minAdProb_.value(), maxAdProb_.value()};
}

/******************************************************************************/
/**
 * Allows to set the allowed range for adaption probability variation
 */
void GExternalEvaluatorIndividualFactory::setAdProbRange(double min_ad_prob, double max_ad_prob) {
#ifdef DEBUG
    if(min_ad_prob < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "min_ad_prob < 0: " << min_ad_prob << '\n'
        );
    }

    if(min_ad_prob > max_ad_prob) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "Invalid min_ad_prob and/or max_ad_prob: " << min_ad_prob << " / " << max_ad_prob << '\n'
        );
    }

    if(max_ad_prob > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "max_ad_prob > 1: " << max_ad_prob << '\n'
        );
    }
#endif /* DEBUG */

    minAdProb_ = min_ad_prob;
    maxAdProb_ = max_ad_prob;
}

/******************************************************************************/
/**
 * Allows to retrieve the delta_ variable
 */
double GExternalEvaluatorIndividualFactory::getDelta() const {
    return delta_;
}

/******************************************************************************/
/**
 * Set the value of the delta_ variable
 */
void GExternalEvaluatorIndividualFactory::setDelta(double delta) {
    delta_ = delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxDelta() const {
    return maxDelta_;
}

/******************************************************************************/
/**
 * Set the value of the maxDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxDelta(double max_delta) {
    maxDelta_ = max_delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxSigma1() const {
    return maxSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxSigma1(double max_sigma1) {
    maxSigma1_ = max_sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxSigma2() const {
    return maxSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxSigma2(double max_sigma2) {
    maxSigma2_ = max_sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the minDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getMinDelta() const {
    return minDelta_;
}

/******************************************************************************/
/**
 * Set the value of the minDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinDelta(double min_delta) {
    minDelta_ = min_delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of delta
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getDeltaRange() const {
    return std::tuple<double, double>{minDelta_, maxDelta_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of delta
 */
void GExternalEvaluatorIndividualFactory::setDeltaRange(std::tuple<double, double> range) {
    double min = std::get<0>(range);
    double max = std::get<1>(range);

    if(min < 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setDeltaRange(): Error" << '\n'
            << "min must be >= 0. Got : " << min << '\n'
        );
    }

    if(min >= max) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setDeltaRange(): Error" << '\n'
            << "Invalid range specified: " << min << " / " << max << '\n'
        );
    }

    minDelta_ = min;
    maxDelta_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minSigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getMinSigma1() const {
    return minSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the minSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinSigma1(double min_sigma1) {
    minSigma1_ = min_sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma1_
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getSigma1Range() const {
    return std::tuple<double, double>{minSigma1_, maxSigma1_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma1_
 */
void GExternalEvaluatorIndividualFactory::setSigma1Range(std::tuple<double, double> range) {
    double min = std::get<0>(range);
    double max = std::get<1>(range);

    if(min < 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setSigma1Range(): Error" << '\n'
            << "min must be >= 0. Got : " << min << '\n'
        );
    }

    if(min >= max) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setSigma1Range(): Error" << '\n'
            << "Invalid range specified: " << min << " / " << max << '\n'
        );
    }

    minSigma1_ = min;
    maxSigma1_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minSigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getMinSigma2() const {
    return minSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the minSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinSigma2(double min_sigma2) {
    minSigma2_ = min_sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma2_
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getSigma2Range() const {
    return std::tuple<double, double>{minSigma2_, maxSigma2_};
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma2_
 */
void GExternalEvaluatorIndividualFactory::setSigma2Range(std::tuple<double, double> range) {
    double min = std::get<0>(range);
    double max = std::get<1>(range);

    if(min < 0) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setSigma2Range(): Error" << '\n'
            << "min must be >= 0. Got : " << min << '\n'
        );
    }

    if(min >= max) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setSigma2Range(): Error" << '\n'
            << "Invalid range specified: " << min << " / " << max << '\n'
        );
    }

    minSigma2_ = min;
    maxSigma2_ = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigma1() const {
    return sigma1_;
}

/******************************************************************************/
/**
 * Set the value of the sigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigma1(double sigma1) {
    sigma1_ = sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigma2() const {
    return sigma2_;
}

/******************************************************************************/
/**
 * Set the value of the sigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigma2(double sigma2) {
    sigma2_ = sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigmaDelta() const {
    return sigmaDelta_;
}

/******************************************************************************/
/**
 * Set the value of the sigmaDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigmaDelta(double sigma_delta) {
    sigmaDelta_ = sigma_delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaSigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigmaSigma1() const {
    return sigmaSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the sigmaSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigmaSigma1(double sigma_sigma1) {
    sigmaSigma1_ = sigma_sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaSigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigmaSigma2() const {
    return sigmaSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the sigmaSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigmaSigma2(double sigma_sigma2) {
    sigmaSigma2_ = sigma_sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the useBiGaussian_ variable
 */
bool GExternalEvaluatorIndividualFactory::getUseBiGaussian() const {
    return useBiGaussian_;
}

/******************************************************************************/
/**
 * Set the value of the useBiGaussian_ variable
 */
void GExternalEvaluatorIndividualFactory::setUseBiGaussian(bool use_bi_gaussian) {
    useBiGaussian_ = use_bi_gaussian;
}

/******************************************************************************/
/**
 * Allows to set the name and path of the external program. Note that this will have a
 * lasting effect even if the external configuration file is parsed repeatedly,
 * as we use a "one-time-reference" parameter. Using the "setValue" option woll in
 * contrast reset the internal value of that object.
 */
void GExternalEvaluatorIndividualFactory::setProgramName(std::string program_name) {
    // Check that the file name isn't empty
    if(program_name.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << '\n'
            << "File name was empty" << '\n'
        );
    }

    // Check that the file exists
    if(not std::filesystem::exists(program_name)) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << '\n'
            << "External program " << program_name << " does not seem to exist" << '\n'
        );
    }

    programName_.setValue(program_name);
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the external program
 */
std::string GExternalEvaluatorIndividualFactory::getProgramName() const {
    return programName_;
}

/******************************************************************************/
/**
 * Sets the name of the external evaluation program. Note that this will have a
 * lasting effect even if the external configuration file is parsed repeatedly,
 * as we use a "one-time-reference" parameter. Using the "setValue" option woll in
 * contrast reset the internal value of that object.
 */
void GExternalEvaluatorIndividualFactory::setCustomOptions(std::string custom_options) {
    customOptions_.setValue(custom_options);
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividualFactory::getCustomOptions() const {
    return customOptions_;
}

/******************************************************************************/
/**
 * Allows to set the base name of the parameter file
 */
void GExternalEvaluatorIndividualFactory::setParameterFileBaseName(
    std::string parameter_file_base_name
) {
    // Check that the name isn't empty
    if(parameter_file_base_name.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setParameterFileBaseName(): Error!"
            << '\n'
            << "Name was empty" << '\n'
        );
    }

    parameterFileBaseName_ = parameter_file_base_name;
}

/******************************************************************************/
/**
 * Allows to retrieve the base name of the parameter file
 */
std::string GExternalEvaluatorIndividualFactory::getParameterFileBaseName() const {
    return parameterFileBaseName_;
}

/******************************************************************************/
/**
 * Indicates the initialization mode
 *
 * TODO: Allow "none" in case parameters should be solely supplied by the external evaluator
 */
void GExternalEvaluatorIndividualFactory::setInitValues(std::string init_values) {
    if(init_values != "random" && init_values != "min" && init_values != "max") {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setInitValues(): Error!" << '\n'
            << "Invalid argument: " << init_values << '\n'
            << "Expected \"min\", \"max\", or \"random\"." << '\n'
        );
    }

    initValues_.setValue(init_values);
}

/******************************************************************************/
/**
 * Allows to retrieve the initialization mode
 */
std::string GExternalEvaluatorIndividualFactory::getInitValues() const {
    return initValues_;
}

/******************************************************************************/
/**
 * Allows to specify whether temporary files should be removed
 */
void GExternalEvaluatorIndividualFactory::setRemoveExecTemporaries(bool remove_exec_temporaries) {
    removeExecTemporaries_.setValue(remove_exec_temporaries);
}

/******************************************************************************/
/**
 * Allows to check whether temporaries should be removed
 */
bool GExternalEvaluatorIndividualFactory::getRemoveExecTemporaries() const {
    return removeExecTemporaries_;
}

/******************************************************************************/
/**
 * Submit work items to the external executable for archiving
 */
void GExternalEvaluatorIndividualFactory::archive(
    const std::vector<std::shared_ptr<GExternalEvaluatorIndividual>> &arch
) const {
    // Check that there are individuals contained in the archive
    if(arch.empty()) {
        return; // Do nothing
    }

    // Transform the objects into a batch of boost property tree
    boost::property_tree::ptree ptr_out; // NOLINT(cppcoreguidelines-init-variables)
    std::string batch = "batch";

    // Output the header data
    ptr_out.put(batch + ".dataType", std::string("archive_data"));
    ptr_out.put(batch + ".run_id", arch.front()->getRunId());
    ptr_out.put(batch + ".n_individuals", arch.size());

    // Output the individuals in turn
    std::vector<std::shared_ptr<GExternalEvaluatorIndividual>>::const_iterator cit;
    std::size_t pos = 0;
    std::string basename;
    for(cit = arch.begin(); cit != arch.

                                   end();

        ++cit) {
        basename = batch + ".individuals.individual" + Gem::Common::to_string(pos++);
        (*cit)->toPropertyTree(ptr_out, basename);
    }

    // Create a suitable extension and exchange file names for this object
    std::chrono::time_point<std::chrono::high_resolution_clock> p1;
    std::chrono::time_point<std::chrono::high_resolution_clock> p2 =
        std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds ms_since_1970 =
        std::chrono::duration_cast<std::chrono::milliseconds>(p2 - p1);
    std::string extension = "-since1970-" + Gem::Common::to_string(ms_since_1970.count()) +
                            Gem::Common::generate_uuid_v4() + ".xml";
    std::string parameterfile_name = parameterFileBaseName_.value() + extension;

    // Save the parameters to a file for the external evaluation
    boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
    boost::property_tree::write_xml(parameterfile_name, ptr_out, std::locale(), settings);

    // Collect all command-line arguments
    std::vector<std::string> arguments;
    if(customOptions_.value() != "empty" && not customOptions_.value().empty()) {
        arguments.push_back(customOptions_.value());
    }
    arguments.emplace_back("--archive");
    arguments.push_back(std::string("--input=\"" + parameterfile_name + "\""));

    // Ask the external evaluation program to perform any final work
    std::string command;
    int error_code = Gem::Common::runExternalCommand(
        std::filesystem::path(programName_.value()),
        arguments,
        std::filesystem::path(),
        command
    );

    // Let the audience know
    if(error_code) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::archive(): Error" << '\n'
            << "Execution of external command failed." << '\n'
            << "Command: " << command << '\n'
            << "Error code: " << error_code << '\n'
        );
    }

    // Clean up (remove) the parameter file. This will only be done if no error occurred
    std::filesystem::remove(parameterfile_name);
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<GParameterSet> GExternalEvaluatorIndividualFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    const std::size_t & /*id*/
) {
    // Will hold the result
    std::shared_ptr<GExternalEvaluatorIndividual> target(new GExternalEvaluatorIndividual());

    // Make the object's local configuration options known
    target->addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GExternalEvaluatorIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder &gpb) {
    // Describe our own options
    using namespace Gem::Courtier;

    // Allow our parent class to describe its options
    Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);

    // Then add our local options
    gpb.registerFileParameter<double>("ad_prob", adProb_.reference(), GEEI_DEF_ADPROB)
        << "The probability for random adaption of values in evolutionary algorithms";

    gpb.registerFileParameter<double>(
        "adapt_ad_prob",
        adaptAdProb_.reference(),
        GEEI_DEF_ADAPTADPROB
    ) << "Determines the rate of adaption of ad_prob. Set to 0, if you do not need this feature";

    gpb.registerFileParameter<double>("min_ad_prob", minAdProb_.reference(), GEEI_DEF_MINADPROB)
        << "The lower allowed boundary for ad_prob-variation";

    gpb.registerFileParameter<double>("max_ad_prob", maxAdProb_.reference(), GEEI_DEF_MAXADPROB)
        << "The upper allowed boundary for ad_prob-variation";

    gpb.registerFileParameter<std::uint32_t>(
        "adaption_threshold",
        adaptionThreshold_.reference(),
        GEEI_DEF_ADAPTIONTHRESHOLD
    ) << "The number of calls to an adaptor after which adaption takes place";

    gpb.registerFileParameter<bool>(
        "use_bi_gaussian",
        useBiGaussian_.reference(),
        GEEI_DEF_USEBIGAUSSIAN
    ) << "Whether to use a double gaussion for the adaption of parmeters in ES";

    gpb.registerFileParameter<double>("sigma1", sigma1_.reference(), GEEI_DEF_SIGMA1)
        << "The sigma for gauss-adaption in ES" << '\n'
        << "(or the sigma of the left peak of a double gaussian)";

    gpb.registerFileParameter<double>(
        "sigma_sigma1",
        sigmaSigma1_.reference(),
        GEEI_DEF_SIGMASIGMA1
    ) << "Influences the self-adaption of gauss-mutation in ES";

    gpb.registerFileParameter<double>("min_sigma1", minSigma1_.reference(), GEEI_DEF_MINSIGMA1)
        << "The minimum value of sigma1";

    gpb.registerFileParameter<double>("max_sigma1", maxSigma1_.reference(), GEEI_DEF_MAXSIGMA1)
        << "The maximum value of sigma1";

    gpb.registerFileParameter<double>("sigma2", sigma2_.reference(), GEEI_DEF_SIGMA2)
        << "The sigma of the right peak of a double gaussian (if any)";

    gpb.registerFileParameter<double>(
        "sigma_sigma2",
        sigmaSigma2_.reference(),
        GEEI_DEF_SIGMASIGMA2
    ) << "Influences the self-adaption of gauss-mutation in ES";

    gpb.registerFileParameter<double>("min_sigma2", minSigma2_.reference(), GEEI_DEF_MINSIGMA2)
        << "The minimum value of sigma2";

    gpb.registerFileParameter<double>("max_sigma2", maxSigma2_.reference(), GEEI_DEF_MAXSIGMA2)
        << "The maximum value of sigma2";

    gpb.registerFileParameter<double>("delta", delta_.reference(), GEEI_DEF_DELTA)
        << "The start distance between both peaks used for bi-gaussian mutations in ES";

    gpb.registerFileParameter<double>("sigma_delta", sigmaDelta_.reference(), GEEI_DEF_SIGMADELTA)
        << "The width of the gaussian used for mutations of the delta parameter";

    gpb.registerFileParameter<double>("min_delta", minDelta_.reference(), GEEI_DEF_MINDELTA)
        << "The minimum allowed value of delta";

    gpb.registerFileParameter<double>("max_delta", maxDelta_.reference(), GEEI_DEF_MAXDELTA)
        << "The maximum allowed value of delta";

    gpb.registerFileParameter<std::string>(
        "program_name",
        programName_.reference() // Upon repeated filling this option will do nothing
        ,
        GEEI_DEF_PROGNAME
    ) << "The name of the external evaluation program";

    gpb.registerFileParameter<std::string>(
        "custom_options",
        customOptions_.reference(),
        GEEI_DEF_CUSTOMOPTIONS
    ) << "Any custom options you wish to pass to the external evaluator";

    gpb.registerFileParameter<std::string>(
        "parameter_file",
        parameterFileBaseName_.reference(),
        GEEI_DEF_PARFILEBASENAME
    ) << "The base name assigned to parameter files"
      << '\n'
      << "in addition to data identifying this specific evaluation";

    gpb.registerFileParameter<std::string>(
        "init_values",
        initValues_.reference(),
        GEEI_DEF_STARTMODE
    ) << "Indicates, whether individuals should be initialized randomly (random),"
      << '\n'
      << "with the lower (min) or upper (max) boundary of their value ranges";

    gpb.registerFileParameter<bool>(
        "remove_exec_temporaries",
        removeExecTemporaries_.reference(),
        GEEI_DEF_REMOVETEMPORARIES
    ) << "Indicates, whether files created during external execution should be removed";
}

/******************************************************************************/
/**
 * This function asks the external evaluation program to perform any necessary
 * setup work and then queries it for setup-information, storing the data in
 * a boost::property_tree object. Note that this function will do nothing when
 * called more than once.
 */
void GExternalEvaluatorIndividualFactory::setUpPropertyTree() {
    if(externalEvaluatorQueried_) {
        return;
    }
    else {
        externalEvaluatorQueried_ = true;
    }

    // Check that the file name isn't empty
    if(programName_.value().empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << '\n'
            << "File name was empty" << '\n'
        );
    }

    // Check that the file exists
    if(not std::filesystem::exists(programName_.value())) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << '\n'
            << "External program " << programName_.value() << " does not seem to exist"
            << '\n'
        );
    }

    // Make sure the property tree is empty
    ptr_.clear();

    { // First we give the external program the opportunity to perform an initial work
        // Collect all command-line arguments
        std::vector<std::string> arguments;
        if(customOptions_.value() != "empty" && not customOptions_.value().empty()) {
            arguments.push_back(customOptions_.value());
        }
        arguments.emplace_back("--init");

        // Ask the external evaluation program to perform any initial work
        std::string command;
        int error_code = Gem::Common::runExternalCommand(
            std::filesystem::path(programName_.value()),
            arguments,
            std::filesystem::path(),
            command
        );

        if(error_code) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividual::setUpPropertyTree(//1//): Error" << '\n'
                << "Execution of external command failed." << '\n'
                << "Command: " << command << '\n'
                << "Error code: " << error_code << '\n'
            );
        }
    }

    { // Now we ask the external program for setup-iformation
        // Collect all command-line arguments
        std::vector<std::string> arguments;
        if(customOptions_.value() != "empty" && not customOptions_.value().empty()) {
            arguments.push_back(customOptions_.value());
        }

        // "/" will be converted to "\" in runExternalCommand, if necessary
        std::string setup_file_name =
            std::string("./setup-") + Gem::Common::to_string(this) + std::string(".xml");
        arguments.push_back("--setup");
        arguments.push_back("--output=\"" + setup_file_name + "\"");
        arguments.push_back("--initvalues=\"" + initValues_.value() + "\"");

        // Ask the external evaluation program tfor setup information
        std::string command;
        int error_code = Gem::Common::runExternalCommand(
            std::filesystem::path(programName_.value()),
            arguments,
            std::filesystem::path(),
            command
        );

        if(error_code) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividual::setUpPropertyTree(//2//): Error" << '\n'
                << "Execution of external command failed." << '\n'
                << "Command: " << command << '\n'
                << "Error code: " << error_code << '\n'
            );
        }

        // Parse the setup file
        pt::read_xml(setup_file_name, ptr_);

        // Clean up
        std::filesystem::remove(std::filesystem::path(setup_file_name));
    }
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GExternalEvaluatorIndividualFactory::postProcess_(std::shared_ptr<GParameterSet> &p_raw) {
    using boost::property_tree::ptree;

    // Convert the base pointer to the target type
    std::shared_ptr<GExternalEvaluatorIndividual> p =
        Gem::Common::convertSmartPointer<GParameterSet, GExternalEvaluatorIndividual>(p_raw);

    // Set up a random number generator
    Gem::Hap::GRandom gr;

    // Here we ask the external evaluator to perform any necessary setup work
    // and query it for the desired structure of our individuals. The work is done
    // but once, and the results are stored in a private object inside of this class.
    this->setUpPropertyTree();

    if(ptr_.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << '\n'
            << "Property tree is empty." << '\n'
        );
    }

    // Set up an adaptor for the collection, so they know how to be adapted
    std::shared_ptr<GAdaptorT<double>> gat_ptr;
    if(useBiGaussian_) {
        std::shared_ptr<GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());
        gdbga_ptr->setAllSigma1(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_);
        gdbga_ptr->setAllSigma2(sigma2_, sigmaSigma2_, minSigma2_, maxSigma2_);
        gdbga_ptr->setAllDelta(delta_, sigmaDelta_, minDelta_, maxDelta_);
        gdbga_ptr->setAdaptionThreshold(adaptionThreshold_);
        gdbga_ptr->setAdaptionProbability(adProb_);
        gat_ptr = gdbga_ptr;
    }
    else {
        std::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(
            new GDoubleGaussAdaptor(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_)
        );
        gdga_ptr->setAdaptionThreshold(adaptionThreshold_);
        gdga_ptr->setAdaptionProbability(adProb_);
        gat_ptr = gdga_ptr;
    }

    // Store parameters pertaining to the adaption probability in the adaptor
    gat_ptr->setAdaptAdProb(adaptAdProb_);
    gat_ptr->setAdProbRange(minAdProb_, maxAdProb_);

    try {
        // Extract the number of individuals
        std::size_t n_individuals =
            ptr_.get<std::size_t>("batch.n_individuals"); // NOLINT(cppcoreguidelines-init-variables)
        if(1 != n_individuals) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << '\n'
                << "Received invalid number of setup-individuals: " << n_individuals << '\n'
            );
        }

        // Get the run-id
        std::string run_id = ptr_.get<std::string>("batch.run_id");

        // Extract the number of variables for the first individual
        std::size_t n_var = ptr_.get<std::size_t>(
            "batch.individuals.individual0.nVars"
        ); // NOLINT(cppcoreguidelines-init-variables)

        // Extract the number of results to be expected from the external evaluation function for the first individual
        std::size_t n_results_expected = ptr_.get<std::size_t>(
            "batch.individuals.individual0.n_results"
        ); // NOLINT(cppcoreguidelines-init-variables)

        // Extract the number of boundaries to be expected for the first individual
        std::size_t n_bounds = ptr_.get<std::size_t>(
            "batch.individuals.individual0.n_bounds"
        ); // NOLINT(cppcoreguidelines-init-variables)

        // Get an iterator over a property tree
        ptree::const_iterator cit;

        // If variables have been specified, extract them
        boost::optional<ptree &> var_set_node_opt =
            ptr_.get_child_optional("batch.individuals.individual0.vars");
        if(var_set_node_opt) {
            // Loop over all children of the variables tree
            // Note that for now we only query GConstrainedDoubleObject objects
            std::size_t var_counter = 0;
            std::string var_string = "var0";
            for(cit = (*var_set_node_opt).begin(); cit != (*var_set_node_opt).end(); ++cit) {
                if(var_string == cit->first) { // O.k., we found a varX string
                    // Just treat GConstrainedDoubleObject objects for now
                    if("GConstrainedDoubleObject" == (cit->second).get<std::string>("type")) {
                        // Extract the boundaries and initial values
                        std::string p_name = (cit->second).get<std::string>("name");
                        double min_var = (cit->second).get<double>("lowerBoundary");
                        double max_var = (cit->second).get<double>("upperBoundary");
                        double init_value = (cit->second).get<double>("values.value0");

                        // Create an initial (empty) pointer to a GConstrainedDoubleObject
                        std::shared_ptr<GConstrainedDoubleObject> gcdo_ptr;

                        // Act on the information, depending on whether random initialization has been requested
                        if(
                            min_var == max_var
                        ) { // We take this as a sign that the parameter should not be modified
                            // Create the parameter object
                            gcdo_ptr = std::make_shared<GConstrainedDoubleObject>(
                                init_value,
                                init_value,
                                std::max(1.0001 * init_value, init_value + 0.0001)
                            );
                            // Disable mutations
                            gcdo_ptr->setAdaptionsInactive();
                        }
                        else if(0 == (cit->second).count("initRandom") ||
                                false == (cit->second).get<bool>("initRandom")) {
                            // Create the parameter object
                            gcdo_ptr = std::make_shared<GConstrainedDoubleObject>(
                                init_value,
                                min_var,
                                max_var
                            );
                        }
                        else { // Random initialization has been requested
                            // Create the parameter object
                            gcdo_ptr = std::make_shared<GConstrainedDoubleObject>(min_var, max_var);
                        }
                        gcdo_ptr->setParameterName(p_name);

                        // Add the adaptor to the parameter object
                        gcdo_ptr->addAdaptor(gat_ptr);

                        // Add the object to the individual
                        p->push_back(gcdo_ptr);
                    }
                    else {
                        throw geneva_exception(
                            g_error_streamer(DO_LOG, time_and_place)
                            << "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!"
                            << '\n'
                            << (cit->second).get<std::string>("type") << " provided as type name."
                            << '\n'
                            << "Currently only GConstrainedDoubleObject is supported." << '\n'
                        );
                    }

                    if(++var_counter >= n_var) {
                        break; // Terminate the loop if we have identified all expected parameter objects
                    }
                    else {
                        var_string = std::string("var") +
                                     Gem::Common::to_string(var_counter); // Create a new var string
                    }
                }
            }
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << '\n'
                << "No variables were specified" << '\n'
            );
        }

        // If boundaries have been specified, add the required boundary objects to the individual.
        boost::optional<ptree &> bounds_node_opt =
            ptr_.get_child_optional("batch.individuals.individual0.bounds");
        if(bounds_node_opt) {
            // Create a check combiner -- it will hold the boundary conditions we find here
            std::shared_ptr<GCheckCombinerT<GParameterSet>> combiner_ptr(
                new GCheckCombinerT<GParameterSet>()
            );

            // Loop over all children of the bounds tree
            // Note that for now we only query GConstrainedDoubleObject objects
            std::size_t bounds_counter = 0;
            std::string bound_string = "bound0";
            for(cit = (*bounds_node_opt).begin(); cit != (*bounds_node_opt).end(); ++cit) {
                if(cit->first == bound_string) {
                    std::string expression = (cit->second).get<std::string>("expression");
                    bool allow_negative = (cit->second).get<bool>("allow_negative");

                    // The actual "function-constraint"
                    std::shared_ptr<GParameterSetFormulaConstraint> formula_constraint(
                        new GParameterSetFormulaConstraint(expression)
                    );

                    formula_constraint->setAllowNegative(allow_negative);

                    // Add the constraint to the check-combiner
                    combiner_ptr->addCheck(formula_constraint);

                    if(++bounds_counter >= n_bounds) {
                        break; // Terminate the loop if we have identified all expected boundaries
                    }
                    else {
                        bound_string =
                            std::string("bound") + Gem::Common::to_string(bounds_counter);
                    }
                }
            }

#ifdef DEBUG
            glogger << "Found " << bounds_counter << " bounds" << '\n' << GLOGGING;
#endif /* DEBUG */

            // Add the check combiner to the individual
            p->registerConstraint(combiner_ptr);
        } // It isn't an error if no boundaries were specified

        // Add the program name and base name for parameter transfers to the object
        p->setExchangeBaseName(parameterFileBaseName_);
        p->setProgramName(programName_);
        p->setCustomOptions(customOptions_);
        p->setNExpectedResults(n_results_expected);
        p->setRemoveExecTemporaries(removeExecTemporaries_);
        p->setRunId(run_id);
    }
    catch(const pt::ptree_bad_path &e) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << '\n'
            << "Caught ptree_bad_path exception with message " << '\n'
            << e.what() << '\n'
        );
    }
    catch(const geneva_exception &gec) {
        throw gec; // Re-throw
    }
    catch(...) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GExternalEvaluatorIndividualFactory::postProcess_(): Caught unknown exception!"
            << '\n'
        );
    }
}

/******************************************************************************/

} /* namespace Gem::Geneva */
