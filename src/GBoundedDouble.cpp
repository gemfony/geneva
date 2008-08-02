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
		void GBoundedDouble::mutate(){
			// First apply the mutation to the internal representation of our value
			if(numberOfAdaptors() == 1){
				this->applyFirstAdaptor(internalValue_);
			}
			else {
				this->applyAllAdaptors(internalValue_);
			}

			// Then calculate the corresponding external value and set it accordingly
			double externalValue = calculateExternalValue(internalValue_);

			// Set the external value accordingly. This will transfer the internal value
			// back into region 0. This is important, so the internal value does not
			// become too large due to the mutation.
			setExternalValue(externalValue);
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
			double previous = this->value();

			// Check the lower an upper boundaries
			if(upperBoundary_ <= lowerBoundary_){
				std::ostringstream error;
				error << "In GBoundedDouble::setExternalValue() : Error!" << std::endl
					  << "Got invalid upper and/or lower boundaries" << std::endl
					  << "lowerBoundary_ = " << lowerBoundary_ << std::endl
					  << "upperBoundary_ = " << upperBoundary_ << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_invalid_boundaries() << error_string(error.str());
			}

			// Check that the value is inside the allowed range
			if(val < lowerBoundary_ || val > upperBoundary_){
				std::ostringstream error;
				error << "In GBoundedDouble::setExternalValue() : Error!" << std::endl
					  << "Attempt to set external value outside of allowed range." << std::endl
					  << "lowerBoundary_ = " << lowerBoundary_ << std::endl
					  << "upperBoundary_ = " << upperBoundary_ << std::endl
					  << "val = " << val << std::endl;
			}

			// The transfer function in this area is just f(x)=x, so we can just
			// assign the external to the internal value.
			internalValue_ = val;
			this->setValue(val);

			return previous;
		}

		/******************************************************************************/
	    /**
	     * Does the actual mapping from external to internal value
	     *
	     * @param in The internal value that is to be converted to an external value
	     * @param out The externally visible representation of in
	     */
	    double GBoundedDouble::calculateExternalValue(const double& in) {
			// Check the boundaries we've been given
			if(upperBoundary_ <= lowerBoundary_){
				std::ostringstream error;
				error << "In GBoundedDouble::calculateExternalValue() : Error!" << std::endl
					  << "Got invalid upper and/or lower boundaries" << std::endl
					  << "lowerBoundary_ = " << lowerBoundary_ << std::endl
					  << "upperBoundary_ = " << upperBoundary_ << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_invalid_boundaries() << error_string(error.str());
			}

			// b) find out which region the value is in (compare figure transferFunction.pdf
			// that should have been delivered with this software . Note that numeric_cast
			// may throw - exceptions must be caught in surrounding functions.
			boost::int32_t region =
				boost::numeric_cast<boost::int32_t>(std::floor((in - lowerBoundary_) / (upperBoundary_ - lowerBoundary_)));

			// c) Check whether we are in an odd or an even range and calculate the
			// external value accordingly
			double externalValue = 0.;
			if(region%2 == 0){ // can it be divided by 2 ? Region 0,2,... or a negative even range
				externalValue = in - region * (upperBoundary_ - lowerBoundary_);
			}
			else{ // Range 1,3,... or a negative odd range
				externalValue = -in + ((region-1)*(upperBoundary_ - lowerBoundary_) + 2*upperBoundary_);
			}

			return externalValue;
	    }

	    /******************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
