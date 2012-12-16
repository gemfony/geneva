/**
 * @file GMultiCriterionParabolaIndividual.hpp
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

#include "GMultiCriterionParabolaIndividual.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GMultiCriterionParabolaIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor. It is declared private in the header file, as it will only be
 * needed for (de-)serialization purposes. As all variables will be set by the Boost.Serialization
 * library, no variables are initialized here.
 */
GMultiCriterionParabolaIndividual::GMultiCriterionParabolaIndividual()
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GMultiCriterionParabolaIndividual::~GMultiCriterionParabolaIndividual()
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard copy constructor. All real work is done by the parent class. All we need to
 * do here is to copy local data.
 *
 * @param cp A copy of another GMultiCriterionParabolaIndividual
 */
GMultiCriterionParabolaIndividual::GMultiCriterionParabolaIndividual(const GMultiCriterionParabolaIndividual& cp)
	: GParameterSet(cp)
	, nPar_(cp.nPar_)
	, par_min_(cp.par_min_)
	, par_max_(cp.par_max_)
	, minima_(cp.minima_)
{ /* nothing */ }

/******************************************************************************/
/**
 * A standard assignment operator
 *
 * @param cp A copy of another GMultiCriterionParabolaIndividual object
 * @return A constant reference to this object
 */
const GMultiCriterionParabolaIndividual& GMultiCriterionParabolaIndividual::operator=(const GMultiCriterionParabolaIndividual& cp){
	GMultiCriterionParabolaIndividual::load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 * @param showOrigin Makes the function indicate the origin of parameters in comments
 */
void GMultiCriterionParabolaIndividual::addConfigurationOptions (
   Gem::Common::GParserBuilder& gpb
   , const bool& showOrigin
) {
   std::string comment;

   // Call our parent class'es function
   GParameterSet::addConfigurationOptions(gpb, showOrigin);

   // Add local data
   comment = "";
   comment += "The lower boundary of the parabola;";
   gpb.registerFileParameter(
         "par_min"
         , par_min_
         , par_min_
         , Gem::Common::VAR_IS_ESSENTIAL
         , comment
   );

   comment = "";
   comment += "The upper boundary of the parabola;";
   gpb.registerFileParameter(
         "par_max"
         , par_max_
         , par_max_
         , Gem::Common::VAR_IS_ESSENTIAL
         , comment
   );

   comment = "";
   comment += "A list of optima, encoded as a string;";
   gpb.registerFileParameter(
         "minima"
         , minima_string_
         , std::string("-1. 0. 1.")
         , Gem::Common::VAR_IS_ESSENTIAL
         , comment
   );
}

/******************************************************************************/
/**
 * Initialize with stored values
 */
void GMultiCriterionParabolaIndividual::init() {
   // Parse the minima string and break it into timing values
   std::vector<std::string> minimaTokens;
   typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
   boost::char_separator<char> space_sep(" ");
   tokenizer minimaTokenizer(minima_string_, space_sep);
   tokenizer::iterator s;
   std::size_t parabola_counter = 0;
   for(s=minimaTokenizer.begin(); s!=minimaTokenizer.end(); ++s){
      std::cout << "Parabola " << parabola_counter++ << " minimum = " << *s << std::endl;
      minimaTokens.push_back(*s);
   }
   if(minimaTokens.empty()) { // No sleep tokens were provided
      glogger
      << "In GMultiCriterionParabolaIndividual::init(): Error!" << std::endl
      << "You did not provide any minimum settings" << std::endl
      << GEXCEPTION;
   }

   minima_.clear();

   std::vector<std::string>::iterator t;
   for(t=minimaTokens.begin(); t!=minimaTokens.end(); ++t) {
      // Assign the converted minimum
      minima_.push_back(boost::lexical_cast<double>(*t));
   }

   // Initialize the number of parabolas
   nPar_ = minima_.size();

   for(std::size_t npar=0; npar<nPar_; npar++) {
      // GConstrainedDoubleObject cannot assume value below or above par_min_/max_
      boost::shared_ptr<GConstrainedDoubleObject> gcdo_ptr(new GConstrainedDoubleObject(par_min_, par_max_));
      // Assign a random value in the expected range
      gcdo_ptr->setValue(gr.uniform_real<double>(par_min_, par_max_));
      // Add the parameters to this individual
      this->push_back(gcdo_ptr);
   }
}

/******************************************************************************/
/**
 * Loads the data of another GMultiCriterionParabolaIndividual, camouflaged as a GObject.
 *
 * @param cp A copy of another GMultiCriterionParabolaIndividual, camouflaged as a GObject
 */
void GMultiCriterionParabolaIndividual::load_(const GObject* cp)
{
	const GMultiCriterionParabolaIndividual *p_load = GObject::gobject_conversion<GMultiCriterionParabolaIndividual>(cp);

	// Load our parent's data ...
	GParameterSet::load_(cp);

	// Local data
	nPar_ = p_load->nPar_;
	par_min_ = p_load->par_min_;
	par_max_ = p_load->par_max_;
	minima_ = p_load->minima_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject* GMultiCriterionParabolaIndividual::clone_() const {
	return new GMultiCriterionParabolaIndividual(*this);
}

/******************************************************************************/
/**
 * The actual fitness calculation takes place here.
 *
 * @return The value of this object
 */
double GMultiCriterionParabolaIndividual::fitnessCalculation(){
	double main_result = 0.; // Will hold the main result
	std::vector<double> parVec; // Will hold the individual parameters

	this->streamline(parVec); // Retrieve the parameters

	// Do the actual calculations. Note that the first calculation
	// counts as the main result and that we can register other,
	// secondary evaluation criteria.
	main_result = GSQUARED(parVec[0] - minima_[0]);
	for(std::size_t i=1; i<parVec.size(); i++) {
		registerSecondaryResult(GSQUARED(parVec[i] - minima_[i]));
	}

	return main_result;
}

/******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor for this class
 *
 * @param cF The name of the configuration file
 */
GMultiCriterionParabolaIndividualFactory::GMultiCriterionParabolaIndividualFactory(const std::string& cF)
	: GFactoryT<GMultiCriterionParabolaIndividual>(cF)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GMultiCriterionParabolaIndividualFactory::~GMultiCriterionParabolaIndividualFactory()
{ /* nothing */ }

/******************************************************************************/
/**
 * Allows to describe configuration options in derived classes
 */
void GMultiCriterionParabolaIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder& gpb)
{ /* nothing */ }

/******************************************************************************/
/**
 * Creates individuals of the desired type. The argument "id" gives the function a means
 * of detecting how often it has been called before. The id will be incremented for each call.
 * This can e.g. be used to act differently for the first call to this function.
 *
 * @param id The id of the individual to be created
 * @return An individual of the desired type
 */
boost::shared_ptr<GMultiCriterionParabolaIndividual> GMultiCriterionParabolaIndividualFactory::getObject_(
      Gem::Common::GParserBuilder& gpb
      , const std::size_t& id
) {
   // Will hold the result
   boost::shared_ptr<GMultiCriterionParabolaIndividual> target(new GMultiCriterionParabolaIndividual());

   // Make the object's local configuration options known
   target->addConfigurationOptions(gpb, true);

   return target;
}

/******************************************************************************/

void GMultiCriterionParabolaIndividualFactory::postProcess_(
      boost::shared_ptr<GMultiCriterionParabolaIndividual>& p
) {
   p->init();
}

} /* namespace Geneva */
} /* namespace Gem */
