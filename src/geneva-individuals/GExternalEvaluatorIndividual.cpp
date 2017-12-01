/**
 * @file GExternalEvaluatorIndividual.cpp
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

#include "geneva-individuals/GExternalEvaluatorIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GExternalEvaluatorIndividual)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GExternalEvaluatorIndividualFactory)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual()
	: GParameterSet()
	  , m_program_name(GEEI_DEF_PROGNAME)
	  , m_custom_options(GEEI_DEF_CUSTOMOPTIONS)
	  , m_parameter_file_base_name(GEEI_DEF_PARFILEBASENAME)
	  , m_n_results(GEEI_DEF_NRESULTS)
	  , runID_(GEEI_DEF_RUNID)
	  , m_remove_exec_temporaries(GEEI_DEF_REMOVETEMPORARIES)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual &cp)
	: GParameterSet(cp) // copies all local collections
	  , m_program_name(cp.m_program_name)
	  , m_custom_options(cp.m_custom_options)
	  , m_parameter_file_base_name(cp.m_parameter_file_base_name)
	  , m_n_results(cp.m_n_results)
	  , runID_(cp.runID_)
	  , m_remove_exec_temporaries(cp.m_remove_exec_temporaries)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GExternalEvaluatorIndividual::~GExternalEvaluatorIndividual() { /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GExternalEvaluatorIndividual &GExternalEvaluatorIndividual::operator=(const GExternalEvaluatorIndividual &cp) {
	GExternalEvaluatorIndividual::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GExternalEvaluatorIndividual object
 *
 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
 * @return A boolean indicating whether both objects are equal
 */
bool GExternalEvaluatorIndividual::operator==(const GExternalEvaluatorIndividual &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
}

/******************************************************************************/
/**
 * Checks for inequality with another GExternalEvaluatorIndividual object
 *
 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
 * @return A boolean indicating whether both objects are inequal
 */
bool GExternalEvaluatorIndividual::operator!=(const GExternalEvaluatorIndividual &cp) const {
	using namespace Gem::Common;
	try {
		this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
		return true;
	} catch (g_expectation_violation &) {
		return false;
	}
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
void GExternalEvaluatorIndividual::compare(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	// Check that we are dealing with a GExternalEvaluatorIndividual reference independent of this object and convert the pointer
	const GExternalEvaluatorIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GExternalEvaluatorIndividual>(cp, this);

	Gem::Common::GToken token("GExternalEvaluatorIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(m_program_name, p_load->m_program_name), token);
	Gem::Common::compare_t(IDENTITY(m_custom_options, p_load->m_custom_options), token);
	Gem::Common::compare_t(IDENTITY(m_parameter_file_base_name, p_load->m_parameter_file_base_name), token);
	Gem::Common::compare_t(IDENTITY(m_n_results, p_load->m_n_results), token);
	Gem::Common::compare_t(IDENTITY(runID_, p_load->runID_), token);
	Gem::Common::compare_t(IDENTITY(m_remove_exec_temporaries, p_load->m_remove_exec_temporaries), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividual::setProgramName(const std::string &programName) {
	m_program_name = programName;
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividual::getProgramName() const {
	return m_program_name;
}


/******************************************************************************/
/**
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividual::setCustomOptions(const std::string &customOptions) {
	m_custom_options = customOptions;
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividual::getCustomOptions() const {
	return m_custom_options;
}

/******************************************************************************/
/**
 * Sets the base name of the data exchange file. Note that the individual might add additional
 * characters in order to distinguish between the exchange files of different individuals.
 *
 * @param parameterFile The desired new base name of the exchange file
 */
void GExternalEvaluatorIndividual::setExchangeBaseName(const std::string &parameterFile) {
	if (parameterFile.empty() || parameterFile == "empty") {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividual::setExchangeBaseName(): Error!" << std::endl
				<< "Invalid file name \"" << parameterFile << "\"" << std::endl
		);
	}

	m_parameter_file_base_name = parameterFile;
}

/******************************************************************************/
/**
 * Retrieves the current value of the parameterFileBaseName_ variable.
 *
 * @return The current base name of the exchange file
 */
std::string GExternalEvaluatorIndividual::getExchangeBaseName() const {
	return m_parameter_file_base_name;
}

/******************************************************************************/
/**
 * Sets the number of results to be expected from the external evaluation program
 */
void GExternalEvaluatorIndividual::setNExpectedResults(const std::size_t &nResults) {
	if (0 == nResults) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividual::setNExpectedResults(): Error!" << std::endl
				<< "Got invalid number of expected results: " << nResults << std::endl
		);
	}

	m_n_results = nResults;
}

/******************************************************************************/
/**
 * Retrieves the number of results to be expected from the external evaluation program
 */
std::size_t GExternalEvaluatorIndividual::getNExpectedResults() const {
	return m_n_results;
}

/******************************************************************************/
/**
 * Loads the data of another GExternalEvaluatorIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GExternalEvaluatorIndividual, camouflaged as a GObject
 */
void GExternalEvaluatorIndividual::load_(const GObject *cp) {
	// Check that we are dealing with a GExternalEvaluatorIndividual reference independent of this object and convert the pointer
	const GExternalEvaluatorIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GExternalEvaluatorIndividual>(cp, this);

	// First load the data of our parent class ...
	GParameterSet::load_(cp);

	// ... and then our own
	m_program_name = p_load->m_program_name;
	m_custom_options = p_load->m_custom_options;
	m_parameter_file_base_name = p_load->m_parameter_file_base_name;
	m_n_results = p_load->m_n_results;
	runID_ = p_load->runID_;
	m_remove_exec_temporaries = p_load->m_remove_exec_temporaries;
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
	boost::property_tree::ptree ptr_out;

	std::string batch = "batch";

	// Output the header data
	ptr_out.put(batch + ".dataType", std::string("run_parameters"));
	ptr_out.put(batch + ".runID", this->getRunId());
	ptr_out.put(batch + ".nIndividuals", std::size_t(1));

	std::string basename = batch + ".individuals.individual0";
	this->toPropertyTree(ptr_out, basename);

	// Create a suitable extension and exchange file names for this object
	std::string extension = std::string("-") + boost::lexical_cast<std::string>(this->getAssignedIteration()) + "-" +
									boost::lexical_cast<std::string>(this);
	std::string parameterfileName = m_parameter_file_base_name + extension + ".xml";
	std::string resultFileName = std::string("result") + extension + ".xml";
	std::string commandOutputFileName = std::string("commandOutput") + extension + ".txt";

	// Save the parameters to a file for the external evaluation
#if BOOST_VERSION > 105500
	boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#else
	boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#endif /* BOOST_VERSION */
	boost::property_tree::write_xml(parameterfileName, ptr_out, std::locale(), settings);

	// Collect all command-line arguments
	std::vector<std::string> arguments;
	if (m_custom_options != "empty" && !m_custom_options.empty()) {
		arguments.push_back(m_custom_options);
	}
	arguments.push_back(std::string("--evaluate"));
	arguments.push_back(std::string("--input=\"") + parameterfileName + "\"");
	arguments.push_back(std::string("--output=\"") + resultFileName + "\"");

	// Perform the external evaluation
	double main_result = 0.;
	std::string command;
	int errorCode = Gem::Common::runExternalCommand(
		boost::filesystem::path(m_program_name), arguments, boost::filesystem::path(commandOutputFileName), command
	);

	if (errorCode) { // Something went wrong
		glogger
			<< "In GExternalEvaluatorIndividual::fitnessCalculation():" << std::endl
			<< "Execution of external command failed." << std::endl
			<< "Command: " << command << std::endl
			<< "Error code: " << errorCode << std::endl
			<< "Program output:" << std::endl
			<< Gem::Common::loadTextDataFromFile(commandOutputFileName) << std::endl
			<< GWARNING;

		// O.k., so the external application crashed or returned an error.
		// All we can do here is to return the worst case. As long as crashes
		// do not happen too often, this will have but little influence on
		// the optimization.
		main_result = this->getWorstCase();
		for (std::size_t res = 1; res < m_n_results; res++) {
			this->registerSecondaryResult(res, this->getWorstCase());
		}

		// Make sure the individual can be recognized as invalid by Geneva
		this->markAsInvalid();
	} else { // Everything is o.k., lets retrieve the evaluation
		// Check that the result file exists
		if (!bf::exists(resultFileName)) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
					<< "Result file " << resultFileName << " does not seem to exist." << std::endl
			);
		}

		// Parse the results
		boost::property_tree::ptree ptr_in; // A property tree object;
		try {
			pt::read_xml(resultFileName, ptr_in);
		} catch (const boost::property_tree::xml_parser::xml_parser_error &e) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error  " << std::endl
					<< "Caught boost::property_tree::xml_parser::xml_parser_error" << std::endl
					<< "for file " << e.filename() << " (line " << e.line() << ")" << std::endl
			);
		} catch (const std::exception &e) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error reading " << resultFileName << std::endl
					<< "with message " << e.what() << std::endl
			);
		}

		// Check that only a single result was returned
		std::size_t nExternalIndividuals = ptr_in.get<std::size_t>(batch + ".nIndividuals");
		if (1 != nExternalIndividuals) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
					<< "Number of result individuals != 1: " << nExternalIndividuals << std::endl
			);
		}

		// Check that the number of results provided by the result file matches the number of expected results
		std::size_t externalNResults = ptr_in.get<std::size_t>("batch.individuals.individual0.nResults");
		if (externalNResults != m_n_results) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
					<< "Result file provides nResults = " << externalNResults << std::endl
					<< "while we expected " << m_n_results << std::endl
			);
		}

		// Check that the evaluation id matches our local id
		std::string externalEvaluationID = ptr_in.get<std::string>("batch.individuals.individual0.id");
		if (externalEvaluationID != this->getCurrentEvaluationID()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
					<< "Local evaluation id " << this->getCurrentEvaluationID() << " does not match external id " <<
					externalEvaluationID << std::endl
			);
		}

		// Check whether the results represent useful values
		bool isValid = ptr_in.get<bool>("batch.individuals.individual0.isValid");
		if (!isValid) { // Assign worst-case values to all result
			main_result = this->getWorstCase();
			for (std::size_t res = 1; res < m_n_results; res++) {
				this->registerSecondaryResult(res, this->getWorstCase());
			}

			// Make sure the individual can be recognized as invalid by Geneva
			this->markAsInvalid();
		} else { // Extract and store all result values
			// Get the results node
			pt::ptree resultsNode = ptr_in.get_child("batch.individuals.individual0.results");

			double currentResult = 0.;
			std::string resultString;
			for (std::size_t res = 0; res < m_n_results; res++) {
				resultString = std::string("rawResult") + boost::lexical_cast<std::string>(res);

				currentResult = resultsNode.get<double>(resultString);

				if (res == 0) {
					main_result = currentResult;
				}
				else {
					this->registerSecondaryResult(res, currentResult);
				}
			}
		}
	}

	// Clean up (remove) parameter-, result- and command-output files, if requested by the user
	if (m_remove_exec_temporaries) {
		bf::remove(parameterfileName);
		bf::remove(resultFileName);
		bf::remove(commandOutputFileName);
	}

	// Return the master result (first result returned)
	return main_result;
}

/******************************************************************************/
/**
 * Allows to assign a run-id to this individual
 */
void GExternalEvaluatorIndividual::setRunId(std::string runID) {
	if (runID.empty() || "empty" == runID) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividual::setRunId(): Error!" << std::endl
				<< "Attempt to set an invalid run id: \"" << runID << "\"" << std::endl
		);
	}

	runID_ = runID;
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
void GExternalEvaluatorIndividual::setRemoveExecTemporaries(bool removeExecTemporaries) {
	m_remove_exec_temporaries = removeExecTemporaries;
}

/******************************************************************************/
/**
 * Allows to check whether temporaries should be removed
 */
bool GExternalEvaluatorIndividual::getRemoveExecTemporaries() const {
	return m_remove_exec_temporaries;
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
 * @param configFile The name of the configuration file
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory(
	const std::string &configFile
)
	: Gem::Common::GFactoryT<GParameterSet>(configFile), m_adProb(GEEI_DEF_ADPROB), m_adaptAdProb(GEEI_DEF_ADAPTADPROB),
	m_minAdProb(GEEI_DEF_MINADPROB), m_maxAdProb(GEEI_DEF_MAXADPROB), m_adaptionThreshold(GEEI_DEF_ADAPTIONTHRESHOLD),
	m_useBiGaussian(GEEI_DEF_USEBIGAUSSIAN), m_sigma1(GEEI_DEF_SIGMA1), m_sigmaSigma1(GEEI_DEF_SIGMASIGMA1),
	m_minSigma1(GEEI_DEF_MINSIGMA1), m_maxSigma1(GEEI_DEF_MAXSIGMA1), m_sigma2(GEEI_DEF_SIGMA2),
	m_sigmaSigma2(GEEI_DEF_SIGMASIGMA2), m_minSigma2(GEEI_DEF_MINSIGMA2), m_maxSigma2(GEEI_DEF_MAXSIGMA2),
	m_delta(GEEI_DEF_DELTA), m_sigmaDelta(GEEI_DEF_SIGMADELTA), m_minDelta(GEEI_DEF_MINDELTA),
	m_maxDelta(GEEI_DEF_MAXDELTA), m_programName(GEEI_DEF_PROGNAME), m_customOptions(GEEI_DEF_CUSTOMOPTIONS),
	m_parameterFileBaseName(GEEI_DEF_PARFILEBASENAME), m_initValues(GEEI_DEF_STARTMODE),
	m_removeExecTemporaries(GEEI_DEF_REMOVETEMPORARIES), m_externalEvaluatorQueried(false) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory(
	const GExternalEvaluatorIndividualFactory &cp
)
	: Gem::Common::GFactoryT<GParameterSet>(cp), m_adProb(cp.m_adProb), m_adaptAdProb(cp.m_adaptAdProb),
	m_minAdProb(cp.m_minAdProb), m_maxAdProb(cp.m_maxAdProb), m_adaptionThreshold(cp.m_adaptionThreshold),
	m_useBiGaussian(cp.m_useBiGaussian), m_sigma1(cp.m_sigma1), m_sigmaSigma1(cp.m_sigmaSigma1), m_minSigma1(cp.m_minSigma1),
	m_maxSigma1(cp.m_maxSigma1), m_sigma2(cp.m_sigma2), m_sigmaSigma2(cp.m_sigmaSigma2), m_minSigma2(cp.m_minSigma2),
	m_maxSigma2(cp.m_maxSigma2), m_delta(cp.m_delta), m_sigmaDelta(cp.m_sigmaDelta), m_minDelta(cp.m_minDelta),
	m_maxDelta(cp.m_maxDelta), m_programName(cp.m_programName), m_customOptions(cp.m_customOptions),
	m_parameterFileBaseName(cp.m_parameterFileBaseName), m_initValues(cp.m_initValues),
	m_removeExecTemporaries(cp.m_removeExecTemporaries), m_externalEvaluatorQueried(cp.m_externalEvaluatorQueried),
	m_ptr(cp.m_ptr) { /* nothing */ }

/******************************************************************************/
/**
 * The default constructor. Only needed for (de-)serialization purposes, hence empty.
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory()
	: Gem::Common::GFactoryT<GParameterSet>("empty"), m_adProb(GEEI_DEF_ADPROB), m_adaptAdProb(GEEI_DEF_ADAPTADPROB),
	m_minAdProb(GEEI_DEF_MINADPROB), m_maxAdProb(GEEI_DEF_MAXADPROB), m_adaptionThreshold(GEEI_DEF_ADAPTIONTHRESHOLD),
	m_useBiGaussian(GEEI_DEF_USEBIGAUSSIAN), m_sigma1(GEEI_DEF_SIGMA1), m_sigmaSigma1(GEEI_DEF_SIGMASIGMA1),
	m_minSigma1(GEEI_DEF_MINSIGMA1), m_maxSigma1(GEEI_DEF_MAXSIGMA1), m_sigma2(GEEI_DEF_SIGMA2),
	m_sigmaSigma2(GEEI_DEF_SIGMASIGMA2), m_minSigma2(GEEI_DEF_MINSIGMA2), m_maxSigma2(GEEI_DEF_MAXSIGMA2),
	m_delta(GEEI_DEF_DELTA), m_sigmaDelta(GEEI_DEF_SIGMADELTA), m_minDelta(GEEI_DEF_MINDELTA),
	m_maxDelta(GEEI_DEF_MAXDELTA), m_programName(GEEI_DEF_PROGNAME), m_customOptions(GEEI_DEF_CUSTOMOPTIONS),
	m_parameterFileBaseName(GEEI_DEF_PARFILEBASENAME), m_initValues(GEEI_DEF_STARTMODE),
	m_removeExecTemporaries(GEEI_DEF_REMOVETEMPORARIES), m_externalEvaluatorQueried(false) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor. Note that, if the external evaluator, when called with the
 * --finalize switch, does anything making optimization impossible, the factory
 * should not be destroyed before the end of the optimization run.
 */
GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory() {
	// Check that the file name isn't empty
	if (m_programName.value().empty()) {
		glogger
			<< "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): Error!" << std::endl
			<< "Program name was empty" << std::endl
			<< GTERMINATION;
	}

	// Check that the file exists
	if (!bf::exists(m_programName.value())) {
		glogger
			<< "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): Error!" << std::endl
			<< "External program " << m_programName.value() << " does not seem to exist" << std::endl
			<< GTERMINATION;
	}

	// Collect all command-line arguments
	std::vector<std::string> arguments;
	if (m_customOptions.value() != "empty" && !m_customOptions.value().empty()) {
		arguments.push_back(m_customOptions.value());
	}
	arguments.push_back(std::string("--finalize"));

	// Ask the external evaluation program to perform any final work
	std::string command;
	int errorCode = Gem::Common::runExternalCommand(
		boost::filesystem::path(m_programName.value()), arguments, boost::filesystem::path(), command
	);

	// Let the audience know
	if (errorCode) {
		glogger
			<< "In GExternalEvaluatorIndividual::~GExternalEvaluatorIndividualFactory(): Error" << std::endl
			<< "Execution of external command failed." << std::endl
			<< "Command: " << command << std::endl
			<< "Error code: " << errorCode << std::endl
			<< GTERMINATION;
	}
}

/******************************************************************************/
/**
 * Loads the data of another GFunctionIndividualFactory object
 */
void GExternalEvaluatorIndividualFactory::load(std::shared_ptr < Gem::Common::GFactoryT<GParameterSet>> cp_raw_ptr) {
	// Load our parent class'es data
	Gem::Common::GFactoryT<GParameterSet>::load(cp_raw_ptr);

	// Convert the base pointer
	std::shared_ptr <GExternalEvaluatorIndividualFactory> cp_ptr
		= Gem::Common::convertSmartPointer<Gem::Common::GFactoryT<GParameterSet>, GExternalEvaluatorIndividualFactory>(
			cp_raw_ptr);

	// And then our own
	m_adProb = cp_ptr->m_adProb;
	m_adaptAdProb = cp_ptr->m_adaptAdProb;
	m_minAdProb = cp_ptr->m_minAdProb;
	m_maxAdProb = cp_ptr->m_maxAdProb;
	m_adaptionThreshold = cp_ptr->m_adaptionThreshold;
	m_useBiGaussian = cp_ptr->m_useBiGaussian;
	m_sigma1 = cp_ptr->m_sigma1;
	m_sigmaSigma1 = cp_ptr->m_sigmaSigma1;
	m_minSigma1 = cp_ptr->m_minSigma1;
	m_maxSigma1 = cp_ptr->m_maxSigma1;
	m_sigma2 = cp_ptr->m_sigma2;
	m_sigmaSigma2 = cp_ptr->m_sigmaSigma2;
	m_minSigma2 = cp_ptr->m_minSigma2;
	m_maxSigma2 = cp_ptr->m_maxSigma2;
	m_delta = cp_ptr->m_delta;
	m_sigmaDelta = cp_ptr->m_sigmaDelta;
	m_minDelta = cp_ptr->m_minDelta;
	m_maxDelta = cp_ptr->m_maxDelta;
	m_programName = cp_ptr->m_programName;
	m_customOptions = cp_ptr->m_customOptions;
	m_parameterFileBaseName = cp_ptr->m_parameterFileBaseName;
	m_initValues = cp_ptr->m_initValues;
	m_removeExecTemporaries = cp_ptr->m_removeExecTemporaries;
	m_externalEvaluatorQueried = cp_ptr->m_externalEvaluatorQueried;
	m_ptr = cp_ptr->m_ptr;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> GExternalEvaluatorIndividualFactory::clone() const {
	return std::shared_ptr<GExternalEvaluatorIndividualFactory>(new GExternalEvaluatorIndividualFactory(*this));
}

/******************************************************************************/
/**
 * Get the value of the adaptionThreshold_ variable
 */
std::uint32_t GExternalEvaluatorIndividualFactory::getAdaptionThreshold() const {
	return m_adaptionThreshold;
}

/******************************************************************************/
/**
 * Set the value of the adaptionThreshold_ variable
 */
void GExternalEvaluatorIndividualFactory::setAdaptionThreshold(
	std::uint32_t adaptionThreshold) {
	m_adaptionThreshold = adaptionThreshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the adProb_ variable
 */
double GExternalEvaluatorIndividualFactory::getAdProb() const {
	return m_adProb;
}

/******************************************************************************/
/**
 * Set the value of the adProb_ variable
 */
void GExternalEvaluatorIndividualFactory::setAdProb(double adProb) {
	m_adProb = adProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the rate of evolutionary adaption of adProb_
 */
double GExternalEvaluatorIndividualFactory::getAdaptAdProb() const {
	return m_adaptAdProb;
}

/******************************************************************************/
/**
 * Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature)
 */
void GExternalEvaluatorIndividualFactory::setAdaptAdProb(double adaptAdProb) {
#ifdef DEBUG
	if(adaptAdProb < 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setAdaptAdProb(): Error!" << std::endl
				<< "Invalid value for adaptAdProb given: " << adaptAdProb << std::endl
		);
	}
#endif /* DEBUG */

	m_adaptAdProb = adaptAdProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for adProb_ variation
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getAdProbRange() const {
	return std::tuple<double, double>(m_minAdProb.value(), m_maxAdProb.value());
}

/******************************************************************************/
/**
 * Allows to set the allowed range for adaption probability variation
 */
void GExternalEvaluatorIndividualFactory::setAdProbRange(double minAdProb, double maxAdProb) {
#ifdef DEBUG
	if(minAdProb < 0.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "minAdProb < 0: " << minAdProb << std::endl
		);
	}

	if(minAdProb > maxAdProb) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb << std::endl
		);
	}

	if(maxAdProb > 1.) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << std::endl
				<< "maxAdProb > 1: " << maxAdProb << std::endl
		);
	}
#endif /* DEBUG */

	m_minAdProb = minAdProb;
	m_maxAdProb = maxAdProb;
}


/******************************************************************************/
/**
 * Allows to retrieve the delta_ variable
 */
double GExternalEvaluatorIndividualFactory::getDelta() const {
	return m_delta;
}

/******************************************************************************/
/**
 * Set the value of the delta_ variable
 */
void GExternalEvaluatorIndividualFactory::setDelta(double delta) {
	m_delta = delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxDelta() const {
	return m_maxDelta;
}

/******************************************************************************/
/**
 * Set the value of the maxDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxDelta(double maxDelta) {
	m_maxDelta = maxDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxSigma1() const {
	return m_maxSigma1;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxSigma1(double maxSigma1) {
	m_maxSigma1 = maxSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxSigma2() const {
	return m_maxSigma2;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxSigma2(double maxSigma2) {
	m_maxSigma2 = maxSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the minDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getMinDelta() const {
	return m_minDelta;
}

/******************************************************************************/
/**
 * Set the value of the minDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinDelta(double minDelta) {
	m_minDelta = minDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of delta
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getDeltaRange() const {
	return std::tuple<double, double>(m_minDelta, m_maxDelta);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of delta
 */
void GExternalEvaluatorIndividualFactory::setDeltaRange(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setDeltaRange(): Error" << std::endl
				<< "min must be >= 0. Got : " << max << std::endl
		);
	}

	if (min >= max) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setDeltaRange(): Error" << std::endl
				<< "Invalid range specified: " << min << " / " << max << std::endl
		);
	}

	m_minDelta = min;
	m_maxDelta = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minSigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getMinSigma1() const {
	return m_minSigma1;
}

/******************************************************************************/
/**
 * Set the value of the minSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinSigma1(double minSigma1) {
	m_minSigma1 = minSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma1_
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getSigma1Range() const {
	return std::tuple<double, double>(m_minSigma1, m_maxSigma1);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma1_
 */
void GExternalEvaluatorIndividualFactory::setSigma1Range(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setSigma1Range(): Error" << std::endl
				<< "min must be >= 0. Got : " << max << std::endl
		);
	}

	if (min >= max) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setSigma1Range(): Error" << std::endl
				<< "Invalid range specified: " << min << " / " << max << std::endl
		);
	}

	m_minSigma1 = min;
	m_maxSigma1 = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the minSigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getMinSigma2() const {
	return m_minSigma2;
}

/******************************************************************************/
/**
 * Set the value of the minSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinSigma2(double minSigma2) {
	m_minSigma2 = minSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma2_
 */
std::tuple<double, double> GExternalEvaluatorIndividualFactory::getSigma2Range() const {
	return std::tuple<double, double>(m_minSigma2, m_maxSigma2);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma2_
 */
void GExternalEvaluatorIndividualFactory::setSigma2Range(std::tuple<double, double> range) {
	double min = std::get<0>(range);
	double max = std::get<1>(range);

	if (min < 0) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setSigma2Range(): Error" << std::endl
				<< "min must be >= 0. Got : " << max << std::endl
		);
	}

	if (min >= max) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setSigma2Range(): Error" << std::endl
				<< "Invalid range specified: " << min << " / " << max << std::endl
		);
	}

	m_minSigma2 = min;
	m_maxSigma2 = max;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigma1() const {
	return m_sigma1;
}

/******************************************************************************/
/**
 * Set the value of the sigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigma1(double sigma1) {
	m_sigma1 = sigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigma2() const {
	return m_sigma2;
}

/******************************************************************************/
/**
 * Set the value of the sigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigma2(double sigma2) {
	m_sigma2 = sigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigmaDelta() const {
	return m_sigmaDelta;
}

/******************************************************************************/
/**
 * Set the value of the sigmaDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigmaDelta(double sigmaDelta) {
	m_sigmaDelta = sigmaDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaSigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigmaSigma1() const {
	return m_sigmaSigma1;
}

/******************************************************************************/
/**
 * Set the value of the sigmaSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigmaSigma1(double sigmaSigma1) {
	m_sigmaSigma1 = sigmaSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the sigmaSigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getSigmaSigma2() const {
	return m_sigmaSigma2;
}

/******************************************************************************/
/**
 * Set the value of the sigmaSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setSigmaSigma2(double sigmaSigma2) {
	m_sigmaSigma2 = sigmaSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the useBiGaussian_ variable
 */
bool GExternalEvaluatorIndividualFactory::getUseBiGaussian() const {
	return m_useBiGaussian;
}

/******************************************************************************/
/**
 * Set the value of the useBiGaussian_ variable
 */
void GExternalEvaluatorIndividualFactory::setUseBiGaussian(bool useBiGaussian) {
	m_useBiGaussian = useBiGaussian;
}

/******************************************************************************/
/**
 * Allows to set the name and path of the external program. Note that this will have a
 * lasting effect even if the external configuration file is parsed repeatedly,
 * as we use a "one-time-reference" parameter. Using the "setValue" option woll in
 * contrast reset the internal value of that object.
 */
void GExternalEvaluatorIndividualFactory::setProgramName(std::string programName) {
	// Check that the file name isn't empty
	if (programName.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << std::endl
				<< "File name was empty" << std::endl
		);
	}

	// Check that the file exists
	if (!bf::exists(programName)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << std::endl
				<< "External program " << programName << " does not seem to exist" << std::endl
		);
	}

	m_programName.setValue(programName);
}

/******************************************************************************/
/**
 * Allows to retrieve the name of the external program
 */
std::string GExternalEvaluatorIndividualFactory::getProgramName() const {
	return m_programName;
}

/******************************************************************************/
/**
 * Sets the name of the external evaluation program. Note that this will have a
 * lasting effect even if the external configuration file is parsed repeatedly,
 * as we use a "one-time-reference" parameter. Using the "setValue" option woll in
 * contrast reset the internal value of that object.
 */
void GExternalEvaluatorIndividualFactory::setCustomOptions(std::string customOptions) {
	m_customOptions.setValue(customOptions);
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividualFactory::getCustomOptions() const {
	return m_customOptions;
}

/******************************************************************************/
/**
 * Allows to set the base name of the parameter file
 */
void GExternalEvaluatorIndividualFactory::setParameterFileBaseName(std::string parameterFileBaseName) {
	// Check that the name isn't empty
	if (parameterFileBaseName.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setParameterFileBaseName(): Error!" << std::endl
				<< "Name was empty" << std::endl
		);
	}

	m_parameterFileBaseName = parameterFileBaseName;
}

/******************************************************************************/
/**
 * Allows to retrieve the base name of the parameter file
 */
std::string GExternalEvaluatorIndividualFactory::getParameterFileBaseName() const {
	return m_parameterFileBaseName;
}

/******************************************************************************/
/**
 * Indicates the initialization mode
 *
 * TODO: Allow "none" in case parameters should be solely supplied by the external evaluator
 */
void GExternalEvaluatorIndividualFactory::setInitValues(std::string initValues) {
	if (initValues != "random" && initValues != "min" && initValues != "max") {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setInitValues(): Error!" << std::endl
				<< "Invalid argument: " << initValues << std::endl
				<< "Expected \"min\", \"max\", or \"random\"." << std::endl
		);
	}

	m_initValues.setValue(initValues);
}

/******************************************************************************/
/**
 * Allows to retrieve the initialization mode
 */
std::string GExternalEvaluatorIndividualFactory::getInitValues() const {
	return m_initValues;
}

/******************************************************************************/
/**
 * Allows to specify whether temporary files should be removed
 */
void GExternalEvaluatorIndividualFactory::setRemoveExecTemporaries(bool removeExecTemporaries) {
	m_removeExecTemporaries.setValue(removeExecTemporaries);
}

/******************************************************************************/
/**
 * Allows to check whether temporaries should be removed
 */
bool GExternalEvaluatorIndividualFactory::getRemoveExecTemporaries() const {
	return m_removeExecTemporaries;
}

/******************************************************************************/
/**
 * Submit work items to the external executable for archiving
 */
void GExternalEvaluatorIndividualFactory::archive(
	const std::vector<std::shared_ptr<GExternalEvaluatorIndividual>>& arch
) const {
	// Check that there are individuals contained in the archive
	if(arch.empty()) return; // Do nothing

	// Transform the objects into a batch of boost property tree
	boost::property_tree::ptree ptr_out;
	std::string batch = "batch";

	// Output the header data
	ptr_out.put(batch + ".dataType", std::string("archive_data"));
	ptr_out.put(batch + ".runID", arch.front() -> getRunId());
	ptr_out.put(batch + ".nIndividuals", arch.size());

	// Output the individuals in turn
	std::vector<std::shared_ptr<GExternalEvaluatorIndividual>>::const_iterator cit;
	std::size_t pos = 0;
	std::string basename;
	for(
		cit = arch.begin();
		cit!=arch.

			end();

		++cit) {
		basename = batch + ".individuals.individual" + boost::lexical_cast<std::string>(pos++);
		(*cit)->
			toPropertyTree(ptr_out, basename
		);
		std::cout << "Current evaluation id = " << (*cit)->

			getCurrentEvaluationID()

					 <<
					 std::endl;
	}

	// Create a suitable extension and exchange file names for this object
	std::chrono::time_point<std::chrono::high_resolution_clock> p1;
	std::chrono::time_point<std::chrono::high_resolution_clock> p2 = std::chrono::high_resolution_clock::now();
	std::chrono::milliseconds ms_since_1970 = std::chrono::duration_cast<std::chrono::milliseconds>(p2 - p1);
	std::string extension =
		"-since1970-"
		+ boost::lexical_cast<std::string>(ms_since_1970.count())
		+ boost::lexical_cast<std::string>(boost::uuids::random_generator()()) + ".xml";
	std::string parameterfileName = m_parameterFileBaseName.value() + extension;

// Save the parameters to a file for the external evaluation
#if BOOST_VERSION > 105500
	boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#else
	boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#endif /* BOOST_VERSION */
	boost::property_tree::write_xml(parameterfileName, ptr_out, std::locale(), settings
	);


	// Collect all command-line arguments
	std::vector<std::string> arguments;
	if(m_customOptions.value() != "empty" && !m_customOptions.value().empty()) {
		arguments.push_back(m_customOptions.value());
	}
	arguments.push_back(std::string("--archive"));
	arguments.push_back(std::string("--input=\"" + parameterfileName + "\""));

	// Ask the external evaluation program to perform any final work
	std::string command;
	int errorCode = Gem::Common::runExternalCommand(
		boost::filesystem::path(m_programName.value())
		, arguments
		, boost::filesystem::path()
		, command
	);

	// Let the audience know
	if(errorCode) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::archive(): Error" << std::endl
				<< "Execution of external command failed." << std::endl
				<< "Command: " << command << std::endl
				<< "Error code: " << errorCode << std::endl
		);
	}

	// Clean up (remove) the parameter file. This will only be done if no error occurred
	bf::remove(parameterfileName);
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr <GParameterSet> GExternalEvaluatorIndividualFactory::getObject_(
	Gem::Common::GParserBuilder &gpb
	, const std::size_t &id
) {
	// Will hold the result
	std::shared_ptr <GExternalEvaluatorIndividual> target(new GExternalEvaluatorIndividual());

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
	gpb.registerFileParameter<double>(
		"adProb", m_adProb.reference(), GEEI_DEF_ADPROB
	)
		<< "The probability for random adaption of values in evolutionary algorithms";

	gpb.registerFileParameter<double>(
		"adaptAdProb", m_adaptAdProb.reference(), GEEI_DEF_ADAPTADPROB
	)
		<< "Determines the rate of adaption of adProb. Set to 0, if you do not need this feature";

	gpb.registerFileParameter<double>(
		"minAdProb", m_minAdProb.reference(), GEEI_DEF_MINADPROB
	)
		<< "The lower allowed boundary for adProb-variation";

	gpb.registerFileParameter<double>(
		"maxAdProb", m_maxAdProb.reference(), GEEI_DEF_MAXADPROB
	)
		<< "The upper allowed boundary for adProb-variation";


	gpb.registerFileParameter<std::uint32_t>(
		"adaptionThreshold", m_adaptionThreshold.reference(), GEEI_DEF_ADAPTIONTHRESHOLD
	)
		<< "The number of calls to an adaptor after which adaption takes place";

	gpb.registerFileParameter<bool>(
		"useBiGaussian", m_useBiGaussian.reference(), GEEI_DEF_USEBIGAUSSIAN
	)
		<< "Whether to use a double gaussion for the adaption of parmeters in ES";

	gpb.registerFileParameter<double>(
		"sigma1", m_sigma1.reference(), GEEI_DEF_SIGMA1
	)
		<< "The sigma for gauss-adaption in ES" << std::endl
		<< "(or the sigma of the left peak of a double gaussian)";

	gpb.registerFileParameter<double>(
		"sigmaSigma1", m_sigmaSigma1.reference(), GEEI_DEF_SIGMASIGMA1
	)
		<< "Influences the self-adaption of gauss-mutation in ES";

	gpb.registerFileParameter<double>(
		"minSigma1", m_minSigma1.reference(), GEEI_DEF_MINSIGMA1
	)
		<< "The minimum value of sigma1";

	gpb.registerFileParameter<double>(
		"maxSigma1", m_maxSigma1.reference(), GEEI_DEF_MAXSIGMA1
	)
		<< "The maximum value of sigma1";

	gpb.registerFileParameter<double>(
		"sigma2", m_sigma2.reference(), GEEI_DEF_SIGMA2
	)
		<< "The sigma of the right peak of a double gaussian (if any)";

	gpb.registerFileParameter<double>(
		"sigmaSigma2", m_sigmaSigma2.reference(), GEEI_DEF_SIGMASIGMA2
	)
		<< "Influences the self-adaption of gauss-mutation in ES";

	gpb.registerFileParameter<double>(
		"minSigma2", m_minSigma2.reference(), GEEI_DEF_MINSIGMA2
	)
		<< "The minimum value of sigma2";

	gpb.registerFileParameter<double>(
		"maxSigma2", m_maxSigma2.reference(), GEEI_DEF_MAXSIGMA2
	)
		<< "The maximum value of sigma2";

	gpb.registerFileParameter<double>(
		"delta", m_delta.reference(), GEEI_DEF_DELTA
	)
		<< "The start distance between both peaks used for bi-gaussian mutations in ES";

	gpb.registerFileParameter<double>(
		"sigmaDelta", m_sigmaDelta.reference(), GEEI_DEF_SIGMADELTA
	)
		<< "The width of the gaussian used for mutations of the delta parameter";

	gpb.registerFileParameter<double>(
		"minDelta", m_minDelta.reference(), GEEI_DEF_MINDELTA
	)
		<< "The minimum allowed value of delta";

	gpb.registerFileParameter<double>(
		"maxDelta", m_maxDelta.reference(), GEEI_DEF_MAXDELTA
	)
		<< "The maximum allowed value of delta";

	gpb.registerFileParameter<std::string>(
		"programName", m_programName.reference() // Upon repeated filling this option will do nothing
		, GEEI_DEF_PROGNAME
	)
		<< "The name of the external evaluation program";

	gpb.registerFileParameter<std::string>(
		"customOptions", m_customOptions.reference(), GEEI_DEF_CUSTOMOPTIONS
	)
		<< "Any custom options you wish to pass to the external evaluator";

	gpb.registerFileParameter<std::string>(
		"parameterFile", m_parameterFileBaseName.reference(), GEEI_DEF_PARFILEBASENAME
	)
		<< "The base name assigned to parameter files" << std::endl
		<< "in addition to data identifying this specific evaluation";

	gpb.registerFileParameter<std::string>(
		"initValues", m_initValues.reference(), GEEI_DEF_STARTMODE
	)
		<< "Indicates, whether individuals should be initialized randomly (random)," << std::endl
		<< "with the lower (min) or upper (max) boundary of their value ranges";

	gpb.registerFileParameter<bool>(
		"removeExecTemporaries", m_removeExecTemporaries.reference(), GEEI_DEF_REMOVETEMPORARIES
	)
		<< "Indicates, whether files created during external execution should be removed";
}

/******************************************************************************/
/**
 * This function asks the external evaluation program to perform any necessary
 * setup work and then queries it for setup-information, storing the data in
 * a boost::property_tree object. Note that this function will do nothing when
 * called more than once.
 */
void GExternalEvaluatorIndividualFactory::setUpPropertyTree() {
	if (m_externalEvaluatorQueried) return;
	else m_externalEvaluatorQueried = true;

	// Check that the file name isn't empty
	if (m_programName.value().empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << std::endl
				<< "File name was empty" << std::endl
		);
	}

	// Check that the file exists
	if (!bf::exists(m_programName.value())) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << std::endl
				<< "External program " << m_programName.value() << " does not seem to exist" << std::endl
		);
	}

	// Make sure the property tree is empty
	m_ptr.clear();

	{ // First we give the external program the opportunity to perform an initial work
		// Collect all command-line arguments
		std::vector<std::string> arguments;
		if (m_customOptions.value() != "empty" && !m_customOptions.value().empty()) {
			arguments.push_back(m_customOptions.value());
		}
		arguments.push_back(std::string("--init"));

		// Ask the external evaluation program to perform any initial work
		std::string command;
		int errorCode = Gem::Common::runExternalCommand(
			boost::filesystem::path(m_programName.value()), arguments, boost::filesystem::path(), command
		);

		if (errorCode) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::setUpPropertyTree(//1//): Error" << std::endl
					<< "Execution of external command failed." << std::endl
					<< "Command: " << command << std::endl
					<< "Error code: " << errorCode << std::endl
			);
		}
	}

	{ // Now we ask the external program for setup-iformation
		// Collect all command-line arguments
		std::vector<std::string> arguments;
		if (m_customOptions.value() != "empty" && !m_customOptions.value().empty()) {
			arguments.push_back(m_customOptions.value());
		}

		// "/" will be converted to "\" in runExternalCommand, if necessary
		std::string setupFileName =
			std::string("./setup-") + boost::lexical_cast<std::string>(this) + std::string(".xml");
		arguments.push_back("--setup");
		arguments.push_back("--output=\"" + setupFileName + "\"");
		arguments.push_back("--initvalues=\"" + m_initValues.value() + "\"");

		// Ask the external evaluation program tfor setup information
		std::string command;
		int errorCode = Gem::Common::runExternalCommand(
			boost::filesystem::path(m_programName.value()), arguments, boost::filesystem::path(), command
		);

		if (errorCode) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividual::setUpPropertyTree(//2//): Error" << std::endl
					<< "Execution of external command failed." << std::endl
					<< "Command: " << command << std::endl
					<< "Error code: " << errorCode << std::endl
			);
		}

		// Parse the setup file
		pt::read_xml(setupFileName, m_ptr);

		// Clean up
		bf::remove(boost::filesystem::path(setupFileName));
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
void GExternalEvaluatorIndividualFactory::postProcess_(std::shared_ptr < GParameterSet > &p_raw) {
	using boost::property_tree::ptree;

	// Convert the base pointer to the target type
	std::shared_ptr <GExternalEvaluatorIndividual> p
		= Gem::Common::convertSmartPointer<GParameterSet, GExternalEvaluatorIndividual>(p_raw);

	// Set up a random number generator
	Gem::Hap::GRandom gr;

	// Here we ask the external evaluator to perform any necessary setup work
	// and query it for the desired structure of our individuals. The work is done
	// but once, and the results are stored in a private object inside of this class.
	this->setUpPropertyTree();

	if (m_ptr.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
				<< "Property tree is empty." << std::endl
		);
	}

	// Set up an adaptor for the collection, so they know how to be adapted
	std::shared_ptr <GAdaptorT<double>> gat_ptr;
	if (m_useBiGaussian) {
		std::shared_ptr <GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());
		gdbga_ptr->setAllSigma1(m_sigma1, m_sigmaSigma1, m_minSigma1, m_maxSigma1);
		gdbga_ptr->setAllSigma2(m_sigma2, m_sigmaSigma2, m_minSigma2, m_maxSigma2);
		gdbga_ptr->setAllDelta(m_delta, m_sigmaDelta, m_minDelta, m_maxDelta);
		gdbga_ptr->setAdaptionThreshold(m_adaptionThreshold);
		gdbga_ptr->setAdaptionProbability(m_adProb);
		gat_ptr = gdbga_ptr;
	} else {
		std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(
			new GDoubleGaussAdaptor(m_sigma1, m_sigmaSigma1, m_minSigma1, m_maxSigma1));
		gdga_ptr->setAdaptionThreshold(m_adaptionThreshold);
		gdga_ptr->setAdaptionProbability(m_adProb);
		gat_ptr = gdga_ptr;
	}

	// Store parameters pertaining to the adaption probability in the adaptor
	gat_ptr->setAdaptAdProb(m_adaptAdProb);
	gat_ptr->setAdProbRange(m_minAdProb, m_maxAdProb);

	try {
		// Extract the number of individuals
		std::size_t nIndividuals = m_ptr.get<std::size_t>("batch.nIndividuals");
		if (1 != nIndividuals) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
					<< "Received invalid number of setup-individuals: " << nIndividuals << std::endl
			);
		}

		// Get the run-id
		std::string runID = m_ptr.get<std::string>("batch.runID");

		// Extract the number of variables for the first individual
		std::size_t nVar = m_ptr.get<std::size_t>("batch.individuals.individual0.nVars");

		// Extract the number of results to be expected from the external evaluation function for the first individual
		std::size_t nResultsExpected = m_ptr.get<std::size_t>("batch.individuals.individual0.nResults");

		// Extract the number of boundaries to be expected for the first individual
		std::size_t nBounds = m_ptr.get<std::size_t>("batch.individuals.individual0.nBounds");

		// Get an iterator over a property tree
		ptree::const_iterator cit;

		// If variables have been specified, extract them
		boost::optional<ptree &> varSetNode_opt = m_ptr.get_child_optional("batch.individuals.individual0.vars");
		if (varSetNode_opt) {
			// Loop over all children of the variables tree
			// Note that for now we only query GConstrainedDoubleObject objects
			std::size_t varCounter = 0;
			std::string varString = "var0";
			for (cit = (*varSetNode_opt).begin(); cit != (*varSetNode_opt).end(); ++cit) {
				if (varString == cit->first) { // O.k., we found a varX string
					// Just treat GConstrainedDoubleObject objects for now
					if ("GConstrainedDoubleObject" == (cit->second).get<std::string>("type")) {
						// Extract the boundaries and initial values
						std::string pName = (cit->second).get<std::string>("name");
						double minVar = (cit->second).get<double>("lowerBoundary");
						double maxVar = (cit->second).get<double>("upperBoundary");
						double initValue = (cit->second).get<double>("values.value0");

						// Create an initial (empty) pointer to a GConstrainedDoubleObject
						std::shared_ptr <GConstrainedDoubleObject> gcdo_ptr;

						// Act on the information, depending on whether random initialization has been requested
						if (minVar == maxVar) { // We take this as a sign that the parameter should not be modified
							// Create the parameter object
							gcdo_ptr = std::shared_ptr<GConstrainedDoubleObject>(
								new GConstrainedDoubleObject(
									initValue, initValue, Gem::Common::gmax(1.0001 * initValue, initValue + 0.0001)
								)
							);
							// Disable mutations
							gcdo_ptr->setAdaptionsInactive();
						} else if (0 == (cit->second).count("initRandom") || false == (cit->second).get<bool>("initRandom")) {
							// Create the parameter object
							gcdo_ptr = std::shared_ptr<GConstrainedDoubleObject>(
								new GConstrainedDoubleObject(initValue, minVar, maxVar));
						} else { // Random initialization has been requested
							// Create the parameter object
							gcdo_ptr = std::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(minVar, maxVar));
						}
						gcdo_ptr->setParameterName(pName);

						// Add the adaptor to the parameter object
						gcdo_ptr->addAdaptor(gat_ptr);

						// Add the object to the individual
						p->push_back(gcdo_ptr);
					} else {
						throw gemfony_exception(
							g_error_streamer(DO_LOG,  time_and_place)
								<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
								<< (cit->second).get<std::string>("type") << " provided as type name." << std::endl
								<< "Currently only GConstrainedDoubleObject is supported." << std::endl
						);
					}

					if (++varCounter >= nVar) {
						break; // Terminate the loop if we have identified all expected parameter objects
					}
					else {
						varString =
							std::string("var") + boost::lexical_cast<std::string>(varCounter); // Create a new var string
					}
				}
			}
		} else {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
					<< "No variables were specified" << std::endl
			);
		}

		// If boundaries have been specified, add the required boundary objects to the individual.
		boost::optional<ptree &> boundsNode_opt = m_ptr.get_child_optional("batch.individuals.individual0.bounds");
		if (boundsNode_opt) {
			// Create a check combiner -- it will hold the boundary conditions we find here
			std::shared_ptr <GCheckCombinerT<GOptimizableEntity>> combiner_ptr(new GCheckCombinerT<GOptimizableEntity>());

			// Loop over all children of the bounds tree
			// Note that for now we only query GConstrainedDoubleObject objects
			std::size_t boundsCounter = 0;
			std::string boundString = "bound0";
			for (cit = (*boundsNode_opt).begin(); cit != (*boundsNode_opt).end(); ++cit) {
				if (cit->first == boundString) {
					std::string expression = (cit->second).get<std::string>("expression");
					bool allowNegative = (cit->second).get<bool>("allowNegative");

					// The actual "function-constraint"
					std::shared_ptr <GParameterSetFormulaConstraint> formula_constraint(
						new GParameterSetFormulaConstraint(expression));

					formula_constraint->setAllowNegative(allowNegative);

					// Add the constraint to the check-combiner
					combiner_ptr->addCheck(formula_constraint);

					if (++boundsCounter >= nBounds) {
						break; // Terminate the loop if we have identified all expected boundaries
					} else {
						boundString = std::string("bound") + boost::lexical_cast<std::string>(boundsCounter);
					}
				}
			}

#ifdef DEBUG
			glogger
				<< "Found " << boundsCounter << " bounds" << std::endl
				<< GLOGGING;
#endif /* DEBUG */

			// Add the check combiner to the individual
			p->registerConstraint(combiner_ptr);
		} // It isn't an error if no boundaries were specified

		// Add the program name and base name for parameter transfers to the object
		p->setExchangeBaseName(m_parameterFileBaseName);
		p->setProgramName(m_programName);
		p->setCustomOptions(m_customOptions);
		p->setNExpectedResults(nResultsExpected);
		p->setRemoveExecTemporaries(m_removeExecTemporaries);
		p->setRunId(runID);
	} catch (const pt::ptree_bad_path &e) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
				<< "Caught ptree_bad_path exception with message " << std::endl
				<< e.what() << std::endl
		);
	} catch (const gemfony_exception &gec) {
		throw gec; // Re-throw
	} catch (...) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Caught unknown exception!" << std::endl
		);
	}
}

/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */
