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
	: GParameterSet(), programName_(GEEI_DEF_PROGNAME), customOptions_(GEEI_DEF_CUSTOMOPTIONS),
	  parameterFileBaseName_(GEEI_DEF_PARFILEBASENAME), nResults_(GEEI_DEF_NRESULTS), runID_(GEEI_DEF_RUNID),
	  removeExecTemporaries_(GEEI_DEF_REMOVETEMPORARIES) { /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual &cp)
	: GParameterSet(cp) // copies all local collections
	, programName_(cp.programName_), customOptions_(cp.customOptions_),
	  parameterFileBaseName_(cp.parameterFileBaseName_), nResults_(cp.nResults_), runID_(cp.runID_),
	  removeExecTemporaries_(cp.removeExecTemporaries_) { /* nothing */ }

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
		this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
		this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
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
	// Check that we are indeed dealing with a GBaseEA reference
	const GExternalEvaluatorIndividual *p_load = GObject::gobject_conversion<GExternalEvaluatorIndividual>(&cp);

	Gem::Common::GToken token("GExternalEvaluatorIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSet>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	Gem::Common::compare_t(IDENTITY(programName_, p_load->programName_), token);
	Gem::Common::compare_t(IDENTITY(customOptions_, p_load->customOptions_), token);
	Gem::Common::compare_t(IDENTITY(parameterFileBaseName_, p_load->parameterFileBaseName_), token);
	Gem::Common::compare_t(IDENTITY(nResults_, p_load->nResults_), token);
	Gem::Common::compare_t(IDENTITY(runID_, p_load->runID_), token);
	Gem::Common::compare_t(IDENTITY(removeExecTemporaries_, p_load->removeExecTemporaries_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividual::setProgramName(const std::string &programName) {
	programName_ = programName;
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividual::getProgramName() const {
	return programName_;
}


/******************************************************************************/
/**
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividual::setCustomOptions(const std::string &customOptions) {
	customOptions_ = customOptions;
}

/******************************************************************************/
/**
 * Retrieves the name of the external evaluation program
 */
std::string GExternalEvaluatorIndividual::getCustomOptions() const {
	return customOptions_;
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
		glogger
		<< "In GExternalEvaluatorIndividual::setExchangeBaseName(): Error!" << std::endl
		<< "Invalid file name \"" << parameterFile << "\"" << std::endl
		<< GEXCEPTION;
	}

	parameterFileBaseName_ = parameterFile;
}

/******************************************************************************/
/**
 * Retrieves the current value of the parameterFileBaseName_ variable.
 *
 * @return The current base name of the exchange file
 */
std::string GExternalEvaluatorIndividual::getExchangeBaseName() const {
	return parameterFileBaseName_;
}

/******************************************************************************/
/**
 * Sets the number of results to be expected from the external evaluation program
 */
void GExternalEvaluatorIndividual::setNExpectedResults(const std::size_t &nResults) {
	if (0 == nResults) {
		glogger
		<< "In GExternalEvaluatorIndividual::setNExpectedResults(): Error!" << std::endl
		<< "Got invalid number of expected results: " << nResults << std::endl
		<< GEXCEPTION;
	}

	nResults_ = nResults;
}

/******************************************************************************/
/**
 * Retrieves the number of results to be expected from the external evaluation program
 */
std::size_t GExternalEvaluatorIndividual::getNExpectedResults() const {
	return nResults_;
}

/******************************************************************************/
/**
 * Loads the data of another GExternalEvaluatorIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GExternalEvaluatorIndividual, camouflaged as a GObject
 */
void GExternalEvaluatorIndividual::load_(const GObject *cp) {
	// Convert to a local representation
	const GExternalEvaluatorIndividual *p_load = gobject_conversion<GExternalEvaluatorIndividual>(cp);

	// First load the data of our parent class ...
	GParameterSet::load_(cp);

	// ... and then our own
	programName_ = p_load->programName_;
	customOptions_ = p_load->customOptions_;
	parameterFileBaseName_ = p_load->parameterFileBaseName_;
	nResults_ = p_load->nResults_;
	runID_ = p_load->runID_;
	removeExecTemporaries_ = p_load->removeExecTemporaries_;
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
	std::string parameterfileName = parameterFileBaseName_ + extension + ".xml";
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
	if (customOptions_ != "empty" && !customOptions_.empty()) {
		arguments.push_back(customOptions_);
	}
	arguments.push_back(std::string("--evaluate"));
	arguments.push_back(std::string("--input=\"") + parameterfileName + "\"");
	arguments.push_back(std::string("--output=\"") + resultFileName + "\"");

	// Perform the external evaluation
	double main_result = 0.;
	std::string command;
	int errorCode = Gem::Common::runExternalCommand(
		boost::filesystem::path(programName_), arguments, boost::filesystem::path(commandOutputFileName), command
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
		for (std::size_t res = 1; res < nResults_; res++) {
			this->registerSecondaryResult(res, this->getWorstCase());
		}

		// Make sure the individual can be recognized as invalid by Geneva
		this->markAsInvalid();
	} else { // Everything is o.k., lets retrieve the evaluation
		// Check that the result file exists
		if (!bf::exists(resultFileName)) {
			glogger
			<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
			<< "Result file " << resultFileName << " does not seem to exist." << std::endl
			<< GEXCEPTION;
		}

		// Parse the results
		boost::property_tree::ptree ptr_in; // A property tree object;
		try {
			pt::read_xml(resultFileName, ptr_in);
		} catch (const boost::property_tree::xml_parser::xml_parser_error &e) {
			glogger
			<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error  " << std::endl
			<< "Caught boost::property_tree::xml_parser::xml_parser_error" << std::endl
			<< "for file " << e.filename() << " (line " << e.line() << ")" << std::endl
			<< GEXCEPTION;
		} catch (const std::exception &e) {
			glogger
			<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error reading " << resultFileName << std::endl
			<< "with message " << e.what() << std::endl
			<< GEXCEPTION;
		}

		// Check that only a single result was returned
		std::size_t nExternalIndividuals = ptr_in.get<std::size_t>(batch + ".nIndividuals");
		if (1 != nExternalIndividuals) {
			glogger
			<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
			<< "Number of result individuals != 1: " << nExternalIndividuals << std::endl
			<< GEXCEPTION;
		}

		// Check that the number of results provided by the result file matches the number of expected results
		std::size_t externalNResults = ptr_in.get<std::size_t>("batch.individuals.individual0.nResults");
		if (externalNResults != nResults_) {
			glogger
			<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
			<< "Result file provides nResults = " << externalNResults << std::endl
			<< "while we expected " << nResults_ << std::endl
			<< GEXCEPTION;
		}

		// Check that the evaluation id matches our local id
		std::string externalEvaluationID = ptr_in.get<std::string>("batch.individuals.individual0.id");
		if (externalEvaluationID != this->getCurrentEvaluationID()) {
			glogger
			<< "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
			<< "Local evaluation id " << this->getCurrentEvaluationID() << " does not match external id " <<
			externalEvaluationID << std::endl
			<< GEXCEPTION;
		}

		// Check whether the results represent useful values
		bool isValid = ptr_in.get<bool>("batch.individuals.individual0.isValid");
		if (!isValid) { // Assign worst-case values to all result
			main_result = this->getWorstCase();
			for (std::size_t res = 1; res < nResults_; res++) {
				this->registerSecondaryResult(res, this->getWorstCase());
			}

			// Make sure the individual can be recognized as invalid by Geneva
			this->markAsInvalid();
		} else { // Extract and store all result values
			// Get the results node
			pt::ptree resultsNode = ptr_in.get_child("batch.individuals.individual0.results");

			double currentResult = 0.;
			std::string resultString;
			for (std::size_t res = 0; res < nResults_; res++) {
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
	if (removeExecTemporaries_) {
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
		glogger
		<< "In GExternalEvaluatorIndividual::setRunId(): Error!" << std::endl
		<< "Attempt to set an invalid run id: \"" << runID << "\"" << std::endl
		<< GEXCEPTION;
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
	removeExecTemporaries_ = removeExecTemporaries;
}

/******************************************************************************/
/**
 * Allows to check whether temporaries should be removed
 */
bool GExternalEvaluatorIndividual::getRemoveExecTemporaries() const {
	return removeExecTemporaries_;
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
	: Gem::Common::GFactoryT<GParameterSet>(configFile), adProb_(GEEI_DEF_ADPROB), adaptAdProb_(GEEI_DEF_ADAPTADPROB),
	  minAdProb_(GEEI_DEF_MINADPROB), maxAdProb_(GEEI_DEF_MAXADPROB), adaptionThreshold_(GEEI_DEF_ADAPTIONTHRESHOLD),
	  useBiGaussian_(GEEI_DEF_USEBIGAUSSIAN), sigma1_(GEEI_DEF_SIGMA1), sigmaSigma1_(GEEI_DEF_SIGMASIGMA1),
	  minSigma1_(GEEI_DEF_MINSIGMA1), maxSigma1_(GEEI_DEF_MAXSIGMA1), sigma2_(GEEI_DEF_SIGMA2),
	  sigmaSigma2_(GEEI_DEF_SIGMASIGMA2), minSigma2_(GEEI_DEF_MINSIGMA2), maxSigma2_(GEEI_DEF_MAXSIGMA2),
	  delta_(GEEI_DEF_DELTA), sigmaDelta_(GEEI_DEF_SIGMADELTA), minDelta_(GEEI_DEF_MINDELTA),
	  maxDelta_(GEEI_DEF_MAXDELTA), programName_(GEEI_DEF_PROGNAME), customOptions_(GEEI_DEF_CUSTOMOPTIONS),
	  parameterFileBaseName_(GEEI_DEF_PARFILEBASENAME), initValues_(GEEI_DEF_STARTMODE),
	  removeExecTemporaries_(GEEI_DEF_REMOVETEMPORARIES), externalEvaluatorQueried_(false) { /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory(
	const GExternalEvaluatorIndividualFactory &cp
)
	: Gem::Common::GFactoryT<GParameterSet>(cp), adProb_(cp.adProb_), adaptAdProb_(cp.adaptAdProb_),
	  minAdProb_(cp.minAdProb_), maxAdProb_(cp.maxAdProb_), adaptionThreshold_(cp.adaptionThreshold_),
	  useBiGaussian_(cp.useBiGaussian_), sigma1_(cp.sigma1_), sigmaSigma1_(cp.sigmaSigma1_), minSigma1_(cp.minSigma1_),
	  maxSigma1_(cp.maxSigma1_), sigma2_(cp.sigma2_), sigmaSigma2_(cp.sigmaSigma2_), minSigma2_(cp.minSigma2_),
	  maxSigma2_(cp.maxSigma2_), delta_(cp.delta_), sigmaDelta_(cp.sigmaDelta_), minDelta_(cp.minDelta_),
	  maxDelta_(cp.maxDelta_), programName_(cp.programName_), customOptions_(cp.customOptions_),
	  parameterFileBaseName_(cp.parameterFileBaseName_), initValues_(cp.initValues_),
	  removeExecTemporaries_(cp.removeExecTemporaries_), externalEvaluatorQueried_(cp.externalEvaluatorQueried_),
	  ptr_(cp.ptr_) { /* nothing */ }

/******************************************************************************/
/**
 * The default constructor. Only needed for (de-)serialization purposes, hence empty.
 */
GExternalEvaluatorIndividualFactory::GExternalEvaluatorIndividualFactory()
	: Gem::Common::GFactoryT<GParameterSet>("empty"), adProb_(GEEI_DEF_ADPROB), adaptAdProb_(GEEI_DEF_ADAPTADPROB),
	  minAdProb_(GEEI_DEF_MINADPROB), maxAdProb_(GEEI_DEF_MAXADPROB), adaptionThreshold_(GEEI_DEF_ADAPTIONTHRESHOLD),
	  useBiGaussian_(GEEI_DEF_USEBIGAUSSIAN), sigma1_(GEEI_DEF_SIGMA1), sigmaSigma1_(GEEI_DEF_SIGMASIGMA1),
	  minSigma1_(GEEI_DEF_MINSIGMA1), maxSigma1_(GEEI_DEF_MAXSIGMA1), sigma2_(GEEI_DEF_SIGMA2),
	  sigmaSigma2_(GEEI_DEF_SIGMASIGMA2), minSigma2_(GEEI_DEF_MINSIGMA2), maxSigma2_(GEEI_DEF_MAXSIGMA2),
	  delta_(GEEI_DEF_DELTA), sigmaDelta_(GEEI_DEF_SIGMADELTA), minDelta_(GEEI_DEF_MINDELTA),
	  maxDelta_(GEEI_DEF_MAXDELTA), programName_(GEEI_DEF_PROGNAME), customOptions_(GEEI_DEF_CUSTOMOPTIONS),
	  parameterFileBaseName_(GEEI_DEF_PARFILEBASENAME), initValues_(GEEI_DEF_STARTMODE),
	  removeExecTemporaries_(GEEI_DEF_REMOVETEMPORARIES), externalEvaluatorQueried_(false) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor. Note that, if the external evaluator, when called with the
 * --finalize switch, does anything making optimization impossible, the factory
 * should not be destroyed before the end of the optimization run.
 */
GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory() {
	// Check that the file name isn't empty
	if (programName_.value().empty()) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): Error!" << std::endl
		<< "Program name was empty" << std::endl
		<< GEXCEPTION;
	}

	// Check that the file exists
	if (!bf::exists(programName_.value())) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): Error!" << std::endl
		<< "External program " << programName_.value() << " does not seem to exist" << std::endl
		<< GTERMINATION;
	}

	// Collect all command-line arguments
	std::vector<std::string> arguments;
	if (customOptions_.value() != "empty" && !customOptions_.value().empty()) {
		arguments.push_back(customOptions_.value());
	}
	arguments.push_back(std::string("--finalize"));

	// Ask the external evaluation program to perform any final work
	std::string command;
	int errorCode = Gem::Common::runExternalCommand(
		boost::filesystem::path(programName_.value()), arguments, boost::filesystem::path(), command
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
void GExternalEvaluatorIndividualFactory::load(std::shared_ptr < Gem::Common::GFactoryT<GParameterSet> > cp_raw_ptr) {
	// Load our parent class'es data
	Gem::Common::GFactoryT<GParameterSet>::load(cp_raw_ptr);

	// Convert the base pointer
	std::shared_ptr <GExternalEvaluatorIndividualFactory> cp_ptr
		= Gem::Common::convertSmartPointer<Gem::Common::GFactoryT<GParameterSet>, GExternalEvaluatorIndividualFactory>(
			cp_raw_ptr);

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
std::shared_ptr <Gem::Common::GFactoryT<GParameterSet>> GExternalEvaluatorIndividualFactory::clone() const {
	return std::shared_ptr<GExternalEvaluatorIndividualFactory>(new GExternalEvaluatorIndividualFactory(*this));
}

/******************************************************************************/
/**
 * Get the value of the adaptionThreshold_ variable
 */
boost::uint32_t GExternalEvaluatorIndividualFactory::getAdaptionThreshold() const {
	return adaptionThreshold_;
}

/******************************************************************************/
/**
 * Set the value of the adaptionThreshold_ variable
 */
void GExternalEvaluatorIndividualFactory::setAdaptionThreshold(
	boost::uint32_t adaptionThreshold) {
	adaptionThreshold_ = adaptionThreshold;
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
void GExternalEvaluatorIndividualFactory::setAdProb(double adProb) {
	adProb_ = adProb;
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
void GExternalEvaluatorIndividualFactory::setAdaptAdProb(double adaptAdProb) {
#ifdef DEBUG
      if(adaptAdProb < 0.) {
         glogger
         << "In GExternalEvaluatorIndividualFactory::setAdaptAdProb(): Error!" << std::endl
         << "Invalid value for adaptAdProb given: " << adaptAdProb << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

	adaptAdProb_ = adaptAdProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed range for adProb_ variation
 */
boost::tuple<double, double> GExternalEvaluatorIndividualFactory::getAdProbRange() const {
	return boost::tuple<double, double>(minAdProb_.value(), maxAdProb_.value());
}

/******************************************************************************/
/**
 * Allows to set the allowed range for adaption probability variation
 */
void GExternalEvaluatorIndividualFactory::setAdProbRange(double minAdProb, double maxAdProb) {
#ifdef DEBUG
      if(minAdProb < 0.) {
         glogger
         << "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << std::endl
         << "minAdProb < 0: " << minAdProb << std::endl
         << GEXCEPTION;
      }

      if(minAdProb > maxAdProb) {
         glogger
         << "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << std::endl
         << "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb << std::endl
         << GEXCEPTION;
      }

      if(maxAdProb > 1.) {
         glogger
         << "In GExternalEvaluatorIndividualFactory::setAdProbRange(): Error!" << std::endl
         << "maxAdProb > 1: " << maxAdProb << std::endl
         << GEXCEPTION;
      }
#endif /* DEBUG */

	minAdProb_ = minAdProb;
	maxAdProb_ = maxAdProb;
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
void GExternalEvaluatorIndividualFactory::setMaxDelta(double maxDelta) {
	maxDelta_ = maxDelta;
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
void GExternalEvaluatorIndividualFactory::setMaxSigma1(double maxSigma1) {
	maxSigma1_ = maxSigma1;
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
void GExternalEvaluatorIndividualFactory::setMaxSigma2(double maxSigma2) {
	maxSigma2_ = maxSigma2;
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
void GExternalEvaluatorIndividualFactory::setMinDelta(double minDelta) {
	minDelta_ = minDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of delta
 */
boost::tuple<double, double> GExternalEvaluatorIndividualFactory::getDeltaRange() const {
	return boost::tuple<double, double>(minDelta_, maxDelta_);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of delta
 */
void GExternalEvaluatorIndividualFactory::setDeltaRange(boost::tuple<double, double> range) {
	double min = boost::get<0>(range);
	double max = boost::get<1>(range);

	if (min < 0) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setDeltaRange(): Error" << std::endl
		<< "min must be >= 0. Got : " << max << std::endl
		<< GEXCEPTION;
	}

	if (min >= max) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setDeltaRange(): Error" << std::endl
		<< "Invalid range specified: " << min << " / " << max << std::endl
		<< GEXCEPTION;
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
void GExternalEvaluatorIndividualFactory::setMinSigma1(double minSigma1) {
	minSigma1_ = minSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma1_
 */
boost::tuple<double, double> GExternalEvaluatorIndividualFactory::getSigma1Range() const {
	return boost::tuple<double, double>(minSigma1_, maxSigma1_);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma1_
 */
void GExternalEvaluatorIndividualFactory::setSigma1Range(boost::tuple<double, double> range) {
	double min = boost::get<0>(range);
	double max = boost::get<1>(range);

	if (min < 0) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setSigma1Range(): Error" << std::endl
		<< "min must be >= 0. Got : " << max << std::endl
		<< GEXCEPTION;
	}

	if (min >= max) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setSigma1Range(): Error" << std::endl
		<< "Invalid range specified: " << min << " / " << max << std::endl
		<< GEXCEPTION;
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
void GExternalEvaluatorIndividualFactory::setMinSigma2(double minSigma2) {
	minSigma2_ = minSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the allowed value range of sigma2_
 */
boost::tuple<double, double> GExternalEvaluatorIndividualFactory::getSigma2Range() const {
	return boost::tuple<double, double>(minSigma2_, maxSigma2_);
}

/******************************************************************************/
/**
 * Allows to set the allowed value range of sigma2_
 */
void GExternalEvaluatorIndividualFactory::setSigma2Range(boost::tuple<double, double> range) {
	double min = boost::get<0>(range);
	double max = boost::get<1>(range);

	if (min < 0) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setSigma2Range(): Error" << std::endl
		<< "min must be >= 0. Got : " << max << std::endl
		<< GEXCEPTION;
	}

	if (min >= max) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setSigma2Range(): Error" << std::endl
		<< "Invalid range specified: " << min << " / " << max << std::endl
		<< GEXCEPTION;
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
void GExternalEvaluatorIndividualFactory::setSigmaDelta(double sigmaDelta) {
	sigmaDelta_ = sigmaDelta;
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
void GExternalEvaluatorIndividualFactory::setSigmaSigma1(double sigmaSigma1) {
	sigmaSigma1_ = sigmaSigma1;
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
void GExternalEvaluatorIndividualFactory::setSigmaSigma2(double sigmaSigma2) {
	sigmaSigma2_ = sigmaSigma2;
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
void GExternalEvaluatorIndividualFactory::setUseBiGaussian(bool useBiGaussian) {
	useBiGaussian_ = useBiGaussian;
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
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << std::endl
		<< "File name was empty" << std::endl
		<< GEXCEPTION;
	}

	// Check that the file exists
	if (!bf::exists(programName)) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << std::endl
		<< "External program " << programName << " does not seem to exist" << std::endl
		<< GEXCEPTION;
	}

	programName_.setValue(programName);
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
void GExternalEvaluatorIndividualFactory::setCustomOptions(std::string customOptions) {
	customOptions_.setValue(customOptions);
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
void GExternalEvaluatorIndividualFactory::setParameterFileBaseName(std::string parameterFileBaseName) {
	// Check that the name isn't empty
	if (parameterFileBaseName.empty()) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setParameterFileBaseName(): Error!" << std::endl
		<< "Name was empty" << std::endl
		<< GEXCEPTION;
	}

	parameterFileBaseName_ = parameterFileBaseName;
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
 */
void GExternalEvaluatorIndividualFactory::setInitValues(std::string initValues) {
	if (initValues != "random" && initValues != "min" && initValues != "max" && initValues != "predef") {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setInitValues(): Error!" << std::endl
		<< "Invalid argument: " << initValues << std::endl
		<< "Expected \"random\", \"min\", \"max\" or \"predef\"." << std::endl
		<< GEXCEPTION;
	}

	initValues_.setValue(initValues);
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
void GExternalEvaluatorIndividualFactory::setRemoveExecTemporaries(bool removeExecTemporaries) {
	removeExecTemporaries_.setValue(removeExecTemporaries);
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
	const std::vector<std::shared_ptr < GExternalEvaluatorIndividual>

>& arch
) const {
// Check that there are individuals contained in the archive
if(arch.

empty()

) return; // Do nothing

// Transform the objects into a batch of boost property tree
boost::property_tree::ptree ptr_out;
std::string batch = "batch";

// Output the header data
ptr_out.
put(batch
+ ".dataType", std::string("archive_data"));
ptr_out.
put(batch
+ ".runID", arch.

front() -> getRunId()

);
ptr_out.
put(batch
+ ".nIndividuals", arch.

size()

);

// Output the individuals in turn
std::vector<std::shared_ptr < GExternalEvaluatorIndividual> >
::const_iterator cit;
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
const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
boost::gregorian::date d = currentTime.date();
std::string extension = std::string("-")
								+ boost::lexical_cast<std::string>(d.year()) + "-"
								+ boost::lexical_cast<std::string>(d.month()) + "-"
								+ boost::lexical_cast<std::string>(d.day()) + "-"
								+ boost::lexical_cast<std::string>(currentTime.time_of_day().hours()) + "-"
								+ boost::lexical_cast<std::string>(currentTime.time_of_day().minutes()) + "-"
								+ boost::lexical_cast<std::string>(currentTime.time_of_day().seconds()) + "-"
								+ boost::lexical_cast<std::string>(boost::uuids::random_generator()()) + ".xml";
std::string parameterfileName = parameterFileBaseName_.value() + extension;

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
if(customOptions_.

value()

!= "empty" && !customOptions_.

value()

.

empty()

) {
arguments.
push_back(customOptions_
.

value()

);
}
arguments.
push_back(std::string("--archive"));
arguments.
push_back(std::string("--input=\"" + parameterfileName + "\""));

// Ask the external evaluation program to perform any final work
std::string command;
int errorCode = Gem::Common::runExternalCommand(
	boost::filesystem::path(programName_.value()), arguments, boost::filesystem::path(), command
);

// Let the audience know
if(errorCode) {
glogger
<< "In GExternalEvaluatorIndividualFactory::archive(): Error" << std::endl
<< "Execution of external command failed." << std::endl
<< "Command: " << command << std::endl
<< "Error code: " << errorCode << std::endl
<<
GEXCEPTION;
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
	Gem::Common::GParserBuilder &gpb, const std::size_t &id
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
		"adProb", adProb_.reference(), GEEI_DEF_ADPROB
	)
	<< "The probability for random adaption of values in evolutionary algorithms";

	gpb.registerFileParameter<double>(
		"adaptAdProb", adaptAdProb_.reference(), GEEI_DEF_ADAPTADPROB
	)
	<< "Determines the rate of adaption of adProb. Set to 0, if you do not need this feature";

	gpb.registerFileParameter<double>(
		"minAdProb", minAdProb_.reference(), GEEI_DEF_MINADPROB
	)
	<< "The lower allowed boundary for adProb-variation";

	gpb.registerFileParameter<double>(
		"maxAdProb", maxAdProb_.reference(), GEEI_DEF_MAXADPROB
	)
	<< "The upper allowed boundary for adProb-variation";


	gpb.registerFileParameter<boost::uint32_t>(
		"adaptionThreshold", adaptionThreshold_.reference(), GEEI_DEF_ADAPTIONTHRESHOLD
	)
	<< "The number of calls to an adaptor after which adaption takes place";

	gpb.registerFileParameter<bool>(
		"useBiGaussian", useBiGaussian_.reference(), GEEI_DEF_USEBIGAUSSIAN
	)
	<< "Whether to use a double gaussion for the adaption of parmeters in ES";

	gpb.registerFileParameter<double>(
		"sigma1", sigma1_.reference(), GEEI_DEF_SIGMA1
	)
	<< "The sigma for gauss-adaption in ES" << std::endl
	<< "(or the sigma of the left peak of a double gaussian)";

	gpb.registerFileParameter<double>(
		"sigmaSigma1", sigmaSigma1_.reference(), GEEI_DEF_SIGMASIGMA1
	)
	<< "Influences the self-adaption of gauss-mutation in ES";

	gpb.registerFileParameter<double>(
		"minSigma1", minSigma1_.reference(), GEEI_DEF_MINSIGMA1
	)
	<< "The minimum value of sigma1";

	gpb.registerFileParameter<double>(
		"maxSigma1", maxSigma1_.reference(), GEEI_DEF_MAXSIGMA1
	)
	<< "The maximum value of sigma1";

	gpb.registerFileParameter<double>(
		"sigma2", sigma2_.reference(), GEEI_DEF_SIGMA2
	)
	<< "The sigma of the right peak of a double gaussian (if any)";

	gpb.registerFileParameter<double>(
		"sigmaSigma2", sigmaSigma2_.reference(), GEEI_DEF_SIGMASIGMA2
	)
	<< "Influences the self-adaption of gauss-mutation in ES";

	gpb.registerFileParameter<double>(
		"minSigma2", minSigma2_.reference(), GEEI_DEF_MINSIGMA2
	)
	<< "The minimum value of sigma2";

	gpb.registerFileParameter<double>(
		"maxSigma2", maxSigma2_.reference(), GEEI_DEF_MAXSIGMA2
	)
	<< "The maximum value of sigma2";

	gpb.registerFileParameter<double>(
		"delta", delta_.reference(), GEEI_DEF_DELTA
	)
	<< "The start distance between both peaks used for bi-gaussian mutations in ES";

	gpb.registerFileParameter<double>(
		"sigmaDelta", sigmaDelta_.reference(), GEEI_DEF_SIGMADELTA
	)
	<< "The width of the gaussian used for mutations of the delta parameter";

	gpb.registerFileParameter<double>(
		"minDelta", minDelta_.reference(), GEEI_DEF_MINDELTA
	)
	<< "The minimum allowed value of delta";

	gpb.registerFileParameter<double>(
		"maxDelta", maxDelta_.reference(), GEEI_DEF_MAXDELTA
	)
	<< "The maximum allowed value of delta";

	gpb.registerFileParameter<std::string>(
		"programName", programName_.reference() // Upon repeated filling this option will do nothing
		, GEEI_DEF_PROGNAME
	)
	<< "The name of the external evaluation program";

	gpb.registerFileParameter<std::string>(
		"customOptions", customOptions_.reference(), GEEI_DEF_CUSTOMOPTIONS
	)
	<< "Any custom options you wish to pass to the external evaluator";

	gpb.registerFileParameter<std::string>(
		"parameterFile", parameterFileBaseName_.reference(), GEEI_DEF_PARFILEBASENAME
	)
	<< "The base name assigned to parameter files" << std::endl
	<< "in addition to data identifying this specific evaluation";

	gpb.registerFileParameter<std::string>(
		"initValues", initValues_.reference(), GEEI_DEF_STARTMODE
	)
	<< "Indicates, whether individuals should be initialized randomly (random)," << std::endl
	<< "with the lower (min) or upper (max) boundary of their value ranges";

	gpb.registerFileParameter<bool>(
		"removeExecTemporaries", removeExecTemporaries_.reference(), GEEI_DEF_REMOVETEMPORARIES
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
	if (externalEvaluatorQueried_) return;
	else externalEvaluatorQueried_ = true;

	// Check that the file name isn't empty
	if (programName_.value().empty()) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << std::endl
		<< "File name was empty" << std::endl
		<< GEXCEPTION;
	}

	// Check that the file exists
	if (!bf::exists(programName_.value())) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << std::endl
		<< "External program " << programName_.value() << " does not seem to exist" << std::endl
		<< GEXCEPTION;
	}

	// Make sure the property tree is empty
	ptr_.clear();

	{ // First we give the external program the opportunity to perform an initial work
		// Collect all command-line arguments
		std::vector<std::string> arguments;
		if (customOptions_.value() != "empty" && !customOptions_.value().empty()) {
			arguments.push_back(customOptions_.value());
		}
		arguments.push_back(std::string("--init"));

		// Ask the external evaluation program to perform any initial work
		std::string command;
		int errorCode = Gem::Common::runExternalCommand(
			boost::filesystem::path(programName_.value()), arguments, boost::filesystem::path(), command
		);

		if (errorCode) {
			glogger
			<< "In GExternalEvaluatorIndividual::setUpPropertyTree(//1//): Error" << std::endl
			<< "Execution of external command failed." << std::endl
			<< "Command: " << command << std::endl
			<< "Error code: " << errorCode << std::endl
			<< GEXCEPTION;
		}
	}

	{ // Now we ask the external program for setup-iformation
		// Collect all command-line arguments
		std::vector<std::string> arguments;
		if (customOptions_.value() != "empty" && !customOptions_.value().empty()) {
			arguments.push_back(customOptions_.value());
		}

		// "/" will be converted to "\" in runExternalCommand, if necessary
		std::string setupFileName =
			std::string("./setup-") + boost::lexical_cast<std::string>(this) + std::string(".xml");
		arguments.push_back("--setup");
		arguments.push_back("--output=\"" + setupFileName + "\"");
		arguments.push_back("--initvalues=\"" + initValues_.value() + "\"");

		// Ask the external evaluation program tfor setup information
		std::string command;
		int errorCode = Gem::Common::runExternalCommand(
			boost::filesystem::path(programName_.value()), arguments, boost::filesystem::path(), command
		);

		if (errorCode) {
			glogger
			<< "In GExternalEvaluatorIndividual::setUpPropertyTree(//2//): Error" << std::endl
			<< "Execution of external command failed." << std::endl
			<< "Command: " << command << std::endl
			<< "Error code: " << errorCode << std::endl
			<< GEXCEPTION;
		}

		// Parse the setup file
		pt::read_xml(setupFileName, ptr_);

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

	if (ptr_.empty()) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
		<< "Property tree is empty." << std::endl
		<< GEXCEPTION;
	}

	// Set up an adaptor for the collection, so they know how to be adapted
	std::shared_ptr <GAdaptorT<double>> gat_ptr;
	if (useBiGaussian_) {
		std::shared_ptr <GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());
		gdbga_ptr->setAllSigma1(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_);
		gdbga_ptr->setAllSigma2(sigma2_, sigmaSigma2_, minSigma2_, maxSigma2_);
		gdbga_ptr->setAllDelta(delta_, sigmaDelta_, minDelta_, maxDelta_);
		gdbga_ptr->setAdaptionThreshold(adaptionThreshold_);
		gdbga_ptr->setAdaptionProbability(adProb_);
		gat_ptr = gdbga_ptr;
	} else {
		std::shared_ptr <GDoubleGaussAdaptor> gdga_ptr(
			new GDoubleGaussAdaptor(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_));
		gdga_ptr->setAdaptionThreshold(adaptionThreshold_);
		gdga_ptr->setAdaptionProbability(adProb_);
		gat_ptr = gdga_ptr;
	}

	// Store parameters pertaining to the adaption probability in the adaptor
	gat_ptr->setAdaptAdProb(adaptAdProb_);
	gat_ptr->setAdProbRange(minAdProb_, maxAdProb_);

	try {
		// Extract the number of individuals
		std::size_t nIndividuals = ptr_.get<std::size_t>("batch.nIndividuals");
		if (1 != nIndividuals) {
			glogger
			<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
			<< "Received invalid number of setup-individuals: " << nIndividuals << std::endl
			<< GEXCEPTION;
		}

		// Get the run-id
		std::string runID = ptr_.get<std::string>("batch.runID");

		// Extract the number of variables for the first individual
		std::size_t nVar = ptr_.get<std::size_t>("batch.individuals.individual0.nVars");

		// Extract the number of results to be expected from the external evaluation function for the first individual
		std::size_t nResultsExpected = ptr_.get<std::size_t>("batch.individuals.individual0.nResults");

		// Extract the number of boundaries to be expected for the first individual
		std::size_t nBounds = ptr_.get<std::size_t>("batch.individuals.individual0.nBounds");

		// Get an iterator over a property tree
		ptree::const_iterator cit;

		// If variables have been specified, extract them
		boost::optional<ptree &> varSetNode_opt = ptr_.get_child_optional("batch.individuals.individual0.vars");
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
						glogger
						<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
						<< (cit->second).get<std::string>("type") << " provided as type name." << std::endl
						<< "Currently only GConstrainedDoubleObject is supported." << std::endl
						<< GEXCEPTION;
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
			glogger
			<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
			<< "No variables were specified" << std::endl
			<< GEXCEPTION;
		}

		// If boundaries have been specified, add the required boundary objects to the individual.
		boost::optional<ptree &> boundsNode_opt = ptr_.get_child_optional("batch.individuals.individual0.bounds");
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
		p->setExchangeBaseName(parameterFileBaseName_);
		p->setProgramName(programName_);
		p->setCustomOptions(customOptions_);
		p->setNExpectedResults(nResultsExpected);
		p->setRemoveExecTemporaries(removeExecTemporaries_);
		p->setRunId(runID);
	} catch (const pt::ptree_bad_path &e) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
		<< "Caught ptree_bad_path exception with message " << std::endl
		<< e.what() << std::endl
		<< GEXCEPTION;
	} catch (const Gem::Common::gemfony_error_condition &gec) {
		throw gec; // Re-throw
	} catch (...) {
		glogger
		<< "In GExternalEvaluatorIndividualFactory::postProcess_(): Caught unknown exception!" << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */
