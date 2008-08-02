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
		 * such as the GDoubleGaussAdaptor. This constructor is private and thus
		 * cannot be used. It is here as it is needed by the serialization
		 * framework.
		 */
		GBoundedDouble::GBoundedDouble()
			:GParameterT<double>(0.),
			 lowerBoundary_(0.),
			 upperBoundary_(0.),
			 internalValue_(0.)
		{ /* nothing */ }

		/******************************************************************************/
	    /**
	     * Initializes the boundaries and sets the external value to a random number
	     * inside the allowed value range.
	     *
	     * @param lowerBoundary The lower boundary of the value range
	     * @param upperBoundary The upper boundary of the value range
	     */
		GBoundedDouble::GBoundedDouble(double lowerBoundary, double upperBoundary)
			:GParameterT<double>(0.),
			 lowerBoundary_(lowerBoundary),
			 upperBoundary_(upperBoundary),
			 internalValue_(0.)
		{
			// This function also sets the internalValue_ variable.
			setExternalValue(gr.evenRandom(lowerBoundary_,upperBoundary_));
		}

		/******************************************************************************/
		/**
		 * Initialize with a given double value and the allowed value range.
		 *
		 * @param val Initialisation value
  	     * @param lowerBoundary The lower boundary of the value range
	     * @param upperBoundary The upper boundary of the value range
		 */
		GBoundedDouble::GBoundedDouble(double val, double lowerBoundary, double upperBoundary)
			:GParameterT<double>(0.),
			 lowerBoundary_(lowerBoundary),
			 upperBoundary_(upperBoundary),
			 internalValue_(0.)
		{
			setExternalValue(val);
		}

		/******************************************************************************/
		/**
		 * A standard copy constructor. Most work is done by the parent
		 * classes, we only need to copy the internal value and the allowed
		 * value range.
		 *
		 * @param cp Another GBoundedDouble object
		 */
		GBoundedDouble::GBoundedDouble(const GBoundedDouble& cp)
			:GParameterT<double>(cp),
			 lowerBoundary_(cp.lowerBoundary_),
			 upperBoundary_(cp.upperBoundary_),
			 internalValue_(cp.internalValue_)
		{ /* nothing */	}

		/******************************************************************************/
		/**
		 * A standard destructor. No local, dynamically allocated data,
		 * hence nothing to do.
		 */
		GBoundedDouble::~GBoundedDouble()
		{ /* nothing */}

		/******************************************************************************/
		/**
		 * A standard assignment operator for GBoundedDouble objects
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
		 * A standard assignment operator for double values. Note that this function
		 * will throw an exception if the new value is not in the allowed value range.
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
		 * Resets the object to its initial state. Note that the allowed value range is
		 * not reset, as it is not possible to set it externally. It is thus considered
		 * to be part of the initial state.
		 */
		void GBoundedDouble::reset(void) {
			// Reset the local data
			internalValue_=0.;

			// Reset the parent class
			GParameterT<double>::reset();
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
			const GBoundedDouble *gbd_load	= this->checkedConversion(cp, this);

			// Load our parent class'es data ...
			GParameterT<double>::load(cp);

			// ... and then our own
			lowerBoundary_ = gbd_load->lowerBoundary_;
			upperBoundary_ = gbd_load->upperBoundary_;
			internalValue_ = gbd_load->internalValue_;
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
	     * Retrieves the lower boundary
	     */
	    double GBoundedDouble::getLowerBoundary() const throw(){
	    	return lowerBoundary_;
	    }

	    /******************************************************************************/
	    /**
	     * Retrieves the upper boundary
	     */
	    double GBoundedDouble::getUpperBoundary() const throw(){
	    	return upperBoundary_;
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
		 * Mutates this object. It is the internal representation of the class'es value
		 * that gets mutated. This value is then "translated" into the external value (stored
		 * in GParameterT<double>, which is set accordingly.
		 */
		void GBoundedDouble::mutate(void){
			// First apply the mutation to the internal representation of our value
			if(numberOfAdaptors() == 1){
				GParameterBaseWithAdaptorsT<T>::applyFirstAdaptor(internalValue_);
			}
			else {
				GParameterBaseWithAdaptorsT<T>::applyAllAdaptors(internalValue_);
			}

			// Then calculate the corresponding external value and set it accordingly

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
			GParameterT<double>::setInternalValue(result);

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
			double iValue = GParameterT<double>::getInternalValue();
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
			GParameterT<double>::setInternalValue(iValue);

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

	} /* namespace GenEvA */
} /* namespace Gem */
