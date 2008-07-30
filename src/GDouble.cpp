/**
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

#include "GDouble.hpp"

/** 
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(GenEvA::GDouble)

namespace GenEvA
{

  /******************************************************************************/
  /**
   * The default constructor. As this class uses the adaptor scheme
   * (see GTemplateAdaptor<T>), you will need to add your own adaptors,
   * such as the GDoubleGaussAdaptor. 
   */
  GDouble::GDouble() :GTemplateValue<double>(0.) {
    // Initialize randomly within the allowed value range.
    // Also checks whether the supplied value is inside the allowed range
    this->setExternalValue(gr.evenRandom(range_.lowerBoundary(),range_.upperBoundary()));
  }

  /******************************************************************************/
  /**
   * Initialize with a given double value. Note that this constructor
   * can also be used for conversion. GTemplateValue<double> is initialized
   * with 0, as that class holds the internal representation. The correct
   * internal value will be set by GDouble::setExternalValue() . The
   * function purposely allows conversions from double values.
   * 
   * \param val Initialisation value
   */
  GDouble::GDouble(double val) :GTemplateValue<double>(0.) {
    this->setExternalValue(val);
  }

  /******************************************************************************/
  /**
   * A standard copy constructor. Most work is done by the parent
   * classes, we only need to copy ranges and gaps.
   * 
   * \param cp Another GDouble object
   */
  GDouble::GDouble(const GDouble& cp) :GTemplateValue<double>(cp){
    // Grange has an operator=
    range_ = cp.range_;
    // this part is complicated. Use a helper function
    copyGaps(cp.gps_);
  }

  /******************************************************************************/
  /**
   * A standard destructor. No local, dynamically allocated data, 
   * hence nothing to do. Note that the pointers to GRange objects
   * stored in the gps_ vector will be cleared automatically, as they
   * are wrapped into a boost::shared_ptr<> object. 
   */
  GDouble::~GDouble()
  { /* nothing */ }

  /******************************************************************************/
  /**
   * A standard assignment operator for GDouble.
   * 
   * \param cp A constant reference to another GDouble object
   * \return A constant reference to this object
   */
  const GDouble& GDouble::operator=(const GDouble& cp){
    GDouble::load(&cp);
    return *this;
  }

  /******************************************************************************/
  /**
   * A standard assignment operator for GObject objects
   * 
   * \param cp Another GDouble object camouflaged as a GObject
   * \return A constant reference to this object, camouflaged as a GObject
   */
  const GObject& GDouble::operator=(const GObject& cp){
    GDouble::load(&cp);
    return *this;
  }

  /******************************************************************************/
  /**
   * A standard assignment operator for double values
   * 
   * \param The desired new external value
   * \return The new external value of this object
   */
  double GDouble::operator=(double val){
    this->setExternalValue(val);
    return this->fitness();
  }

  /******************************************************************************/
  /**
   * Resets the object to its initial state.
   */
  void GDouble::reset(void){
    // Reset the local data
    range_.reset();
    gps_.clear();
    // Reset the parent class
    GTemplateValue<double>::reset();
  }
	
  /******************************************************************************/
  /**
   * Loads the data of another GDouble, camouflaged as a GObject
   * into this object.
   * 
   * \param gb Another GDouble object, camouflaged as a GObject
   */
  void GDouble::load(const GObject *gb){
    // Check that this object is not accidently assigned to itself. 
    // As we do not actually do any calls with this pointer, we
    // can use the faster static_cast<>
    if(static_cast<const GDouble *>(gb) == this){
      GException ge;
      ge << "In GDouble::load(): Error!" << endl
	 << "Tried to assign an object to itself." << endl
	 << raiseException;
    }
	
    GTemplateValue<double>::load(gb);

    const GDouble *gdptr = dynamic_cast<const GDouble *>(gb);
    if(!gdptr){
      GException ge;
      ge << "In GDouble::load() (1.) : Bad Conversion!" << endl
	 << raiseException;
    }

    range_ = gdptr->range_;

    // this part is complicated. Use a helper function
    copyGaps(gdptr->gps_);
  }

  /******************************************************************************/
  /**
   * Create a deep copy of this object. Basically this is a fabric function.
   * 
   * \return The newly generated GDouble
   */
  GObject *GDouble::clone(void){
    return new GDouble(*this);
  }

  /******************************************************************************/
  /**
   * Sets the upper and lower boundaries of this object. Boundaries
   * can be open or closed.
   * 
   * \param lbnd The new lower boundary
   * \param lopn A boolean indicating whether or not lbnd is open
   * \param ubnd The new upper boundary
   * \param uopn A boolean indicating whether or not ubnd is open
   */
  void GDouble::setBoundaries(double lbnd, bool lopn, double ubnd, bool uopn){
    // save current external value;
    double val = this->fitness();
	
    // set the boundaries as required
    range_.setBoundaries(lbnd,lopn,ubnd,uopn);
	
    // Determine a suitable new external value
    if(val < range_.lowerBoundary()) val = range_.lowerBoundary();
    else if(val > range_.upperBoundary()) val = range_.upperBoundary();
	
    // set the external value to the original value (if possible). 
    // Note that in the presence of boundaries the internal representation
    // will be re-calculated.
    setExternalValue(val);
  }

  /******************************************************************************/
  /**
   * A similar function to GDouble::setBoundaries(double, bool, double, bool), 
   * but assumes that the boundaries are closed.
   * 
   * \param lbnd The new lower boundary
   * \param ubnd The new upper boundary
   */
  void GDouble::setBoundaries(double lbnd, double ubnd){
    this->setBoundaries(lbnd,false,ubnd,false);
  }

  /******************************************************************************/
  /**
   * Adds a gap to the value range_ of this object. Gaps can have open 
   * or closed boundaries. As gaps act as helper classes, there is no
   * function that allows to add a gap object. Users should not be
   * required to know about the internal implementation of this class.
   * 
   * \param lbnd The new lower boundary
   * \param lopn A boolean indicating whether or not lbnd is open
   * \param ubnd The new upper boundary
   * \param uopn A boolean indicating whether or not ubnd is open   
   */
  void GDouble::addGap(double lbnd, bool lopn, double ubnd, bool uopn){
    // create and add range
    GRange *grange = new GRange(lbnd,lopn,ubnd,uopn);
    shared_ptr<GRange> p(grange);
    gps_.push_back(p);

    // sort gaps
    sortGaps();

    // check that nothing overlaps
    if(!checkRanges()){
      GException ge;
      ge << "In GDouble::addGap() : Error!" << endl
	 << "Range seems to overlap another" << endl
	 << raiseException;
    }	
  }

  /******************************************************************************/
  /**
   * A similar function to GDouble::addGap(double, bool, double, bool), but
   * assumes that both boundaries of the gap are closed.
   * 
   * \param lbnd The new lower boundary
   * \param ubnd The new upper boundary
   */
  void GDouble::addGap(double lbnd, double ubnd){
    this->addGap(lbnd,false,ubnd,false);
  }

  /******************************************************************************/
  /**
   * The standard way to retrieve the external value of a class
   * derived from GTemplateValue<T> is the getValue() function.
   * In the case of a GDouble the external value is different from
   * the internal value and needs to be recalculated after every
   * mutation. Hence we need to overload this function from the
   * default version, which just returns the internal value.
   */
  double GDouble::getValue(void){
    return this->fitness();
  }

  /******************************************************************************/
  /**
   * Sets the internal value in such a way that the user-visible
   * value is set to "val". GDouble performs a linear transformation
   * from inner to outer value in the areas where no gaps are present.
   * A gap introduces a difference equal to its width between the 
   * inner and outer representation.
   * 
   * \param val The desired new external value
   * \return The original value
   */
  double GDouble::setExternalValue(double val){
    double result, gapcounter, sumgaps;
    double low, up, lov=val;

    vector<shared_ptr<GRange> >::iterator it;

    low = range_.lowerBoundary();
    up  = range_.upperBoundary();

    // check that the value is inside the allowed range.
    // Adapt if necessary ...
    if(lov > up) lov=up;
    else if(lov < low) lov=low;

    // no gaps ? result is just f(x)=x
    if(gps_.size()==0){
      result=lov;
    }
    else {
      // The overall sum of gap-widths
      sumgaps=0.;
      for(it=gps_.begin(); it!=gps_.end(); ++it) sumgaps += (*it)->width();
    	
      // find out, where we are with respect to the gaps
    	
      // we are below the lowest gap
      if(lov< gps_.front()->lowerBoundary()) result=lov;
      // we are inside the lowest gap if the following is true
      else if(lov>=gps_.front()->lowerBoundary() && lov<=gps_.front()->upperBoundary()){
	result=gps_.front()->lowerBoundary();
      }
      else if(lov>gps_.back()->upperBoundary()){ // above the uppermost gap
	result=lov-sumgaps;
      }
      else{ // somewhere above the lowest upper-gap and below the highest upper gap boundary
	gapcounter=0;
	result=0.;
      		
	for(it=gps_.begin()+1; it!=gps_.end(); ++it){
	  // if we are in the range of a gap, return
	  // the y-value of the lower boundary
	  gapcounter += (*(it-1))->width();
	  if(lov>=(*it)->lowerBoundary() && lov<=(*it)->upperBoundary()){
	    result=(*it)->lowerBoundary() - gapcounter;
	    break;
	  }
	  else if(lov>(*(it-1))->upperBoundary() && lov<(*it)->lowerBoundary()){ // between gaps
	    result=lov-gapcounter;
	    break;
	  }
	}
      }
    }

    // Ready to set the internal value ...
    GTemplateValue<double>::setInternalValue(result);
  	
    // We have changed the internal state of this object and hence need 
    // to mark it as dirty. Note that evaluation will only take place when
    // the fitness() function is called (either directly, or through the 
    // getValue() wrapper).
    setDirtyFlag();
  	
    // Return the original value for reference
    return lov;
  }

  /******************************************************************************/
  /**
   * This function allows automatic conversion from GDouble to GDouble. This
   * allows us to define only few operators, as the bulk of the work will be 
   * done by automatic conversions done by the C++ compiler.
   */
  GDouble::operator double (){
    return this->fitness();	
  }

  /******************************************************************************/
  /** 
   * A comparison operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value to be compared with this object
   * \return A boolean indicating whether val is larger than this object
   */
  bool GDouble::operator<(GDouble& val){
    return this->fitness() < val.fitness();
  }

  /******************************************************************************/
  /** 
   * A comparison operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value to be compared with this object
   * \return A boolean indicating equivalence
   */
  bool GDouble::operator==(GDouble& val){
    return this->fitness() == val.fitness();
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value to be added to this object
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator+=(GDouble& val){
    this->setExternalValue(this->fitness()+val.fitness());
    return *this;
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value to be subtracted from this object
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator-=(GDouble& val){
    this->setExternalValue(this->fitness()-val.fitness());
    return *this;	
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value to be multiplied with this object
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator*=(GDouble& val){
    this->setExternalValue(this->fitness() * val.fitness());
    return *this;		
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value by which this object should be devided
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator/=(GDouble& val){
    double divisor=val.fitness();
    if(divisor==0.){
      GException ge;
      ge << "In GDouble::operator/=() : Attempted division by 0." << endl
	 << raiseException;
    }
	
    setExternalValue(this->fitness()/divisor);
    return *this;
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value used for assignment
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator%=(GDouble& val){
    this->setExternalValue(double(int32_t(this->fitness()) % int32_t(val.fitness())));
    return *this;		
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value used for assignment
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator|=(GDouble& val){
    this->setExternalValue(double(int32_t(this->fitness()) | int32_t(val.fitness())));
    return *this;	
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value used for assignment
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator&=(GDouble& val){
    this->setExternalValue(double(int32_t(this->fitness()) & int32_t(val.fitness())));
    return *this;		
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \param  val The value used for assignment
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator^=(GDouble& val){
    this->setExternalValue(double(int32_t(this->fitness()) ^ int32_t(val.fitness())));
    return *this;	
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator++(){
    this->setExternalValue(this->fitness() + 1.);
    return *this;	
  }

  /******************************************************************************/
  /** 
   * An assignment operator, which aids Boost.operators in the definition of
   * arithmentic operators for calculations performed with this class.
   * 
   * \return A constant reference to this object
   */
  GDouble& GDouble::operator--(){
    this->setExternalValue(this->fitness() - 1.);
    return *this;		
  }

  /******************************************************************************/
  /**
   * This function assembles a report about the classes' internal state and then calls the 
   * parent classes' assembleReport() function.
   * 
   * \param indention The number of white spaces in front of each output line 
   * \return A string containing a report about the inner state of the object
   */
  string GDouble::assembleReport(uint16_t indention) const
  {
    ostringstream oss;
    oss << ws(indention) << "GDouble: " << this << endl
	<< ws(indention) << "-----> Report from value range:" << endl
	<< range_.assembleReport(indention+NINDENTION) << endl;

    vector<shared_ptr<GRange> >::const_iterator it;
    uint16_t counter = 0;
    for(it=gps_.begin(); it!=gps_.end(); ++it){
      oss << ws(indention) << "-----> Report from Gap " << counter++ << ": " << endl
	  << (*it)->assembleReport(indention+NINDENTION) << endl;
    }
    oss << ws(indention) << "-----> Report from parent class GTemplateValue<double> : " << endl
	<< GTemplateValue<double>::assembleReport(indention+NINDENTION) << endl;

    return oss.str();
  }

  /******************************************************************************/
  /**
   * This function implements the mapping from internal to external
   * value.
   * 
   * \return The user-visible value
   * 
   * \todo Inefficient. The first part of the function will always
   * be calculated, although this is really only necessary during
   * changes of gps_ or range_ . Also, if gps_size() == 0, we 
   * nevertheless follow all the code.
   */
  double GDouble::customFitness(void){
    double result=0.,low, up;
    double sumgaps, sumgapsm1, gapcounter, peak=0.;
    double distance=0., current_region=0.;
    double iValue = GTemplateValue<double>::getInternalValue();
    vector<shared_ptr<GRange> >::iterator it;
	
	
    low = range_.lowerBoundary();
    up  = range_.upperBoundary();

    // Find out where the first peak of the transfer function is.
    // width() takes care of open/closed gap boundaries
    peak=up;  	
    for(it=gps_.begin(); it!=gps_.end(); ++it) peak -= (*it)->width();
  	  	
    // find out the distance between adjacent (lower+upper) peaks
    // This will fail if (peak-low) > DBL_MAX
    if((peak - DBL_MAX) >= low){
      GException ge;
      ge << "In GDouble::customFitness(): Error!" << endl
	 << "Bad peak or low : " << peak << " " << low << endl
	 << raiseException;
    }

    distance = peak-low;
    if(distance <= 0.){
      GException ge;
      ge << "In GDouble::customFitness(): Error!" << endl
	 << "Bad distance : " << distance << endl
	 << raiseException;
    }

    // Find out which region "iValue" is in :

    // (iValue-low) may not be larger than DBL_MAX
    if((iValue - DBL_MAX) >= low){
      GException ge;
      ge << "In GDouble::customFitness(): Error!" << endl
	 << "Bad iValue or low : " << iValue << " " << low << endl
	 << raiseException;
    }

    // how many integer multiples of distance fit into region
    // right of "low" on x-Axis. "+1" is being used due to the numbering
    // scheme being used.
    current_region=floor((iValue-low)/distance)+1.;

    // shift iValue back to region 1, so iValue doesn't keep increasing over time :

    // if(current_region%2==0.) // we are in region 0,2,...
    if(fmod(current_region,2.)==0.){ // we are in region 0,2,...
      // shift variable to region 2
      iValue-=(current_region-2.)*distance;

      // shift variable into region 1 (i.e., search for iValue that reproduces
      // last y-value of iValue within region 1
      iValue -= (2*distance + 2*low);
      iValue *= -1.;
    }
    else{ // we are in region 1,3,...
      // shift variable to region 1
      iValue-=(current_region-1.)*distance;
    }

    // set the internal value as desired
    GTemplateValue<double>::setInternalValue(iValue);

    // this is the real transformation x->y. All the code before this line was
    // just needed to find a suitable x-variable yielding the same y-value as "var".
    if(gps_.size()==0){
      result=iValue;
    }
    else{
      // needed to find out, in the range of which gap we are
      sumgaps=0.;
      for(it=gps_.begin(); it!=gps_.end(); ++it) sumgaps += (*it)->width();
      sumgapsm1=sumgaps - gps_.back()->width();

      // find out, where we are with respect to the gaps
      if(iValue <= gps_.front()->lowerBoundary() && iValue>low){ // we are below the lowest gap
	result=iValue; // just f(x)=x
      }
      else if(iValue<=(up-sumgaps) && 
	      iValue>(gps_.back()->lowerBoundary()-sumgapsm1)){ // above the uppermost gap
	result=iValue+sumgaps;
      }
      else{ // somewhere in the range of the gaps
	gapcounter=0;
	result=0.;
	for(it=gps_.begin(); it!=(gps_.end()-1);++it){
	  gapcounter += (*it)->width();
	  if(iValue <= ((*(it+1))->lowerBoundary() - gapcounter)){
	    result=iValue+gapcounter;
	    break;      				
	  }
	}
      }
    }

    return result;
  }

  /******************************************************************************/
  /**
   * The internal value is retrieved and all registered adaptors are
   * applied to it. Then the value is restored. 
   */
  void GDouble::customMutate(void){
    double mutVal = GTemplateValue<double>::getInternalValue();
    GMutable<double>::applyAllAdaptors(mutVal);
    GTemplateValue<double>::setInternalValue(mutVal);
  }

  /******************************************************************************/
  /**
   * Sanity check - do any ranges overlap ? Is any range beyond the
   * boundaries of this GDouble ? NOTE: This function assumes that the
   * ranges have been sorted. A return value "true" means that there
   * are no overlapping regions.
   * 
   * \return A boolean indicating whether no overlapping regions exist
   */
  bool GDouble::checkRanges(void){
    // No local gaps ? Nothing to do.
    if(gps_.size() == 0) return true;

    // Check that no range overlaps the boundaries of this GDouble
    if(gps_.back()->lowerBoundary() < range_.lowerBoundary() ||
       gps_.back()->upperBoundary() > range_.upperBoundary()){
      return false;
    }

    vector<shared_ptr<GRange> >::iterator it;
    for(it=gps_.begin()+1; it!=gps_.end(); ++it){
      if((*it)->overlaps(*((it-1)->get()))) return false;
    }

    return true;
  }

  /******************************************************************************/
  /**
   * This is a helper function used in the copy constructor and
   * the GDouble::load() function. It is used to copy the gaps of 
   * another GDouble into our own gps_ vector.
   * 
   * \param cp A vector holding gaps.
   */
  void GDouble::copyGaps(const vector<shared_ptr<GRange> > & cp){
    vector<shared_ptr<GRange> >::iterator it_l;
    vector<shared_ptr<GRange> >::const_iterator it_f;
	
    uint16_t lsize = gps_.size(), fsize = cp.size();

    if(lsize <= fsize){
      // first copy the data until lsize
      for(it_l=gps_.begin(), it_f=cp.begin(); it_l!=gps_.end(); ++it_l, ++it_f) *it_l = *it_f;
		
      // Now take care of the items in cp above lsize
      for(it_f=cp.begin()+lsize; it_f!=cp.end(); ++it_f){
	// We already know that it_f points to a GRange object whose
	// clone() function emits a GObject * . Hence we can use the
	// faster static_cast instead of a dynamic_cast .
	GRange *grange = static_cast<GRange *>((*it_f)->clone());
	shared_ptr<GRange> p(grange);
	gps_.push_back(p);
      }
    }
    else{ // we need to remove items
      for(it_l=gps_.begin(), it_f=cp.begin(); it_f!=cp.end(); ++it_l, ++it_f) *it_l = *it_f;
      gps_.resize(fsize); // drops items at the end of the vector
    }
  }

  /******************************************************************************/
  /**
   * Sorts the gaps according to their lower values. This function creates
   * a function object on the fly using the Boost.Bind framework. See 
   * http://www.boost.org/libs/bind/bind.html for further information.
   */
  void GDouble::sortGaps(void){
    sort(gps_.begin(), gps_.end(),
	 boost::bind(
		     std::less<double>(),
		     boost::bind(&GRange::lowerBoundary,_1),
		     boost::bind(&GRange::lowerBoundary,_2)
		     ));
  }

  /******************************************************************************/

} /* namespace GenEvA */
