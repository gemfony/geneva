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

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual()
: programName_(GEEI_DEF_PROGNAME)
, customOptions_(GEEI_DEF_CUSTOMOPTIONS)
, parameterFileBaseName_(GEEI_DEF_PARFILEBASENAME)
, nResults_(GEEI_DEF_NRESULTS)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor.
 */
GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual& cp)
: GParameterSet(cp) // copies all local collections
, programName_(cp.programName_)
, customOptions_(cp.customOptions_)
, parameterFileBaseName_(cp.parameterFileBaseName_)
, nResults_(cp.nResults_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GExternalEvaluatorIndividual::~GExternalEvaluatorIndividual()
{ /* nothing */   }

/******************************************************************************/
/**
 * A standard assignment operator
 */
const GExternalEvaluatorIndividual& GExternalEvaluatorIndividual::operator=(const GExternalEvaluatorIndividual& cp){
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
bool GExternalEvaluatorIndividual::operator==(const GExternalEvaluatorIndividual& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GExternalEvaluatorIndividual::operator==","cp", CE_SILENT);
}

/******************************************************************************/
/**
 * Checks for inequality with another GExternalEvaluatorIndividual object
 *
 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
 * @return A boolean indicating whether both objects are inequal
 */
bool GExternalEvaluatorIndividual::operator!=(const GExternalEvaluatorIndividual& cp) const {
   using namespace Gem::Common;
   // Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
   return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GExternalEvaluatorIndividual::operator!=","cp", CE_SILENT);
}

/******************************************************************************/
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
boost::optional<std::string> GExternalEvaluatorIndividual::checkRelationshipWith(
      const GObject& cp,
      const Gem::Common::expectation& e,
      const double& limit,
      const std::string& caller,
      const std::string& y_name,
      const bool& withMessages) const
{
   using namespace Gem::Common;

   // Check that we are indeed dealing with a GParamterBase reference
   const GExternalEvaluatorIndividual *p_load = gobject_conversion<GExternalEvaluatorIndividual>(&cp);

   // Will hold possible deviations from the expectation, including explanations
   std::vector<boost::optional<std::string> > deviations;

   // Check our parent class'es data ...
   deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GExternalEvaluatorIndividual", y_name, withMessages));

   // ... and then our local data
   deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", programName_, p_load->programName_, "programName_", "p_load->programName_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", customOptions_, p_load->customOptions_, "customOptions_", "p_load->customOptions_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", parameterFileBaseName_, p_load->parameterFileBaseName_, "parameterFileBaseName_", "p_load->parameterFileBaseName_", e , limit));
   deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", nResults_, p_load->nResults_, "nResults_", "p_load->nResults_", e , limit));

   return evaluateDiscrepancies("GExternalEvaluatorIndividual", caller, deviations, e);
}


/******************************************************************************/
/**
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividual::setProgramName(const std::string& programName) {
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
void GExternalEvaluatorIndividual::setCustomOptions(const std::string& customOptions) {
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
void GExternalEvaluatorIndividual::setExchangeFileName(const std::string& parameterFile) {
   if(parameterFile.empty() || parameterFile == "empty") {
      std::ostringstream error;
      error << "In GExternalEvaluatorIndividual::setExchangeFileName(): Error!" << std::endl
            << "Invalid file name \"" << parameterFile << "\"" << std::endl;
      throw Gem::Common::gemfony_error_condition(error.str());
   }

   parameterFileBaseName_ = parameterFile;
}

/******************************************************************************/
/**
 * Retrieves the current value of the parameterFileBaseName_ variable.
 *
 * @return The current base name of the exchange file
 */
std::string GExternalEvaluatorIndividual::getExchangeFileName() const {
   return parameterFileBaseName_;
}

/******************************************************************************/
/**
 * Sets the number of results to be expected from the external evaluation program
 */
void GExternalEvaluatorIndividual::setNExpectedResults(const std::size_t& nResults) {
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
/** @brief Triggers archiving of the evaluation results */
void GExternalEvaluatorIndividual::archive() const {
   // Transform this object into a boost property tree
   pt::ptree ptr_out;
   this->toPropertyTree(ptr_out);

   // Create a suitable extension and exchange file names for this object
   std::string extension = std::string("-") + boost::lexical_cast<std::string>(this->getAssignedIteration()) + "-" + boost::lexical_cast<std::string>(this) + ".xml";
   std::string parameterfileName = parameterFileBaseName_ + extension;

   // Save the parameters to a file for the external evaluation
#if BOOST_VERSION > 105500
      boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#else
      boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#endif /* BOOST_VERSION */
   pt::write_xml(parameterfileName, ptr_out, std::locale(), settings);

   // Create full command string for the external evaluation
   std::string command;
   if(customOptions_ != "empty" && !customOptions_.empty()) {
      command = programName_ + " " + customOptions_ + " --evaluate=\"" +  parameterfileName + "\" --archive";
   } else {
      command = programName_ + " --evaluate=\"" +  parameterfileName + "\" --archive";
   }

   // Perform the external evaluation
   Gem::Common::runExternalCommand(command);

   // Clean up (remove) the parameter file
   bf::remove(parameterfileName);
}

/******************************************************************************/
/**
 * Loads the data of another GExternalEvaluatorIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GExternalEvaluatorIndividual, camouflaged as a GObject
 */
void GExternalEvaluatorIndividual::load_(const GObject* cp){
   // Convert to a local representation
   const GExternalEvaluatorIndividual *p_load = gobject_conversion<GExternalEvaluatorIndividual>(cp);

   // First load the data of our parent class ...
   GParameterSet::load_(cp);

   // ... and then our own
   programName_ = p_load->programName_;
   customOptions_ = p_load->customOptions_;
   parameterFileBaseName_ = p_load->parameterFileBaseName_;
   nResults_ = p_load->nResults_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GExternalEvaluatorIndividual::clone_() const{
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
   pt::ptree ptr_out;
   this->toPropertyTree(ptr_out);

   // Create a suitable extension and exchange file names for this object
   std::string extension = std::string("-") + boost::lexical_cast<std::string>(this->getAssignedIteration()) + "-" + boost::lexical_cast<std::string>(this) + ".xml";
   std::string parameterfileName = parameterFileBaseName_ + extension;
   std::string resultFileName = std::string("result") + extension;

   // Save the parameters to a file for the external evaluation
#if BOOST_VERSION > 105500
      boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
#else
      boost::property_tree::xml_writer_settings<char> settings('\t', 1);
#endif /* BOOST_VERSION */
   pt::write_xml(parameterfileName, ptr_out, std::locale(), settings);

   // Create full command string for the external evaluation
   std::string command;

   if(customOptions_ != "empty" && !customOptions_.empty()) {
      command = programName_ + " " + customOptions_ + " --evaluate=\"" +  parameterfileName + "\" --result=\"" + resultFileName + "\"";
   } else {
      command = programName_ + " --evaluate=\"" +  parameterfileName + "\" --result=\"" + resultFileName + "\"";
   }

   // Perform the external evaluation
   Gem::Common::runExternalCommand(command);

   // Check that the result file exists
   if(!bf::exists(resultFileName)) {
      glogger
      << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
      << "Result file " << resultFileName << " does not seem to exist." << std::endl
      << GEXCEPTION;
   }

   // Parse the results
   pt::ptree ptr_in; // A property tree object;
   pt::read_xml(resultFileName, ptr_in);

   // Check that the number of results provided by the result file
   // matches the number of expected results
   std::size_t externalNResults = ptr_in.get<std::size_t>("results.nresult");
   if(externalNResults != nResults_) {
      glogger
      << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
      << "Result file provides nResults = " << externalNResults << std::endl
      << " while we expected " << nResults_ << std::endl
      << GEXCEPTION;
   }

   // Check whether the results represent useful values
   double main_result = 0.;
   bool isUseful = ptr_in.get<bool>("results.isUseful");
   if(!isUseful) { // Assign worst-case values to all result
      main_result = this->getWorstCase();
      for(std::size_t res=1; res<nResults_; res++) {
         this->registerSecondaryResult(res, this->getWorstCase());
      }
   } else { // Extract and store all result values
      // Get the results node
      pt::ptree resultsNode = ptr_in.get_child("results");

      double currentResult = 0.;
      std::size_t resultCounter = 0;
      std::string resultString;
      pt::ptree::const_iterator cit;
      for (std::size_t res=0; res<nResults_; res++) {
         resultString = std::string("result") + boost::lexical_cast<std::string>(res);

         currentResult = resultsNode.get<double>(resultString);

         if(res == 0) main_result = currentResult;
         else registerSecondaryResult(res, currentResult);
      }
   }

   // Clean up (remove) parameter and result files
   bf::remove(parameterfileName);
   bf::remove(resultFileName);

   // Return the master result (first result returned)

   return main_result;
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
      const std::string& configFile
)
   : Gem::Common::GFactoryT<GParameterSet>(configFile)
   , adProb_(GEEI_DEF_ADPROB)
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
   , externalEvaluatorQueried_(false)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor. Note that, if the external evaluator, when called with the
 * --finalize switch, does anything making optimization impossible, the factory
 * should not be destroyed before the end of the optimization run.
 */
GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory()
{
   // Check that the file name isn't empty
   if(programName_.value().empty()) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): Error!" << std::endl
      << "Program name was empty" << std::endl
      << GEXCEPTION;
   }

   // Check that the file exists
   if(!bf::exists(programName_.value())) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::~GExternalEvaluatorIndividualFactory(): Error!" << std::endl
      << "External program " << programName_.value() << " does not seem to exist" << std::endl
      << GEXCEPTION;
   }

   std::string programCommand;
   if(customOptions_.value() == "empty" || customOptions_.value().empty()) {
      programCommand = programName_.value();
   } else {
      programCommand = programName_.value() + " " + customOptions_.value();
   }

   // Ask the external evaluation program to perform any initial work
   Gem::Common::runExternalCommand(programCommand + " --finalize");
}

/******************************************************************************/
/**
 * Get the value of the adaptionThreshold_ variable
 */
boost::uint32_t GExternalEvaluatorIndividualFactory::getAdaptionThreshold() const
{
   return adaptionThreshold_;
}

/******************************************************************************/
/**
 * Set the value of the adaptionThreshold_ variable
 */
void GExternalEvaluatorIndividualFactory::setAdaptionThreshold(
      boost::uint32_t adaptionThreshold)
{
   adaptionThreshold_ = adaptionThreshold;
}

/******************************************************************************/
/**
 * Allows to retrieve the adProb_ variable
 */
double GExternalEvaluatorIndividualFactory::getAdProb() const
{
   return adProb_;
}

/******************************************************************************/
/**
 * Set the value of the adProb_ variable
 */
void GExternalEvaluatorIndividualFactory::setAdProb(double adProb)
{
   adProb_ = adProb;
}

/******************************************************************************/
/**
 * Allows to retrieve the delta_ variable
 */
double GExternalEvaluatorIndividualFactory::getDelta() const
{
   return delta_;
}

/******************************************************************************/
/**
 * Set the value of the delta_ variable
 */
void GExternalEvaluatorIndividualFactory::setDelta(double delta)
{
   delta_ = delta;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxDelta() const
{
   return maxDelta_;
}

/******************************************************************************/
/**
 * Set the value of the maxDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxDelta(double maxDelta)
{
   maxDelta_ = maxDelta;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma1_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxSigma1() const
{
   return maxSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxSigma1(double maxSigma1)
{
   maxSigma1_ = maxSigma1;
}

/******************************************************************************/
/**
 * Allows to retrieve the maxSigma2_ variable
 */
double GExternalEvaluatorIndividualFactory::getMaxSigma2() const
{
   return maxSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the maxSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setMaxSigma2(double maxSigma2)
{
   maxSigma2_ = maxSigma2;
}

/******************************************************************************/
/**
 * Allows to retrieve the minDelta_ variable
 */
double GExternalEvaluatorIndividualFactory::getMinDelta() const
{
   return minDelta_;
}

/******************************************************************************/
/**
 * Set the value of the minDelta_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinDelta(double minDelta)
{
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

   if(min < 0) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setDeltaRange(): Error" << std::endl
      << "min must be >= 0. Got : " << max << std::endl
      << GEXCEPTION;
   }

   if(min >= max) {
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
double GExternalEvaluatorIndividualFactory::getMinSigma1() const
{
   return minSigma1_;
}

/******************************************************************************/
/**
 * Set the value of the minSigma1_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinSigma1(double minSigma1)
{
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

   if(min < 0) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setSigma1Range(): Error" << std::endl
      << "min must be >= 0. Got : " << max << std::endl
      << GEXCEPTION;
   }

   if(min >= max) {
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
double GExternalEvaluatorIndividualFactory::getMinSigma2() const
{
   return minSigma2_;
}

/******************************************************************************/
/**
 * Set the value of the minSigma2_ variable
 */
void GExternalEvaluatorIndividualFactory::setMinSigma2(double minSigma2)
{
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

   if(min < 0) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setSigma2Range(): Error" << std::endl
      << "min must be >= 0. Got : " << max << std::endl
      << GEXCEPTION;
   }

   if(min >= max) {
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
 * Allows to set the name and path of the external program
 */
void GExternalEvaluatorIndividualFactory::setProgramName(std::string programName) {
   // Check that the file name isn't empty
   if(programName.empty()) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << std::endl
      << "File name was empty" << std::endl
      << GEXCEPTION;
   }

   // Check that the file exists
   if(!bf::exists(programName)) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setProgramName(): Error!" << std::endl
      << "External program " << programName << " does not seem to exist" << std::endl
      << GEXCEPTION;
   }

   programName_ = programName;
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
 * Sets the name of the external evaluation program
 */
void GExternalEvaluatorIndividualFactory::setCustomOptions(std::string customOptions) {
   customOptions_ = customOptions;
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
   if(parameterFileBaseName.empty()) {
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
   if(initValues != "random" && initValues != "min" && initValues != "max") {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setInitValues(): Error!" << std::endl
      << "Invalid argument: " << initValues << std::endl
      << "Expected \"random\", \"min\" or \"max\"." << std::endl
      << GEXCEPTION;
   }

   initValues_ = initValues;
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
 * Creates items of this type
 *
 * @return Items of the desired type
 */
boost::shared_ptr<GParameterSet> GExternalEvaluatorIndividualFactory::getObject_(
   Gem::Common::GParserBuilder& gpb
   , const std::size_t& id
) {
   // Will hold the result
   boost::shared_ptr<GExternalEvaluatorIndividual> target(new GExternalEvaluatorIndividual());

   // Make the object's local configuration options known
   target->addConfigurationOptions(gpb);

   return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GExternalEvaluatorIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {
   // Describe our own options
   using namespace Gem::Courtier;

   std::string comment;

   comment = "";
   comment += "The probability for random adaption of values in evolutionary algorithms;";
   gpb.registerFileParameter<double>(
      "adProb"
      , adProb_.reference()
      , GEEI_DEF_ADPROB
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The number of calls to an adaptor after which adaption takes place;";
   gpb.registerFileParameter<boost::uint32_t>(
      "adaptionThreshold"
      , adaptionThreshold_.reference()
      , GEEI_DEF_ADAPTIONTHRESHOLD
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "Whether to use a double gaussion for the adaption of parmeters in ES;";
   gpb.registerFileParameter<bool>(
      "useBiGaussian"
      , useBiGaussian_.reference()
      , GEEI_DEF_USEBIGAUSSIAN
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The sigma for gauss-adaption in ES;(or the sigma of the left peak of a double gaussian);";
   gpb.registerFileParameter<double>(
      "sigma1"
      , sigma1_.reference()
      , GEEI_DEF_SIGMA1
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "Influences the self-adaption of gauss-mutation in ES;";
   gpb.registerFileParameter<double>(
      "sigmaSigma1"
      , sigmaSigma1_.reference()
      , GEEI_DEF_SIGMASIGMA1
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The minimum value of sigma1;";
   gpb.registerFileParameter<double>(
      "minSigma1"
      , minSigma1_.reference()
      , GEEI_DEF_MINSIGMA1
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The maximum value of sigma1;";
   gpb.registerFileParameter<double>(
      "maxSigma1"
      , maxSigma1_.reference()
      , GEEI_DEF_MAXSIGMA1
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The sigma of the right peak of a double gaussian (if any);";
   gpb.registerFileParameter<double>(
      "sigma2"
      , sigma2_.reference()
      , GEEI_DEF_SIGMA2
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "Influences the self-adaption of gauss-mutation in ES;";
   gpb.registerFileParameter<double>(
      "sigmaSigma2"
      , sigmaSigma2_.reference()
      , GEEI_DEF_SIGMASIGMA2
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The minimum value of sigma2;";
   gpb.registerFileParameter<double>(
      "minSigma2"
      , minSigma2_.reference()
      , GEEI_DEF_MINSIGMA2
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The maximum value of sigma2;";
   gpb.registerFileParameter<double>(
      "maxSigma2"
      , maxSigma2_.reference()
      , GEEI_DEF_MAXSIGMA2
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The start distance between both peaks used for bi-gaussian mutations in ES;";
   gpb.registerFileParameter<double>(
      "delta"
      , delta_.reference()
      , GEEI_DEF_DELTA
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The width of the gaussian used for mutations of the delta parameter;";
   gpb.registerFileParameter<double>(
      "sigmaDelta"
      , sigmaDelta_.reference()
      , GEEI_DEF_SIGMADELTA
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The minimum allowed value of delta;";
   gpb.registerFileParameter<double>(
      "minDelta"
      , minDelta_.reference()
      , GEEI_DEF_MINDELTA
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The maximum allowed value of delta;";
   gpb.registerFileParameter<double>(
      "maxDelta"
      , maxDelta_.reference()
      , GEEI_DEF_MAXDELTA
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The name of the external evaluation program";
   gpb.registerFileParameter<std::string>(
      "programName"
      , programName_.reference()
      , GEEI_DEF_PROGNAME
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "Any custom options you wish to pass to the external evaluator";
   gpb.registerFileParameter<std::string>(
      "customOptions"
      , customOptions_.reference()
      , GEEI_DEF_CUSTOMOPTIONS
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "The base name assigned to parameter files;";
   comment += "in addition to data identifying this specific evaluation;";
   gpb.registerFileParameter<std::string>(
      "parameterFile"
      , parameterFileBaseName_.reference()
      , GEEI_DEF_PARFILEBASENAME
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   comment = "";
   comment += "Indicates, whether individuals should be initialized randomly (random),;";
   comment += "with the lower (min) or upper (max) boundary of their value ranges;";
   gpb.registerFileParameter<std::string>(
      "initValues"
      , initValues_.reference()
      , GEEI_DEF_STARTMODE
      , Gem::Common::VAR_IS_ESSENTIAL
      , comment
   );

   // Allow our parent class to describe its options
   Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * This function asks the external evaluation program to perform any necessary
 * setup work and then queries it for setup-information, storing the data in
 * a boost::property_tree object. Note that this function will do nothing when
 * called more than once.
 */
void GExternalEvaluatorIndividualFactory::setUpPropertyTree() {
   if(externalEvaluatorQueried_) return;
   else externalEvaluatorQueried_= true;

   // Check that the file name isn't empty
   if(programName_.value().empty()) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << std::endl
      << "File name was empty" << std::endl
      << GEXCEPTION;
   }

   // Check that the file exists
   if(!bf::exists(programName_.value())) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::setUpPropertyTree(): Error!" << std::endl
      << "External program " << programName_.value() << " does not seem to exist" << std::endl
      << GEXCEPTION;
   }

   // Make sure the property tree is empty
   ptr_.clear();

   std::string programCommand;
   if(customOptions_.value() == "empty" || customOptions_.value().empty()) {
      programCommand = programName_.value();
   } else {
      programCommand = programName_.value() + " " + customOptions_.value();
   }

   // Ask the external evaluation program to perform any initial work
   Gem::Common::runExternalCommand(programCommand + " --init");

   // Query the external evaluator for setup information for our individuals
   Gem::Common::runExternalCommand(programCommand + " --initvalues=" + initValues_.value() + " --setup=\"./setup.xml\"");

   // Parse the setup file
   pt::read_xml("./setup.xml", ptr_);

   // Clean up
   bf::remove("./setup.xml");
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GExternalEvaluatorIndividualFactory::postProcess_(boost::shared_ptr<GParameterSet>& p_raw) {
   using boost::property_tree::ptree;

   // Convert the base pointer to the target type
   boost::shared_ptr<GExternalEvaluatorIndividual> p
      = Gem::Common::convertSmartPointer<GParameterSet, GExternalEvaluatorIndividual>(p_raw);

   // Set up a random number generator
   Gem::Hap::GRandom gr;

   // Here we ask the external evaluator to perform any necessary setup work
   // and query it for the desired structure of our individuals. The work is done
   // but once, and the results are stored in a private object inside of this class.
   this->setUpPropertyTree();

   if(ptr_.empty()) {
      glogger
      << "In GExternalEvaluatorIndividualFactory::postProcess_(): Error!" << std::endl
      << "Property tree is empty." << std::endl
      << GEXCEPTION;
   }

   // Set up an adaptor for the collection, so they know how to be adapted
   boost::shared_ptr<GAdaptorT<double> > gat_ptr;
   if(useBiGaussian_) {
      boost::shared_ptr<GDoubleBiGaussAdaptor> gdbga_ptr(new GDoubleBiGaussAdaptor());
      gdbga_ptr->setAllSigma1(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_);
      gdbga_ptr->setAllSigma2(sigma2_, sigmaSigma2_, minSigma2_, maxSigma2_);
      gdbga_ptr->setAllDelta(delta_, sigmaDelta_, minDelta_, maxDelta_);
      gdbga_ptr->setAdaptionThreshold(adaptionThreshold_);
      gdbga_ptr->setAdaptionProbability(adProb_);
      gat_ptr = gdbga_ptr;
   } else {
      boost::shared_ptr<GDoubleGaussAdaptor> gdga_ptr(new GDoubleGaussAdaptor(sigma1_, sigmaSigma1_, minSigma1_, maxSigma1_));
      gdga_ptr->setAdaptionThreshold(adaptionThreshold_);
      gdga_ptr->setAdaptionProbability(adProb_);
      gat_ptr = gdga_ptr;
   }

   // Extract the number of variables
   std::size_t nvar = ptr_.get<std::size_t>("parameterset.nvar");

   // Extract the number of results to be expected from the external evaluation function
   std::size_t nResultsExpected = ptr_.get<std::size_t>("parameterset.nResultsExpected");

   // Get the parameter set node
   ptree parSetNode = ptr_.get_child("parameterset");

   // Loop over all children of the property tree
   // Note that for now we only query GConstrainedDoubleObject objects
   std::size_t varCounter = 0;
   std::string varString = "var0";
   ptree::const_iterator cit;
   for (cit = parSetNode.begin(); cit != parSetNode.end(); ++cit) {
      if(varString == cit->first) { // O.k., we found a var string
         // Just treat GConstrainedDoubleObject objects for now
         if((cit->second).get<std::string>("type") == "GConstrainedDoubleObject") {
            // Extract the boundaries and initial values
            std::string pName = (cit->second).get<std::string>("name");
            double minVar     = (cit->second).get<double>("lowerBoundary");
            double maxVar     = (cit->second).get<double>("upperBoundary");
            double initValue  = (cit->second).get<double>("value0");

            // Create an initial (empty) pointer to a GConstrainedDoubleObject
            boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr;

            // Act on the information, depending on whether random initialization has been requested
            if(0 == (cit->second).count("initRandom") || false==(cit->second).get<bool>("initRandom")) {
               // Extract the initial value
               double initValue = (cit->second).get<double>("value0");
               // Create the parameter object
               gcdo_ptr = boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(initValue, minVar, maxVar));
            } else { // Random initialization has been requested
               // Create the parameter object
               gcdo_ptr = boost::shared_ptr<GConstrainedDoubleObject>(new GConstrainedDoubleObject(minVar, maxVar));
            }
            gcdo_ptr->setParameterName(pName);

            // Add the adaptor to the parameter object
            gcdo_ptr->addAdaptor(gat_ptr);

            // Add the object to the individual
            p->push_back(gcdo_ptr);
         }

         if(++varCounter >= nvar) break; // Terminate the loop if we have identified all expected parameter objects
         else {
            varString = std::string("var") + boost::lexical_cast<std::string>(varCounter); // Create a new var string
         }
      }
   }

   // Add the program name and base name for parameter transfers to the object
   p->setExchangeFileName(parameterFileBaseName_);
   p->setProgramName(programName_);
   p->setCustomOptions(customOptions_);
   p->setNExpectedResults(nResultsExpected);
}

/******************************************************************************/


} /* namespace Geneva */
} /* namespace Gem */
