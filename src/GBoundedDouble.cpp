/**
 * @file
 */

/* GBoundedDouble.cpp
 *
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

#include "GBoundedDouble.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoundedDouble)

namespace Gem
{
	namespace GenEvA
	{

		/******************************************************************************/
		/**
		 * The default constructor. As this class uses the adaptor scheme
		 * (see GTemplateAdaptor<T>), you will need to add your own adaptors,
		 * such as the GDoubleGaussAdaptor.
		 */
		GBoundedDouble::GBoundedDouble()
			:GTemplateValue<double>(0.),
			 internal_value_(0.)

		{
			// Initialize randomly within the allowed value range.
			// Also checks whether the supplied value is inside the allowed range
			setExternalValue(gr.evenRandom(range_.lowerBoundary(),range_.upperBoundary()));
		}

		/******************************************************************************/
		/**
		 * Initialize with a given double value. Note that this constructor
		 * can also be used for conversion. GTemplateValue<double> is initialized
		 * with 0, as that class holds the internal representation. The correct
		 * internal value will be set by GBoundedDouble::setExternalValue() . The
		 * function purposely allows conversions from double values.
		 *
		 * @param val Initialisation value
		 */
		GBoundedDouble::GBoundedDouble(double val)
			:GTemplateValue<double>(0.),
			 internal_value_(0.)
		{
			setExternalValue(val);
		}

		/******************************************************************************/
		/**
		 * A standard copy constructor. Most work is done by the parent
		 * classes, we only need to copy ranges and gaps.
		 *
		 * @param cp Another GBoundedDouble object
		 */
		GBoundedDouble::GBoundedDouble(const GBoundedDouble& cp)
			:GTemplateValue<double>(cp),
			 range_(cp.range_),
			 internal_value_(cp.internal_value_)
		{
			// this part is complicated. Use a helper function
			copyRanges(cp.gps_);
		}

		/******************************************************************************/
		/**
		 * A standard destructor. No local, dynamically allocated data,
		 * hence nothing to do. Note that the pointers to GRange objects
		 * stored in the gps_ vector will be cleared automatically, as they
		 * are wrapped into a boost::shared_ptr<> object.
		 */
		GBoundedDouble::~GBoundedDouble()
		{ /* nothing */}

		/******************************************************************************/
		/**
		 * A standard assignment operator for GBoundedDouble.
		 *
		 * @param cp A constant reference to another GBoundedDouble object
		 * @return A constant reference to this object
		 */
		const GBoundedDouble& GBoundedDouble::operator=(const GBoundedDouble& cp) {
			GBoundedDouble::load(&cp);
			return *this;
		}

		/******************************************************************************/
		/**
		 * A standard assignment operator for double values
		 *
		 * @param The desired new external value
		 * @return The new external value of this object
		 */
		double GBoundedDouble::operator=(double val) {
			this->setExternalValue(val);
			return this->value();
		}

		/******************************************************************************/
		/**
		 * Resets the object to its initial state.
		 */
		void GBoundedDouble::reset(void) {
			// Reset the local data
			range_.reset();
			gps_.clear();
			internal_value_=0.;

			// Reset the parent class
			GTemplateValue<double>::reset();
		}

		/******************************************************************************/
		/**
		 * Loads the data of another GBoundedDouble, camouflaged as a GObject
		 * into this object.
		 *
		 * @param cp Another GBoundedDouble object, camouflaged as a GObject
		 */
		void GBoundedDouble::load(const GObject *cp) {
			// Convert GObject pointer to local format
			const GBoundedDouble *gbd_load	= checkedConversion<GBoundedDouble>(cp, this);

			// Load our parent class'es data ...
			GParameterT<double>::load(cp);

			// ... and then our own
			range_ = gbd_load->range_;
			copyRanges(gbd_load->gps_);
			internal_value_ = gbd_load->internal_value_;
		}

		/******************************************************************************/
		/**
		 * Create a deep copy of this object. Basically this is a fabric function.
		 *
		 * @return A newly generated GBoundedDouble
		 */
		GObject *GBoundedDouble::clone(void) {
			return new GBoundedDouble(*this);
		}

		/******************************************************************************/
		/**
		 * Sets the upper and lower boundaries of this object. Boundaries
		 * can be open or closed. Note that this function may only be called
		 * while no gaps have been added. So once you have added gaps, this
		 * function will throw an exception.
		 *
		 * @param lower_bound The new lower boundary
		 * @param lower_bound_open A boolean indicating whether or not lower_bound is open
		 * @param upper_bound The new upper boundary
		 * @param upper_bound_open A boolean indicating whether or not upper_bound is open
		 */
		void GBoundedDouble::setBoundaries(double lower_bound, bool lower_bound_open,
										   double upper_bound, bool upper_bound_open) {
			// Resizing the boundaries with gaps present could result in the need to
			// remove gaps - a situation which we do not know how to handle - after all a gap
			// could indicate a crucial area of the parameter space that may not be touched.
			if(!gps_.empty()){
				std::ostringstream error;
				error << "In GBoundedDouble::setBoundaries(): Error!" << std::endl
					  << "Tried to change boundaries while GRange objects were present." << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_resize_with_ranges() << error_string(error.str());
			}

			// save current external value;
			double val = this->value();

			// set the boundaries as required
			range_.setBoundaries(lower_bound,lower_bound_open,
								 upper_bound,upper_bound_open);

			// Reset the external value if it is outside of the range
			if(val < range_.lowerBoundary()) val = range_.lowerBoundary();
			else if(val> range_.upperBoundary()) val = range_.upperBoundary();

			// set the external value to the original value (if possible).
			// Note that in the presence of boundaries the internal representation
			// will be re-calculated.
			setExternalValue(val);
		}

		/******************************************************************************/
		/**
		 * A similar function to GBoundedDouble::setBoundaries(double, bool, double, bool),
		 * but assumes that the boundaries are closed.
		 *
		 * @param lower_bound The new lower boundary
		 * @param upper_bound The new upper boundary
		 */
		void GBoundedDouble::setBoundaries(double lower_bound, double upper_bound) {
			this->setBoundaries(lower_bound, BNDISCLOSED, upper_bound, BNDISCLOSED);
		}

		/******************************************************************************/
		/**
		 * Adds a gap to the value range_ of this object. Gaps can have open
		 * or closed boundaries. As gaps act as helper classes, there is no
		 * function that allows to add a gap object. Users should not be
		 * required to know about the internal implementation of this class.
	     * Note that this function needs to be called after the external boundaries
	     * of the object have been set. Adding a range which overlaps with these boundaries
	     * or with other ranges will result in an exception being thrown. Note also that,
	     * as a post-condition, if the externally visible value suddenly lies within
	     * one of the disallowed ranges, it will be adapted to the lower boundary of
	     * that range.
		 *
		 * @param lower_bound The new lower boundary
		 * @param lower_bound_open A boolean indicating whether or not lower_bound is open
		 * @param upper_bound The new upper boundary
		 * @param upper_bound_open A boolean indicating whether or not upper_bound is open
		 */
		void GBoundedDouble::addRange(double lower_bound, bool lower_bound_open,
								      double upper_bound, bool upper_bound_open)
		{
			// create and add range
			boost::shared_ptr<GRange> p(
					new GRange(lower_bound, lower_bound_open,
							   upper_bound, upper_bound_open);
			)
			gps_.push_back(p);

			// sort gaps
			sortRanges();

			// check that nothing overlaps
			if(!rangesOk()) {
				std::ostringstream error;
				error << "In GBoundedDouble::addRange() : Error!" << std::endl
					  << "Range seems to overlap another" << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_invalid_range_added() << error_string(error.str());
			}

			// Adjust the external value, if necessary
		}

		/******************************************************************************/
		/**
		 * A similar function to GBoundedDouble::addRange(double, bool, double, bool), but
		 * assumes that both boundaries of the gap are closed.
		 *
		 * @param lbnd The new lower boundary
		 * @param ubnd The new upper boundary
		 */
		void GBoundedDouble::addRange(double lbnd, double ubnd) {
			this->addRange(lbnd,BNDISCLOSED,ubnd,BNDISCLOSED);
		}

		/******************************************************************************/
		/**
		 * The standard way to retrieve the external value of a class
		 * derived from GTemplateValue<T> is the getValue() function.
		 * In the case of a GBoundedDouble the external value is different from
		 * the internal value and needs to be recalculated after every
		 * mutation. Hence we need to overload this function from the
		 * default version, which just returns the internal value.
		 */
		double GBoundedDouble::getValue(void) {
			return this->value();
		}

		/******************************************************************************/
		/**
		 * Sets the internal value in such a way that the user-visible
		 * value is set to "val". GBoundedDouble performs a linear transformation
		 * from inner to outer value in the areas where no gaps are present.
		 * A gap introduces a difference equal to its width between the
		 * inner and outer representation.
		 *
		 * @param val The desired new external value
		 * @return The original value
		 */
		double GBoundedDouble::setExternalValue(double val) {
			double result, gapcounter, sumgaps;
			double low, up, lov=val;

			std::vector<boost::shared_ptr<GRange> >::iterator it;

			low = range_.lowerBoundary();
			up = range_.upperBoundary();

			// check that the value is inside the allowed range.
			// Adapt if necessary ...
			if(lov> up) lov=up;
			else if(lov < low) lov=low;

			// no gaps ? result is just f(x)=x
			if(gps_.size()==0) {
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
				else if(lov>=gps_.front()->lowerBoundary() && lov<=gps_.front()->upperBoundary()) {
					result=gps_.front()->lowerBoundary();
				}
				else if(lov>gps_.back()->upperBoundary()) { // above the uppermost gap
					result=lov-sumgaps;
				}
				else { // somewhere above the lowest upper-gap and below the highest upper gap boundary
					gapcounter=0;
					result=0.;

					for(it=gps_.begin()+1; it!=gps_.end(); ++it) {
						// if we are in the range of a gap, return
						// the y-value of the lower boundary
						gapcounter += (*(it-1))->width();
						if(lov>=(*it)->lowerBoundary() && lov<=(*it)->upperBoundary()) {
							result=(*it)->lowerBoundary() - gapcounter;
							break;
						}
						else if(lov>(*(it-1))->upperBoundary() && lov<(*it)->lowerBoundary()) { // between gaps
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
		 * This function allows automatic conversion from GBoundedDouble to GBoundedDouble. This
		 * allows us to define only few operators, as the bulk of the work will be
		 * done by automatic conversions done by the C++ compiler.
		 */
		GBoundedDouble::operator double () {
			return this->value();
		}

		/******************************************************************************/
		/**
		 * A comparison operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value to be compared with this object
		 * @return A boolean indicating whether val is larger than this object
		 */
		bool GBoundedDouble::operator<(GBoundedDouble& val) {
			return this->value() < val.value();
		}

		/******************************************************************************/
		/**
		 * A comparison operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value to be compared with this object
		 * @return A boolean indicating equivalence
		 */
		bool GBoundedDouble::operator==(GBoundedDouble& val) {
			return this->value() == val.value();
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value to be added to this object
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator+=(GBoundedDouble& val) {
			this->setExternalValue(this->value()+val.value());
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value to be subtracted from this object
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator-=(GBoundedDouble& val) {
			this->setExternalValue(this->value()-val.value());
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value to be multiplied with this object
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator*=(GBoundedDouble& val) {
			this->setExternalValue(this->value() * val.value());
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value by which this object should be divided
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator/=(GBoundedDouble& val) {
			double divisor=val.value();
			if(divisor==0.) {
				GException ge;
				ge << "In GBoundedDouble::operator/=() : Attempted division by 0." << std::endl
				<< raiseException;
			}

			setExternalValue(this->value()/divisor);
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value used for assignment
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator%=(GBoundedDouble& val) {
			this->setExternalValue(double(int32_t(this->value()) % int32_t(val.value())));
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value used for assignment
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator|=(GBoundedDouble& val) {
			this->setExternalValue(double(int32_t(this->value()) | int32_t(val.value())));
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value used for assignment
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator&=(GBoundedDouble& val) {
			this->setExternalValue(double(int32_t(this->value()) & int32_t(val.value())));
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @param  val The value used for assignment
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator^=(GBoundedDouble& val) {
			this->setExternalValue(double(int32_t(this->value()) ^ int32_t(val.value())));
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator++() {
			this->setExternalValue(this->value() + 1.);
			return *this;
		}

		/******************************************************************************/
		/**
		 * An assignment operator, which aids Boost.operators in the definition of
		 * arithmentic operators for calculations performed with this class.
		 *
		 * @return A constant reference to this object
		 */
		GBoundedDouble& GBoundedDouble::operator--() {
			this->setExternalValue(this->value() - 1.);
			return *this;
		}

		/******************************************************************************/
		/**
		 * This function implements the mapping from internal to external
		 * value.
		 *
		 * @return The user-visible value
		 *
		 * \todo Inefficient. The first part of the function will always
		 * be calculated, although this is really only necessary during
		 * changes of gps_ or range_ . Also, if gps_size() == 0, we
		 * nevertheless follow all the code.
		 */
		double GBoundedDouble::customFitness(void) {
			double result=0.,low, up;
			double sumgaps, sumgapsm1, gapcounter, peak=0.;
			double distance=0., current_region=0.;
			double iValue = GTemplateValue<double>::getInternalValue();
			std::vector<boost::shared_ptr<GRange> >::iterator it;

			low = range_.lowerBoundary();
			up = range_.upperBoundary();

			// Find out where the first peak of the transfer function is.
			// width() takes care of open/closed gap boundaries
			peak=up;
			for(it=gps_.begin(); it!=gps_.end(); ++it) peak -= (*it)->width();

			// find out the distance between adjacent (lower+upper) peaks
			// This will fail if (peak-low) > DBL_MAX
			if((peak - DBL_MAX) >= low) {
				GException ge;
				ge << "In GBoundedDouble::customFitness(): Error!" << std::endl
				<< "Bad peak or low : " << peak << " " << low << std::endl
				<< raiseException;
			}

			distance = peak-low;
			if(distance <= 0.) {
				GException ge;
				ge << "In GBoundedDouble::customFitness(): Error!" << std::endl
				<< "Bad distance : " << distance << std::endl
				<< raiseException;
			}

			// Find out which region "iValue" is in :

			// (iValue-low) may not be larger than DBL_MAX
			if((iValue - DBL_MAX) >= low) {
				GException ge;
				ge << "In GBoundedDouble::customFitness(): Error!" << std::endl
				<< "Bad iValue or low : " << iValue << " " << low << std::endl
				<< raiseException;
			}

			// how many integer multiples of distance fit into region
			// right of "low" on x-Axis. "+1" is being used due to the numbering
			// scheme being used.
			current_region=floor((iValue-low)/distance)+1.;

			// shift iValue back to region 1, so iValue doesn't keep increasing over time :

			// if(current_region%2==0.) // we are in region 0,2,...
			if(fmod(current_region,2.)==0.) { // we are in region 0,2,...
				// shift variable to region 2
				iValue-=(current_region-2.)*distance;

				// shift variable into region 1 (i.e., search for iValue that reproduces
				// last y-value of iValue within region 1
				iValue -= (2*distance + 2*low);
				iValue *= -1.;
			}
			else { // we are in region 1,3,...
				// shift variable to region 1
				iValue-=(current_region-1.)*distance;
			}

			// set the internal value as desired
			GTemplateValue<double>::setInternalValue(iValue);

			// this is the real transformation x->y. All the code before this line was
			// just needed to find a suitable x-variable yielding the same y-value as "var".
			if(gps_.size()==0) {
				result=iValue;
			}
			else {
				// needed to find out, in the range of which gap we are
				sumgaps=0.;
				for(it=gps_.begin(); it!=gps_.end(); ++it) sumgaps += (*it)->width();
				sumgapsm1=sumgaps - gps_.back()->width();

				// find out, where we are with respect to the gaps
				if(iValue <= gps_.front()->lowerBoundary() && iValue>low) { // we are below the lowest gap
					result=iValue; // just f(x)=x
				}
				else if(iValue<=(up-sumgaps) &&
						iValue>(gps_.back()->lowerBoundary()-sumgapsm1)) { // above the uppermost gap
					result=iValue+sumgaps;
				}
				else { // somewhere in the range of the gaps
					gapcounter=0;
					result=0.;
					for(it=gps_.begin(); it!=(gps_.end()-1);++it) {
						gapcounter += (*it)->width();
						if(iValue <= ((*(it+1))->lowerBoundary() - gapcounter)) {
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
		 * Sanity check - do any ranges overlap ? Is any range beyond the
		 * boundaries of this GBoundedDouble ? NOTE: This function assumes that the
		 * ranges have been sorted. A return value "true" means that there
		 * are no overlapping regions.
		 *
		 * @return A boolean indicating whether no overlapping regions exist
		 */
		bool GBoundedDouble::rangesOk(void) {
			// No local gaps ? Nothing to do.
			if(gps_.size() == 0) return true;

			// Check that the ranges themselves don't overlap
			std::vector<boost::shared_ptr<GRange> >::iterator gr_it;
			for(gt_it=gps_.begin()+1; gr_it!=gps_.end(); ++gr_it) {
				if((*gr_it)->overlaps(*((gr_it-1)->get()))) return false;
			}

			// Now we know that the upper boundary of each range is below
			// the lower boundary of the next range.

			// Check that no range overlaps the boundaries of this GBoundedDouble
			if(gps_.front()->lowerBoundary() < range_.lowerBoundary() ||
			   gps_.back()->upperBoundary() > range_upperBoundary()) return false;

			return true;
		}

		/******************************************************************************/
		/**
		 * This is a helper function used in the copy constructor and
		 * the GBoundedDouble::load() function. It is used to copy the gaps of
		 * another GBoundedDouble into our own gps_ vector.
		 *
		 * @param cp A vector holding gaps.
		 */
		void GBoundedDouble::copyRanges(const std::vector<boost::shared_ptr<GRange> > & cp) {
			std::vector<boost::shared_ptr<GRange> >::iterator it_this;
			std::vector<boost::shared_ptr<GRange> >::const_iterator cit_cp;

			// Copy all GRange objects until we reach the end of one of the vectors
			for(it_this=gps_.begin, cit_cp=cp.gps_.begin();
				it_this!=gps_.end(), cit_cp!=cp.gps_.end();
				++it_this, ++cit_cp){
				it_this->load = cit_cp->get();
			}

			std::size_t this_size = gps_.size(), cp_size = cp.gps_.size();
			if(this_size == cp_size); // Nothing to do
			else if(this_size < cp_size){
				// Attach remaining GRange objects
				for(cit_cp=cp.gps_.begin()+this_size; cit_cp!=cp.gps_.end(); ++cit_cp){
					shared_ptr<GRange> p(new GRange());
					p->load(cit_cp->get());
					this->push_back(p);
				}
			}
			else{ // this_size > cp_size
				// Removes the surplus items at the end of the vector
				gps_.resize(cp_size);
			}

			// Make sure the GRange objects are sorted
			sortRanges();
		}

		/******************************************************************************/
		/**
		 * Sorts the gaps according to their lower values. This function creates
		 * a function object on the fly using the Boost.Bind framework. See
		 * http://www.boost.org/libs/bind/bind.html for further information.
		 */
		void GBoundedDouble::sortRanges(void) {
			std::sort(gps_.begin(), gps_.end(),
					boost::bind(
							std::less<double>(),
							boost::bind(&GRange::lowerBoundary,_1),
							boost::bind(&GRange::lowerBoundary,_2)
					));
		}

		/******************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
