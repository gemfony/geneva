/**
 * @file GBaseSA.cpp
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
#include "geneva/GBaseSA.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GBaseSA::GSAOptimizationMonitor)

namespace Gem {
namespace Geneva {

/******************************************************************************/
/** A short identifier suitable for storage in a std::map */
G_API_GENEVA const std::string GBaseSA::nickname = "sa";

/******************************************************************************/
/**
 * The default constructor. All initialization work of member variable
 * is done in the class body.
 */
GBaseSA::GBaseSA() : GParameterSetParChild()
{
	// Register the default optimization monitor
	this->registerOptimizationMonitor(
		std::shared_ptr<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(
			new GSAOptimizationMonitor()
		)
	);

	// Make sure we start with a valid population size if the user does not supply these values
	this->setPopulationSizes(100, 1);
}

/******************************************************************************/
/**
 * A standard copy constructor. Note that the generation number is reset to 0 and
 * is not copied from the other object. We assume that a new optimization run will
 * be started.
 *
 * @param cp Another GBaseSA object
 */
GBaseSA::GBaseSA(const GBaseSA &cp)
	: GParameterSetParChild(cp)
	, t0_(cp.t0_)
	, t_(cp.t_)
	, alpha_(cp.alpha_)
{
	// Copying / setting of the optimization algorithm id is done by the parent class. The same
	// applies to the copying of the optimization monitor.
}

/******************************************************************************/
/**
 * The standard destructor. All work is done in the parent class.
 */
GBaseSA::~GBaseSA()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBaseSA &GBaseSA::operator=(const GBaseSA &cp) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GBaseSA object
 *
 * @param  cp A constant reference to another GBaseSA object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseSA::operator==(const GBaseSA &cp) const {
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
 * Checks for inequality with another GBaseSA object
 *
 * @param  cp A constant reference to another GBaseSA object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseSA::operator!=(const GBaseSA &cp) const {
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
 * Loads the data of another GBaseSA object, camouflaged as a GObject.
 *
 * @param cp A pointer to another GBaseSA object, camouflaged as a GObject
 */
void GBaseSA::load_(const GObject *cp) {
	// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
	const GBaseSA *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSA>(cp, this);

	// First load the parent class'es data ...
	GParameterSetParChild::load_(cp);

	// ... and then our own data
	t0_ = p_load->t0_;
	t_ = p_load->t_;
	alpha_ = p_load->alpha_;
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
void GBaseSA::compare(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBaseSwarm::GSwarmOptimizationMonitor reference independent of this object and convert the pointer
	const GBaseSA *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSA>(cp, this);

	GToken token("GBaseSA", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GParameterSetParChild>(IDENTITY(*this, *p_load), token);

	// ... and then the local data
	compare_t(IDENTITY(t0_, p_load->t0_), token);
	compare_t(IDENTITY(t_, p_load->t_), token);
	compare_t(IDENTITY(alpha_, p_load->alpha_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Emits a name for this class / object
 */
std::string GBaseSA::name() const {
	return std::string("GBaseSA");
}

/******************************************************************************/
/**
 * Returns information about the type of optimization algorithm. This function needs
 * to be overloaded by the actual algorithms to return the correct type.
 *
 * @return The type of optimization algorithm
 */
std::string GBaseSA::getOptimizationAlgorithm() const {
	return "PERSONALITY_SA";
}

/******************************************************************************/
/**
 * Returns the name of this optimization algorithm
 *
 * @return The name assigned to this optimization algorithm
 */
std::string GBaseSA::getAlgorithmName() const {
	return std::string("Simulated Annealing");
}

/******************************************************************************/
/**
 * The function checks that the population size meets the requirements and does some
 * tagging. It is called from within GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::optimize(), before the
 * actual optimization cycle starts.
 */
void GBaseSA::init() {
	// To be performed before any other action
	GParameterSetParChild::init();
}

/******************************************************************************/
/**
 * Does any necessary finalization work
 */
void GBaseSA::finalize() {
	// Last action
	GParameterSetParChild::finalize();
}

/******************************************************************************/
/**
 * Retrieve a GPersonalityTraits object belonging to this algorithm
 */
std::shared_ptr <GPersonalityTraits> GBaseSA::getPersonalityTraits() const {
	return std::shared_ptr<GSAPersonalityTraits>(new GSAPersonalityTraits());
}

/******************************************************************************/
/**
 * Performs a simulated annealing style sorting and selection
 */
void GBaseSA::sortSAMode() {
	// Position the nParents best children of the population right behind the parents
	std::partial_sort(
		data.begin() + nParents_, data.begin() + 2 * nParents_, data.end(),
		[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);

	// Check for each parent whether it should be replaced by the corresponding child
	for (std::size_t np = 0; np < nParents_; np++) {
		double pPass = saProb(this->at(np)->minOnly_fitness(), this->at(nParents_ + np)->minOnly_fitness());
		if (pPass >= 1.) {
			this->at(np)->GObject::load(this->at(nParents_ + np));
		} else {
			double challenge = gr.uniform_01<double>();
			if (challenge < pPass) {
				this->at(np)->GObject::load(this->at(nParents_ + np));
			}
		}
	}

	// Sort the parents -- it is possible that a child with a worse fitness has replaced a parent
	std::sort(
		data.begin(), data.begin() + nParents_,
		[](std::shared_ptr <GParameterSet> x, std::shared_ptr <GParameterSet> y) -> bool {
			return x->minOnly_fitness() < y->minOnly_fitness();
		}
	);

	// Make sure the temperature gets updated
	updateTemperature();
}

/******************************************************************************/
/**
 * Calculates the simulated annealing probability for a child to replace a parent
 *
 * @param qParent The fitness of the parent
 * @param qChild The fitness of the child
 * @return A double value in the range [0,1[, representing the likelihood for the child to replace the parent
 */
double GBaseSA::saProb(const double &qParent, const double &qChild) {
	// We do not have to do anything if the child is better than the parent
	if (this->isBetter(qChild, qParent)) {
		return 2.;
	}

	double result = 0.;
	if (this->getMaxMode()) {
		result = exp(-(qParent - qChild) / t_);
	} else {
		result = exp(-(qChild - qParent) / t_);
	}

	return result;
}

/******************************************************************************/
/**
 * Updates the temperature. This function is used for simulated annealing.
 */
void GBaseSA::updateTemperature() {
	t_ *= alpha_;
}

/******************************************************************************/
/**
 * Determines the strength of the temperature degradation. This function is used for simulated annealing.
 *
 * @param alpha The degradation speed of the temperature
 */
void GBaseSA::setTDegradationStrength(double alpha) {
	if (alpha <= 0.) {
		glogger
		<< "In GBaseSA::setTDegradationStrength(const double&):" << std::endl
		<< "Got negative alpha: " << alpha << std::endl
		<< GEXCEPTION;
	}

	alpha_ = alpha;
}

/******************************************************************************/
/**
 * Retrieves the temperature degradation strength. This function is used for simulated annealing.
 *
 * @return The temperature degradation strength
 */
double GBaseSA::getTDegradationStrength() const {
	return alpha_;
}

/******************************************************************************/
/**
 * Sets the start temperature. This function is used for simulated annealing.
 *
 * @param t0 The start temperature
 */
void GBaseSA::setT0(double t0) {
	if (t0 <= 0.) {
		glogger
		<< "In GBaseSA::setT0(const double&):" << std::endl
		<< "Got negative start temperature: " << t0 << std::endl
		<< GEXCEPTION;
	}

	t0_ = t0;
}

/******************************************************************************/
/**
 * Retrieves the start temperature. This function is used for simulated annealing.
 *
 * @return The start temperature
 */
double GBaseSA::getT0() const {
	return t0_;
}

/******************************************************************************/
/**
 * Retrieves the current temperature. This function is used for simulated annealing.
 *
 * @return The current temperature
 */
double GBaseSA::getT() const {
	return t_;
}

/******************************************************************************/
/**
 * Adds local configuration options to a GParserBuilder object
 *
 * @param gpb The GParserBuilder object to which configuration options should be added
 */
void GBaseSA::addConfigurationOptions(
	Gem::Common::GParserBuilder &gpb
) {
	// Call our parent class'es function
	GParameterSetParChild::addConfigurationOptions(gpb);

	gpb.registerFileParameter<double>(
		"t0" // The name of the variable
		, SA_T0 // The default value
		, [this](double sat0) { this->setT0(sat0); }
	)
	<< "The start temperature used in simulated annealing";

	gpb.registerFileParameter<double>(
		"alpha" // The name of the variable
		, SA_ALPHA // The default value
		, [this](double ds) { this->setTDegradationStrength(ds); }
	)
	<< "The degradation strength used in the cooling" << std::endl
	<< "schedule in simulated annealing;";
}

/******************************************************************************/
/**
 * Some error checks related to population sizes
 */
void GBaseSA::populationSanityChecks() const {
	// First check that we have been given a suitable value for the number of parents.
	// Note that a number of checks (e.g. population size != 0) has already been done
	// in the parent class.
	if (nParents_ == 0) {
		glogger
		<< "In GBaseSA::populationSanityChecks(): Error!" << std::endl
		<< "Number of parents is set to 0"
		<< GEXCEPTION;
	}

	// We need at least as many children as parents
	std::size_t popSize = getPopulationSize();
	if (popSize <= nParents_) {
		glogger
		<< "In GBaseSA::populationSanityChecks() :" << std::endl
		<< "Requested size of population is too small :" << popSize << " " << nParents_ << std::endl
		<< GEXCEPTION;
	}
}

/******************************************************************************/
/**
 * Choose new parents, based on the SA selection scheme.
 */
void GBaseSA::selectBest() {
	// Sort according to the "Simulated Annealing" scheme
	sortSAMode();

	// Let parents know they are parents
	markParents();
}

/******************************************************************************/
/**
 * Retrieves the evaluation range in a given iteration and sorting scheme. Depending on the
 * iteration and sorting scheme, the start point will be different. The end-point is not meant
 * to be inclusive.
 *
 * @return The range inside which evaluation should take place
 */
boost::tuple<std::size_t, std::size_t> GBaseSA::getEvaluationRange() const {
	// We evaluate all individuals in the first iteration This happens so pluggable
	// optimization monitors do not need to distinguish between algorithms
	return boost::tuple<std::size_t, std::size_t>(
		inFirstIteration() ? 0 : getNParents(), data.size()
	);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 *
 * @return A boolean which indicates whether modifications were made
 */
bool GBaseSA::modify_GUnitTests() {
#ifdef GEM_TESTING

	bool result = false;

	// Call the parent class'es function
	if (GParameterSetParChild::modify_GUnitTests()) result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSA::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseSA::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GParameterSetParChild::specificTestsNoFailureExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseSA::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GParameterSetParChild::specificTestsFailuresExpected_GUnitTests();

	//------------------------------------------------------------------------------
	//------------------------------------------------------------------------------

#else /* GEM_TESTING */
   condnotset("GBaseSA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. All initialization work for member variables is
 * done in the class body.
 */
GBaseSA::GSAOptimizationMonitor::GSAOptimizationMonitor()
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GSAOptimizationMonitor object
 */
GBaseSA::GSAOptimizationMonitor::GSAOptimizationMonitor(
	const GBaseSA::GSAOptimizationMonitor &cp
)
	: GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT(cp)
	, xDim_(cp.xDim_)
	, yDim_(cp.yDim_)
	, nMonitorInds_(cp.nMonitorInds_)
	, resultFile_(cp.resultFile_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GBaseSA::GSAOptimizationMonitor::~GSAOptimizationMonitor()
{ /* nothing */ }

/***************************************************************************/
/**
 * The standard assignment operator
 */
const GBaseSA::GSAOptimizationMonitor &GBaseSA::GSAOptimizationMonitor::operator=(
	const GBaseSA::GSAOptimizationMonitor &cp
) {
	this->load_(&cp);
	return *this;
}

/******************************************************************************/
/**
 * Checks for equality with another GParameter Base object
 *
 * @param  cp A constant reference to another GSAOptimizationMonitor object
 * @return A boolean indicating whether both objects are equal
 */
bool GBaseSA::GSAOptimizationMonitor::operator==(const GBaseSA::GSAOptimizationMonitor &cp) const {
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
 * Checks for inequality with another GSAOptimizationMonitor object
 *
 * @param  cp A constant reference to another GSAOptimizationMonitor object
 * @return A boolean indicating whether both objects are inequal
 */
bool GBaseSA::GSAOptimizationMonitor::operator!=(const GBaseSA::GSAOptimizationMonitor &cp) const {
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
void GBaseSA::GSAOptimizationMonitor::compare(
	const GObject &cp
	, const Gem::Common::expectation &e
	, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GBaseSA::GSAOptimizationMonitor reference independent of this object and convert the pointer
	const GBaseSA::GSAOptimizationMonitor *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSA::GSAOptimizationMonitor>(cp, this);

	GToken token("GBaseSA::GSAOptimizationMonitor", e);

	// Compare our parent data ...
	Gem::Common::compare_base<GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT>(IDENTITY(*this, *p_load),
																														  token);

	// ... and then the local data
	compare_t(IDENTITY(xDim_, p_load->xDim_), token);
	compare_t(IDENTITY(yDim_, p_load->yDim_), token);
	compare_t(IDENTITY(nMonitorInds_, p_load->nMonitorInds_), token);
	compare_t(IDENTITY(resultFile_, p_load->resultFile_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Allows to specify a different name for the result file
 *
 * @param resultFile The desired name of the result file
 */
void GBaseSA::GSAOptimizationMonitor::setResultFileName(
	const std::string &resultFile
) {
	resultFile_ = resultFile;
}

/******************************************************************************/
/**
 * Allows to retrieve the current value of the result file name
 *
 * @return The current name of the result file
 */
std::string GBaseSA::GSAOptimizationMonitor::getResultFileName() const {
	return resultFile_;
}

/******************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseSA::GSAOptimizationMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> *const goa) {
	// Perform the conversion to the target algorithm
#ifdef DEBUG
   if(goa->getOptimizationAlgorithm() != "PERSONALITY_SA") {
      glogger
      <<  "In GBaseSA::GSAOptimizationMonitor::cycleInformation():" << std::endl
      << "Provided optimization algorithm has wrong type: " << goa->getOptimizationAlgorithm() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	// Convert the base pointer to the target type
	GBaseSA *const sa = static_cast<GBaseSA *const>(goa);

#ifdef DEBUG
   if(nMonitorInds_ > sa->size()) {
      glogger
      <<  "In GBaseSA::GSAOptimizationMonitor::cycleInformation():" << std::endl
      << "Provided number of monitored individuals is larger than the population: " << std::endl
      << nMonitorInds_ << " / " << sa->size() << std::endl
      << GEXCEPTION;
   }
#endif /* DEBUG */

	// Determine a suitable number of monitored individuals, if it hasn't already
	// been set externally. We allow a maximum of 3 monitored individuals by default
	// (or the number of parents, if <= 3). Setting the number to 0 will result in
	// the same number of individuals being monitored as the number of parents.
	if (nMonitorInds_ == 0) {
		nMonitorInds_ = (std::min)(sa->getNParents(), std::size_t(3));
	}

	// Set up the plotters
	for (std::size_t ind = 0; ind < nMonitorInds_; ind++) {
		std::shared_ptr <Gem::Common::GGraph2D> graph(new Gem::Common::GGraph2D());
		graph->setXAxisLabel("Iteration");
		graph->setYAxisLabel("Fitness");
		graph->setPlotLabel(std::string("Individual ") + boost::lexical_cast<std::string>(ind));
		graph->setPlotMode(Gem::Common::CURVE);

		fitnessGraphVec_.push_back(graph);
	}
}

/******************************************************************************/
/**
 * A function that is called during each optimization cycle. It is possible to
 * extract quite comprehensive information in each iteration. For examples, see
 * the standard overloads provided for the various optimization algorithms.
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseSA::GSAOptimizationMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> *const goa) {
	bool isDirty = false;
	double currentTransformedEvaluation = 0.;

	// Convert the base pointer to the target type
	GBaseSA *const sa = static_cast<GBaseSA *const>(goa);

	// Retrieve the current iteration
	boost::uint32_t iteration = sa->getIteration();

	for (std::size_t ind = 0; ind < nMonitorInds_; ind++) {
		// Get access to the individual
		std::shared_ptr <GParameterSet> gi_ptr = sa->individual_cast<GParameterSet>(ind);

		// Retrieve the fitness of this individual -- all individuals should be "clean" here
		currentTransformedEvaluation = gi_ptr->transformedFitness();
		// Add the data to our graph
		*(fitnessGraphVec_.at(ind)) & boost::tuple<double, double>(iteration, currentTransformedEvaluation);
	}
}

/******************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param goa A pointer to the current optimization algorithm for which information should be emitted
 */
void GBaseSA::GSAOptimizationMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> *const goa) {
	Gem::Common::GPlotDesigner gpd(
		std::string("Fitness of ") + boost::lexical_cast<std::string>(nMonitorInds_) +
		std::string(" best SA individuals"), 1, nMonitorInds_
	);

	gpd.setCanvasDimensions(xDim_, yDim_);

	// Copy all plotters into the GPlotDesigner object
	std::vector<std::shared_ptr < Gem::Common::GGraph2D>> ::iterator
	it;
	for (it = fitnessGraphVec_.begin(); it != fitnessGraphVec_.end(); ++it) {
		gpd.registerPlotter(*it);
	}

	// Write out the plot
	gpd.writeToFile(this->getResultFileName());

	// Clear all plotters, so they do not get added repeatedly, when
	// optimize is called repeatedly on the same (or a cloned) object.
	fitnessGraphVec_.clear();
}

/******************************************************************************/
/**
 * Allows to set the dimensions of the canvas
 *
 * @param xDim The desired dimension of the canvas in x-direction
 * @param yDim The desired dimension of the canvas in y-direction
 */
void GBaseSA::GSAOptimizationMonitor::setDims(
	const boost::uint16_t &xDim
	, const boost::uint16_t &yDim
) {
	xDim_ = xDim;
	yDim_ = yDim;
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in x-direction
 *
 * @return The dimension of the canvas in x-direction
 */
boost::uint16_t GBaseSA::GSAOptimizationMonitor::getXDim() const {
	return xDim_;
}

/******************************************************************************/
/**
 * Retrieves the dimension of the canvas in y-direction
 *
 * @return The dimension of the canvas in y-direction
 */
boost::uint16_t GBaseSA::GSAOptimizationMonitor::getYDim() const {
	return yDim_;
}

/******************************************************************************/
/**
 * Sets the number of individuals in the population that should be monitored
 *
 * @oaram nMonitorInds The number of individuals in the population that should be monitored
 */
void GBaseSA::GSAOptimizationMonitor::setNMonitorIndividuals(const std::size_t &nMonitorInds) {
	nMonitorInds_ = nMonitorInds;
}

/******************************************************************************/
/**
 * Retrieves the number of individuals that are being monitored
 *
 * @return The number of individuals in the population being monitored
 */
std::size_t GBaseSA::GSAOptimizationMonitor::getNMonitorIndividuals() const {
	return nMonitorInds_;
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * cp A pointer to another GBaseSA::GSAOptimizationMonitor object, camouflaged as a GObject
 */
void GBaseSA::GSAOptimizationMonitor::load_(const GObject *cp) {
	// Check that we are dealing with a GBaseSA::GSAOptimizationMonitor reference independent of this object and convert the pointer
	const GBaseSA::GSAOptimizationMonitor *p_load = Gem::Common::g_convert_and_compare<GObject, GBaseSA::GSAOptimizationMonitor>(cp, this);

	// Load the parent classes' data ...
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::load_(cp);

	// ... and then our local data
	xDim_ = p_load->xDim_;
	yDim_ = p_load->yDim_;
	nMonitorInds_ = p_load->nMonitorInds_;
	resultFile_ = p_load->resultFile_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject *GBaseSA::GSAOptimizationMonitor::clone_() const {
	return new GBaseSA::GSAOptimizationMonitor(*this);
}

/******************************************************************************/
/**
 * Applies modifications to this object. This is needed for testing purposes
 */
bool GBaseSA::GSAOptimizationMonitor::modify_GUnitTests() {
#ifdef GEM_TESTING
	bool result = false;

	// Call the parent class'es function
	if (GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::modify_GUnitTests()) result = true;

	this->setDims(this->getXDim() + 1, this->getYDim() + 1);
	result = true;

	return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSA::GSAOptimizationMonitor::modify_GUnitTests", "GEM_TESTING");
   return false;
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to succeed. This is needed for testing purposes
 */
void GBaseSA::GSAOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsNoFailureExpected_GUnitTests();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
   condnotset("GBaseSA::GSAOptimizationMonitor::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/
/**
 * Performs self tests that are expected to fail. This is needed for testing purposes
 */
void GBaseSA::GSAOptimizationMonitor::specificTestsFailuresExpected_GUnitTests() {
#ifdef GEM_TESTING
	// Call the parent class'es function
	GOptimizationAlgorithmT<GParameterSet>::GOptimizationMonitorT::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */
   condnotset("GBaseSA::GSAOptimizationMonitor::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
