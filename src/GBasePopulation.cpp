/*
 * \file
 */

/**
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GBasePopulation.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GBasePopulation)

namespace GenEvA
{

  /***********************************************************************************/
  /**
   * The default constructor, It sets all internal variables to sensible values.
   * In particular, as we do not have any individuals yet, we set the population size,
   * and number of parents to 0. It is the philosophy of this class not
   * to provide constructors for each and every use case. Instead, you should set
   * vital parameters, such as the population size or the parent individuals by hand.
   */
  GBasePopulation::GBasePopulation(void)
    :GManagedMemberCollection(),
     maxDuration_(boost::posix_time::duration_from_string(DEFAULTDURATION))
  {
    setGeneration(0);
    setMaxGeneration(DEFAULTMAXGEN);
    setSortingScheme(MUPLUSNU);
    setMaximize(DEFAULTMAXMODE);
    setRecombinationMethod(DEFAULTRECOMBINE);
    setReportGeneration(DEFAULTREPORTGEN);

    nParents_ = 0;
    popSize_=0;
    defaultChildren_=0;

    // The "real" id will be set in the GBasePopulation::optimize function
    id_="empty";
    firstId_ = true;

    // Store function objects for minimization and maximization.
    // Uses Boost.Bind and Boost.Function.
    f_min_ = boost::bind(
			 std::less<double>(),
			 boost::bind(&GMember::fitness,_1),
			 boost::bind(&GMember::fitness,_2)
			 );

    // Maximization can be achieved by swapping
    // the two input parameters while still using
    // the std::less<double>() adaptor.
    f_max_ = boost::bind(
			 std::less<double>(),
			 boost::bind(&GMember::fitness,_2),
			 boost::bind(&GMember::fitness,_1)
			 );
  }

  /***********************************************************************************/
  /**
   * A standard copy constructor. Note that the generation number is reset to 0 and
   * is not copied from the other object. We assume that a new optimization run will
   * be started.
   *
   * \param cp Another GBasePopulation object
   */
  GBasePopulation::GBasePopulation(const GBasePopulation& cp)
    :GManagedMemberCollection(cp)
  {
    setGeneration(0);
    setMaxGeneration(cp.maxGeneration_);
    setSortingScheme(cp.muplusnu_);
    setMaximize(cp.maximize_);
    setRecombinationMethod(cp.recombinationMethod_);
    nParents_=cp.nParents_;
    popSize_=cp.popSize_;
    defaultChildren_=cp.defaultChildren_;
    setReportGeneration(cp.reportGeneration_);
    id_=cp.id_;
    maxDuration_ = cp.maxDuration_;
    // We can copy boost::function objects like any ordinary variable
    f_max_=cp.f_max_;
    f_min_=cp.f_min_;
    infoFunction_=cp.infoFunction_;

    // We want the id to be re-calculated for a new object
    firstId_ = true;
  }

  /***********************************************************************************/
  /**
   * All reset work is done in the parent class, as we have no local, dynamically
   * allocated data.
   */
  GBasePopulation::~GBasePopulation()
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * The assignment operator works like the copy constructor.
   *
   * \param cp Another GBasePopulation object
   * \return A constant reference to this object
   */
  const GBasePopulation& GBasePopulation::operator=(const GBasePopulation& cp) {
    GBasePopulation::load(&cp);
    return *this;
  }

  /***********************************************************************************/
  /**
   * A standard assignment operator for GBasePopulation objects, camouflaged as a
   * GObject
   *
   * \param cp A GBasePopulation object, camouflaged as a GObject
   * \return A constant reference to this object
   */
  const GObject& GBasePopulation::operator=(const GObject& cp){
    GBasePopulation::load(&cp);
    return *this;
  }

  /***********************************************************************************/
  /**
   * We reset the population to the same state it was in upon initialization with the default
   * constructor. In particular, all individuals of the population are destroyed in the
   * GManagedMemberCollection::reset() function. Afterwards, the population is empty, and we need
   * to set the nParents_ and popSize_ variables manually. This function is found in
   * almost all GenEvA classes.
   */
  void GBasePopulation::reset(void)
  {
    setGeneration(0);
    setMaxGeneration(DEFAULTMAXGEN);
    setSortingScheme(MUPLUSNU);
    setMaximize(DEFAULTMAXMODE);
    setRecombinationMethod(DEFAULTRECOMBINE);

    GManagedMemberCollection::reset();

    nParents_ = 0;
    popSize_=0;
    defaultChildren_=0;
    setReportGeneration(DEFAULTREPORTGEN);
    maxDuration_ = boost::posix_time::duration_from_string(DEFAULTDURATION);

    id_="empty";
    firstId_=true;
  }

  /***********************************************************************************/
  /**
   * The load() function is a standard feature of almost all GenEvA classes.
   * In our case it takes another GBasePopulation object (camouflaged as a
   * pointer to a GBase object) and loads all its internal data. First we check
   * that we are no accidently assigning a copy of this object to itself. Then, all
   * individuals of the population are loaded using the GManagedMemberCollection::load()
   * function. Then we load the internal data of the other population. Note that the
   * generation number is reset to 0 and is not copied from the other object. We
   * assume that a new optimization run will be started. We do not copy the function
   * objects f_min_ and f_max_, as these must already have been constructed by the
   * constructors.
   *
   * \param cp A pointer to another GBasePopulation object, camouflaged as a GObject
   */
  void GBasePopulation::load(const GObject * cp)
  {
    // Check that this object is not accidently assigned to itself.
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GBasePopulation *>(cp) == this){
      GException ge;
      ge << "In GBasePopulation::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }

    GManagedMemberCollection::load(cp);

    // Get a pointer to a GBasePopulation ...
    const GBasePopulation *gbp = dynamic_cast<const GBasePopulation *>(cp);

    if(gbp){ // NULL pointer ?
      setGeneration(0);
      setMaxGeneration(gbp->maxGeneration_);
      setSortingScheme(gbp->muplusnu_);
      setMaximize(gbp->maximize_);
      setRecombinationMethod(gbp->recombinationMethod_);
      nParents_=gbp->nParents_;
      popSize_=gbp->popSize_;
      defaultChildren_=gbp->defaultChildren_;
      setReportGeneration(gbp->reportGeneration_);
      id_="empty";
      firstId_=true; // We want the id to be re-calculated
      maxDuration_=gbp->maxDuration_;

      // We can copy boost::function objects like any ordinary variable
      f_max_=gbp->f_max_;
      f_min_=gbp->f_min_;
      infoFunction_=gbp->infoFunction_;
    }
    else{
      GException ge;
      ge << "In GBasePopulation::load() : Conversion error! " << endl
	 << raiseException;
    }
  }

  /***********************************************************************************/
  /**
   * The clone() function is a standard feature of almost all GenEvA classes.
   * It creates a deep copy of the current object and thus acts like a factory
   * function.
   *
   * \return A deep copy of this object, camouflaged as a pointer to a GObject
   */
  GObject *GBasePopulation::clone(void) {
    return new GBasePopulation(*this);
  }

  /***********************************************************************************/
  /**
   * This is the main optimisation function and the heart of the GenEvA library.
   * Everytime it is called, the number of generations is resetted. The recombination
   * scheme, type of child mutations and the selection scheme are determined in
   * other functions, namely GBasePopulation::recombine(), GBasePopulation::mutateChildren()
   * and GBasePopulation::select() (or overloaded versions in derived classes).
   *
   * A possible improvement for this function (and the function it calls) is to
   * create temporary copies of the variables used so that accidental calls to
   * modifying functions by the user cannot interfer with the calculation.
   */
  void GBasePopulation::optimize(void){
    // Reset the generation
    GBasePopulation::resetGeneration();

    // Fill up the population as needed
    GBasePopulation::adjustPopulationSize();

    // Inform all GMember objects in this population that they are not at the
    // top of the hierarchy.
    GManagedMemberCollection::setIsNotRootExcl();

    // Initialize the start time with the current time. Uses Boost::date_time
    startTime_ = boost::posix_time::second_clock::local_time(); // millisecond or second ???

    do{
      this->recombine();			 // create new children from parents
      this->mutateChildren(); 	 // mutate children and calculate their value
      this->select();  		 	 // sort children according to their fitness

      // We want to provide feedback to the user in regular intervals.
      // Set the reportGeneration_ variable to 0 in order not to emit
      // any information.
      if(getReportGeneration() && (getGeneration()%getReportGeneration() == 0)) doInfo();

      incrGeneration();    // update the generation_ counter
    }
    while(!halt());         // allows custom halt criteria
  }

  /***********************************************************************************/
  /**
   * This function emits information about the state of the population. Note that
   * information about the members should be emitted in the parent class
   * GManagedMemberCollection . We do not want to accidently trigger fitness recalculation
   * (i.e. optimization) for this class. Hence we only emit information about the internal
   * fitness and warn if the dirty flag is set.
   *
   * \param indention The number of white spaces in front of each output line
   * \return A string containing a report about the inner state of the object
   */
  string GBasePopulation::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GBasePopulation: " << this << endl
	<< ws(indention) << "in generation / maxGeneration = " << getGeneration() << " / " << maxGeneration_ << endl
	<< ws(indention) << "with nParents_ = " << nParents_ << " and nChildren_ = " << getNChildren() << endl
	<< ws(indention) << "defaultChildren_ = " << defaultChildren_ << endl
	<< ws(indention) << "and recombinationMethod_ = " << recombinationMethod_ << endl
	<< ws(indention) << "muplusnu_ = " << (muplusnu_?"MUPLUSNU":"MUCOMMANU") << ", " <<  "maximize_ = " << maximize_ << endl
	<< ws(indention) << "id_ = " << id_ << " and firstId_ = " << firstId_ << endl
	<< ws(indention) << "maxDuration_ = " << maxDuration_ << endl
	<< ws(indention) << "quality is " << this->getMyCurrentFitness() << endl;
    if(this->isDirty()){
      oss << ws(indention) << "Note that the dirty flag is set for this object!" << endl;
    }

    oss << ws(indention) << "-----> Report from parent class GManagedMemberCollection:" << endl
	<< GManagedMemberCollection::assembleReport(indention+NINDENTION) << endl;
    return oss.str();
  }

  /***********************************************************************************/
  /**
   * Emits information specific to this population. The function can be overloaded
   * in derived classes. By default we allow the user to register a call-back function
   * using GBasePopulation::registerInfoFunction() . Please note that it is not
   * possible to serialize this function, so it will only be active on the host were
   * it was registered, but not on remote systems.
   */

  void GBasePopulation::doInfo(void){
    if(!infoFunction_.empty()) infoFunction_(this);
  }

  /***********************************************************************************/
  /**
   * The user can specify what information should be emitted in a call-back function
   * that is registered in the setup phase. This functionality is based on boost::function .
   */
  void GBasePopulation::registerInfoFunction(boost::function<void (GBasePopulation * const)> infoFunction){
    infoFunction_ = infoFunction;
  }

  /***********************************************************************************/
  /**
   * Sets the number of generations after which the population should
   * report about its inner state.
   *
   * \param reportGeneration The number of generations after which information should be emitted
   */
  void GBasePopulation::setReportGeneration(uint32_t reportGeneration){
    reportGeneration_ = reportGeneration;
  }

  /***********************************************************************************/
  /**
   * Returns the number of generations after which the population should
   * report about its inner state.
   *
   * \return The number of generation after which reporting should be done
   */
  uint32_t GBasePopulation::getReportGeneration(void) const{
    return reportGeneration_;
  }

  /***********************************************************************************/
  /**
   * Specifies the initial size of the population plus the number of parents.
   * The population will be filled with additional individuals in
   * GBasePopulation::optimize() later, as required -- see
   * GBasePopulation::adjustPopulationSize() . Also, all error checking is done in
   * that function.
   *
   * \param popSize The desired size of the population
   * \param nParents The desired number of parents
   */
  void GBasePopulation::setPopulationSize(uint16_t popSize, uint16_t nParents){
    popSize_ = popSize;
    nParents_ = nParents;
  }

  /***********************************************************************************/
  /**
   * This is the last action to be called before the optimization cycle. The
   * function checks that the population size meets the requirements and resizes
   * the population to the apropriate size, if required. An obvious precondition
   * is that at least one individial (aka a GMember object) has been added to
   * the population. It is interpreted as a parent and serves as the template for
   * missing individuals (children and parents). Parents that have already
   * been added will not be replaced. This is one of the few occasions where
   * popSize_ is used directly. In most occasions we refer to the size of the
   * vector instead to allow short-term adjustments of the vector size.
   */
  void GBasePopulation::adjustPopulationSize(void){
    uint16_t np = getNParents();

    // First check that we have been given suitable values for population size
    // and number of parents

    // 1. Have the population size and number of parents been set at all ?
    if(popSize_ == 0 || np == 0){
      GException ge;
      ge << "In GBasePopulation::adjustPopulationSize() : Error!" << endl
	 << "The population size and/or the number of parents is set to 0 !" << endl
	 << "Did you call GBasePopulation::setPopulationSize() ?" << endl
	 << "population size = " << popSize_ << endl
	 << "number of parents = " << np << endl
	 << raiseException;
    }

    // 2. In MUCOMMANU mode we want to have at least as many children as parents,
    // whereas MUPLUSNU only requires the population size to be larger than the
    // number of parents.
    if((getSortingScheme()==MUCOMMANU && (popSize_ < 2*np)) ||
       (getSortingScheme()==MUPLUSNU && popSize_<=np))
      {
	GException ge;
	ge << "In GBasePopulation::adjustPopulationSize() : Error!" << endl
	   << "Requested size of population is too small :" << popSize_ << " " << np << endl
	   << "Sorting scheme is " << (getSortingScheme()==MUCOMMANU?"MUCOMMANU":"MUPLUSNU") << endl
	   << raiseException;
      }

    // Check how many individuals have been added already.
    // At least one is required.
    uint16_t mySize = this->size();
    if(mySize == 0){
      GException ge;
      ge << "In GBasePopulation::adjustPopulationSize() : Error!" << endl
	 << "size of population is 0. Did you add any individuals?" << endl
	 << raiseException;
    }

    // Do the smart pointers actually point to any objects ?
    vector<boost::shared_ptr<GMember> >::iterator it;
    for(it=this->begin(); it!=this->end(); ++it){
      if(!(*it)){ // shared_ptr can be implicitly converted to bool
	GException ge;
	ge << "In GBasePopulation::adjustPopulationSize() : Error!" << endl
	   << "Found empty smart pointer." << endl
	   << raiseException;
      }
    }

    // Fill up as required. We are now sure we have a suitable number of members
    if(mySize == popSize_) return; // Nothing to do
    else if(mySize > popSize_){ // Somehow we ended up with too many individuals
      // Log so the user doesn't add too many members next time
      gls << "In GBasePopulation::adjustPopulationSize() :" << endl
	  << "Too many individuals present. Resizing." << endl
	  << "desired size = " << popSize_ << "; actual size = " << mySize << endl
	  << logLevel(UNCRITICAL);
      // get rid of surplus members
      this->resize(popSize_);
      return;
    }
    else{ // We need to add members
      // Missing members are created as copies of the population's first individual
      for(uint16_t i=0; i<(popSize_-mySize); i++){
	GMember *gm = dynamic_cast<GMember *>(this->at(0)->clone());
	if(gm){ // did the conversion work ?
	  shared_ptr<GMember> p(gm);
	  this->push_back(p);
	}
	else{
	  GException ge;
	  ge << "In GBasePopulation::adjustPopulationSize() : conversion error!" << endl
	     << raiseException;
	}
      }
    }

    // Let parents know they are parents and children that they are children
    markParents();

    // Make sure derived classes (such as GTransferPopulation) have a way of finding out
    // what the default number of children is. This is particularly important, if - in a
    // network environment, some individuals might not return. In this case we need to
    // fix the population.
    setDefaultChildren(GBasePopulation::getNChildren());
  }

  /***********************************************************************************/
  /**
   * This is a little helper function that tries to give this object a unique id.
   */
  void GBasePopulation::setId(void){
    // create a unique id from our memory location
    try{
      id_ = lexical_cast<string>(this);
    }
    catch (bad_lexical_cast &) {
      GException ge;
      ge << "In  GTransferPopulation::setId() : Error!" << endl
	 << "Bad cast. This should never happen." << endl
	 << raiseException;
    }
  }


  /***********************************************************************************/
  /**
   * Retrieves the id of this object. If this is the first time the function
   * has been called, we additionally create the id.
   *
   * \return The value of the id_ variable
   */
  string GBasePopulation::getId(void){
    if(firstId_){
      this->setId();
      firstId_=false;
    }

    return id_;
  }

  /***********************************************************************************/
  /**
   * Retrieves the size of the population. Note that the popSize_ parameter set in
   * setPopulationSize() is only needed in the setup phase. In all other occasions
   * the population size is derived from the size of the vector.
   *
   * \return The current size of the population
   */
  uint16_t GBasePopulation::getPopulationSize(void) const{
    return this->size();
  }


  /***********************************************************************************/
  /**
   * Retrieve the number of parents as set by the user. This is a fixed parameter and
   * should not be changed after it has first been set.
   *
   * \return The number of parents in the population
   */
  uint16_t GBasePopulation::getNParents(void) const {
    return nParents_;
  }

  /***********************************************************************************/
  /**
   * Calculates the number of children from the number of parents and the
   * size of the vector.
   *
   * \return The number of children in the population
   */
  uint16_t GBasePopulation::getNChildren(void) const {
    return getPopulationSize() - getNParents();
  }

  /***********************************************************************************/
  /**
   * Sets the sorting scheme to MUPLUSNU (new parents will be selected from the entire
   * population, including the old parents) or MUCOMMANU (new parents will be selected
   * from children only).
   *
   * \param sc The desired sorting scheme
   */
  void GBasePopulation::setSortingScheme(bool muplusnu) {
    muplusnu_=muplusnu;
  }

  /***********************************************************************************/
  /**
   * Retrieves information about the current sorting scheme (see the
   * documentation of GBasePopulation::setSortingScheme() for further
   * information).
   *
   * \return The current sorting scheme
   */
  bool GBasePopulation::getSortingScheme(void) const {
    return muplusnu_;
  }

  /***********************************************************************************/
  /**
   * Sets the maximum number of generations allowed for an optimization run. Set
   * to 0 in order for this stop criterium to be disabled.
   *
   * \param The maximum number of allowed generations
   */
  void GBasePopulation::setMaxGeneration(uint32_t maxGeneration) {
    maxGeneration_ = maxGeneration;
  }

  /***********************************************************************************/
  /**
   * Retrieves the maximum number of generations allowed in an optimization run.
   *
   * \return The maximum number of generations
   */
  uint32_t GBasePopulation::getMaxGeneration(void) const {
    return maxGeneration_;
  }

  /***********************************************************************************/
  /**
   * Retrieves the currently active generation number
   *
   * \return The currently active generation
   */
  const uint32_t GBasePopulation::getGeneration(void) const {
    return generation_;
  }

  /***********************************************************************************/
  /**
   * Sets the maximum allowed processing time in days, hours, minutes, seconds . In order to
   * terminate the optimization after 3 days, 15 hours and 12 seconds, you would call
   * setMaxTime(3,15,0,12) .Entries for hours, minutes and seconds may also be larger than
   * 23 and 59 respectively.
   *
   * \param days    The number of    days allowed for this optimization
   * \param hours   The number of   hours allowed for this optimization
   * \param minutes The number of minutes allowed for this optimization
   * \param seconds The number of seconds allowed for this optimization
   * \return        The total requested processing time in seconds
   */
  int32_t GBasePopulation::setMaxTime(int32_t d, int32_t h, int32_t m, int32_t s){
    using namespace boost::posix_time;
    maxDuration_ = hours(d*24 + h) + minutes(m) + seconds(s);
    return getMaxTime();
  }

  /***********************************************************************************/
  /**
   * Retrieves the maximum allowed processing time in seconds.
   *
   * \return The maximum allowed processing time in seconds
   */
  uint32_t GBasePopulation::getMaxTime(void){
    return maxDuration_.total_seconds();
  }

  /***********************************************************************************/
  /**
   * This function emits a boolean once a given time, set with GBasePopulation::setMaxTime()
   * has passed. It is used in the GBasePopulation::halt() function.
   *
   * \return A boolean indicating whether a given amount of time has passed
   */
  bool GBasePopulation::timedHalt(void){
    using namespace boost::posix_time;
    ptime currentTime = second_clock::local_time();
    if((currentTime - startTime_) >= maxDuration_) return true;
    return false;
  }

  /***********************************************************************************/
  /**
   * Resets the generation counter.
   */
  void GBasePopulation::resetGeneration(void){
    GBasePopulation::setGeneration(0);
  }

  /***********************************************************************************/
  /**
   * Lets the user specify whether he wants to perform maximization or minimization.
   *
   * \param maximize Indicates whether we want to maximize (true) or minimize (false)
   */
  void GBasePopulation::setMaximize(bool maximize){
    maximize_ = maximize;
  }

  /***********************************************************************************/
  /**
   * Retrieves the maximize_ parameter. It indicates whether we are maximizing (true)
   * or minimizing (false).
   *
   * \return The value of the maximize_ parameter
   */
  bool GBasePopulation::getMaximize(void) const{
    return maximize_;
  }

  /***********************************************************************************/
  /**
   * It is possible for users to specifiy in overloaded versions of this
   * function under which conditions the optimization should be stopped. The
   * function is called from GBasePopulation::halt .
   *
   * \return boolean indicating that a stop condition was reached
   */
  bool GBasePopulation::customHalt(void) {
    /* nothing - specify your own criteria in derived classes */
    return false;
  }

  /***********************************************************************************/
  /**
   * This function assigns a new value to each child individual
   * according to the chosen recombination scheme. It can be
   * overloaded by a user in order to implement his own recombination
   * scheme.
   */
  void GBasePopulation::customRecombine(void){
    uint16_t i, np = getNParents(), mySize = this->size();

    // for all children do ...
    for(i=np; i<mySize; i++) {
      // A recombination taking into account the value does not make
      // sense in generation 0, as parents might not have a suitable
      // value. Instead, this function might accidently trigger value
      // calculation in this case. Hence we fall back to random
      // recombination in generation 0. No value calculation takes
      // place there.
      if(getGeneration() == 0){
	randomRecombine(i);
	continue;
      }

      switch(getRecombinationMethod()){
      case DEFAULTRECOMBINE: // we want the RANDOMRECOMBINE behaviour
      case RANDOMRECOMBINE:
	randomRecombine(i);
	break;
      case VALUERECOMBINE:
	valueRecombine(i);
	break;
      default:
	GException ge;
	ge << "In GBasePopulation::customRecombine() : Error!" << endl
	   << "Bad recombination method " << getRecombinationMethod() << endl << raiseException;
      }
    }
  }

  /***********************************************************************************/
  /**
   * This function implements the RANDOMRECOMBINE scheme. This functions uses BOOST's
   * numeric_cast function for safe conversion between int and uint16_t.
   *
   * \param pos The position of the individual for which a new value should be chosen
   */
  void GBasePopulation::randomRecombine(uint16_t pos){
    uint16_t np = getNParents();

    // Sanity check: Is the number of parents at least 1 ?
    if(np == 0){
      GException ge;
      ge << "In GBasePopulation::randomRecombine(): Error!" << endl
	 << "Function called with 0 parents" << endl
	 << raiseException;
    }

    uint16_t p_pos;

    // Choose a parent to be used for the recombination
    try{
      p_pos = numeric_cast<uint16_t>(gr.discreteRandom(numeric_cast<int16_t>(np)));
    }
    catch(boost::bad_numeric_cast& e){
      GException ge;
      ge << "In GBasePopulation::randomRecombine(): Conversion from int to" << endl
	 << "uint16_t failed with message " << endl
	 << e.what() << endl
	 << raiseException;
    }

    this->at(pos)->load((this->at(p_pos)).get());
  }

  /***********************************************************************************/
  /**
   * This function implements the VALUERECOMBINE scheme. We divide the range [0.,1.[
   * into nParents_ sub-areas with different size (the largest for the first parent,
   * the smallest for the last). Parents are chosen for recombination according to a
   * random number evenly distributed between 0 and 1. This way parents with higher
   * fitness are more likely to be chosen for recombination. We need to make sure that
   * the sum of all nParents_ areas is 1.
   *
   * \param pos The child individual for which a parent should be chosen
   */
  void GBasePopulation::valueRecombine(uint16_t pos){
    uint16_t i, np = getNParents();
    bool done=false;

    // Sanity check: Is the number of parents at least 1 ?
    if(np == 0){
      GException ge;
      ge << "In GBasePopulation::valueRecombine(): Error!" << endl
	 << "Function called with 0 parents" << endl
	 << raiseException;
    }
    // This function only makes sense if we have at least 2 parents
    // We do the recombination manually if we only have one parent.
    else if(np==1){
      this->at(pos)->load((this->at(0)).get());
      return;
    }
    else{ // We have more than one parent
      // Calculate the individual thresholds
      vector<double> threshold(np);
      double thresholdSum=0.;
      for(i=0; i<np; i++) {
	thresholdSum += 1./(static_cast<double>(i)+2.);
      }
      for(i=0; i<np-1; i++) {
    	  // norming the sum to 1
    	  threshold[i] = (1./(static_cast<double>(i)+2.)) / thresholdSum;
		  // Make sure the subsequent range is in the right position
		  if(i>0) threshold[i] += threshold[i-1];
      }
      threshold[np-1] = 1.;

      // get the test value
      double randTest = gr.evenRandom();

      for(i=0; i<np; i++){
	if(randTest<threshold[i]){
	  this->at(pos)->load((this->at(i)).get());
	  done = true;
	  break;
	}
      }

      if(!done){
	GException ge;
	ge << "In GBasePopulation::valueRecombine : Error!" << endl
	   << "Failed to perform recombination : " << endl
	   << randTest << " " << threshold[np-1] << endl
	   << raiseException;
      }
    }
  }

  /***********************************************************************************/
  /**
   * This function is called from GBasePopulation::optimize() and performs the
   * actual recombination, based on the recombination schemes defined by the user.
   * This function assumes that the population already has the desired size.
   */
  void GBasePopulation::recombine(void)
  {
    uint16_t nParents = getNParents();
    uint32_t generation = getGeneration();
    vector<boost::shared_ptr<GMember> >::iterator it;

    // Let parents know which generation they are in and that they
    // are parents.
    for(it=this->begin(); it!=this->begin()+nParents; ++it){
      (*it)->setParentPopGeneration(generation);
      (*it)->setIsParent(true);
    }

    // Do the actual recombination
    this->customRecombine();

    // Let children know they are children and what generation
    // they are in.
    for(it=this->begin()+nParents; it!=this->end(); ++it){
      (*it)->setParentPopGeneration(generation);
      (*it)->setIsParent(false);
    }
  }

  /***********************************************************************************/
  /**
   * Mutate all children in sequence. Notet that this also triggers their value
   * calculation, so this function needs to be overloaded for optimization in a
   * network context.
   */
  void GBasePopulation::mutateChildren(void)
  {
    uint16_t nParents = getNParents();
    vector<boost::shared_ptr<GMember> >::iterator it;

    // We need to make sure that fitness calculation is
    // triggered for all parents. As either the user
    // has supplied all parents or they are mutated clones
    // of a single parent (created through setNParents() ),
    // we can assume that all parents are different.
    if(this->getGeneration() == 0){
      for(it=this->begin(); it!=this->begin()+nParents; ++it){
	(*it)->fitness();
      }
    }

    // Next we perform the mutation of each child individual in
    // sequence. Note that this can also trigger fitness calculation.
    for(it=this->begin()+nParents; it!=this->end(); ++it){
      (*it)->mutate();
    }
  }

  /***********************************************************************************/
  /**
   * Choose new parents, based on the recombination scheme set by the user.
   * This function uses Boost.bind and Boost.function (see http://www.boost.org).
   * The function objects used for maximization and minimization are defined
   * in the constructor.
   */
  void GBasePopulation::select(void)
  {
    uint16_t np=getNParents();
    vector<boost::shared_ptr<GMember> >::iterator it_begin, it_end;

    // Find suitable start and end-points
    if(getSortingScheme() == MUPLUSNU){
      it_begin = this->begin();
    }
    else{ // MUCOMMANU
      it_begin = this->begin() + np;
    }
    it_end = this->end();

    // Sort the arrays. Note that we are using boost::function
    // objects here, that have been "loaded" with function objects
    // in the default constructor
    if(getMaximize()) sort(it_begin, it_end, f_max_);
    else sort(it_begin, it_end, f_min_);

    // Move the parents' region to the end of the range if
    // this is the MUCOMMANU case
    if(getSortingScheme() == MUCOMMANU)
      swap_ranges(this->begin(),this->begin()+np,this->begin()+np);

    // Tell parents and children about their status
    markParents();
  }

  /***********************************************************************************/
  /**
   * Possible mutations of a population could involve shifting of individuals.
   * By default, no mutation is defined.
   */
  void GBasePopulation::customMutate(void)
  { /* nothing */ }

  /***********************************************************************************/
  /**
   * Fitness calculation for a population means optimization. The fitness is then determined
   * by the best individual which, after the end of the optimization cycle, can be found in
   * the first position of the array. This is true both for MUPLUSNU and MUCOMMANU sorting
   * mode.
   *
   * \return The fitness of the best individual in the population
   */
  double GBasePopulation::customFitness(void){
    this->optimize();

    double val = this->at(0)->getMyCurrentFitness();
    // is this the current fitness ?
    if(this->at(0)->isDirty()){
      gls << "In GBasePopulation::customFitness(): Warning:" << endl
	  << "Fitness belongs to a component whose dirtyFlag has been set." << endl
	  << logLevel(CRITICAL);
    }
    return val;
  }

  /***********************************************************************************/
  /**
   * This helper function marks parents as parents and children as children.
   */
  void GBasePopulation::markParents(void){
    uint16_t np = getNParents();

    vector<shared_ptr<GMember> >::iterator it;
    for(it=this->begin(); it!=this->begin()+np; ++it){
      (*it)->setIsParent(true);
    }

    for(it=this->begin()+np; it!=this->end(); ++it){
      (*it)->setIsParent(false);
    }
  }

  /******************************************************************************/
  /**
   * Sets the defaultChildren_ parameter. E.g. in GTransferPopulation::mutateChildren() ,
   * this factor controls when a population is considered to be complete. The corresponding
   * loop which waits for new arrivals will then be stopped, which in turn allows
   * a new generation to start. This is a private function which is called from
   * GBasePopulation::optimize() .
   *
   * \param defaultChildren The number of children after which the population is considered to be complete
   */
  void GBasePopulation::setDefaultChildren(uint16_t defaultChildren){
    defaultChildren_=defaultChildren;
  }

  /******************************************************************************/
  /**
   * Retrieves the defaultChildren_ parameter. See GBasePopulation::setDefaultChildren()
   * for an explanation of this parameter.
   *
   * \return The defaultChildren_ parameter
   */
  uint16_t GBasePopulation::getDefaultChildren(void) const {
    return defaultChildren_;
  }

  /***********************************************************************************/
  /**
   * A little helper function that increases the generation counter by 1.
   */
  void GBasePopulation::incrGeneration(void){
    generation_++;
  }

  /***********************************************************************************/
  /**
   * A private helper function used to set the generation counter to
   * a given value.
   *
   * \param generation The desired new value for the generation counter
   */
  void GBasePopulation::setGeneration(uint32_t generation){
    generation_ = generation;
  }

  /***********************************************************************************/
  /**
   * This function checks whether a halt criterium has been reached. The most
   * common criterium is the maximum number of generations. Set the maxGeneration_
   * counter to 0 if you want to disable this criterium.
   *
   * \return A boolean indicating whether a halt criterium has been reached
   */
  bool GBasePopulation::halt(void)
  {
    // Have we exceeded the maximum number of generations and
    // do we indeed intend to stop in this case ?
    if(getMaxGeneration() && (getGeneration() > getMaxGeneration()))
      return true;

    // Do we have a scheduled halt time ?
    if(maxDuration_.total_seconds())
      if(timedHalt()) return true;

    // Has the user specified an additional break condition ?
    if(customHalt()) return true;

    // Fine, we can continue.
    return false;
  }

  /***********************************************************************************/
  /**
   * Lets the user set the desired recombination method. A sanity check is done
   * so we are sure to be using a valid method.
   *
   * \param recombinationMethod The desired recombination method
   */
  void GBasePopulation::setRecombinationMethod(int8_t recombinationMethod){
    switch(recombinationMethod){
    case DEFAULTRECOMBINE: break;
    case RANDOMRECOMBINE: break;
    case VALUERECOMBINE: break;
    default:
      GException ge;
      ge << "In GBasePopulation::setRecombinationMethod() : Error!" << endl
	 << "Bad recombination method " << recombinationMethod << endl
	 << raiseException;
    }

    recombinationMethod_ = recombinationMethod;
  }

  /***********************************************************************************/
  /**
   * Retrieves the value of the recombinationMethod_ variable
   *
   * \return The value of the recombinationMethod_ variable
   */
  int8_t GBasePopulation::getRecombinationMethod(void) const{
    return recombinationMethod_;
  }

  /***********************************************************************************/

} /* namespace GenEvA */
