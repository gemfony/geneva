/**
 * @file
 */

/* GRange.cpp
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

#include "GRange.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GRange)

namespace Gem
{
	namespace GenEvA
	{

		/***********************************************************************/
		/**
		 * The default constructor. It tries to set the lower and upper boundary in
		 * such a way that at least DEFMINDIGITS decimal places are available.
		 * Note that the ranges are actually set in GRange::setMinDigits, not in this
		 * constructor.
		 */
		GRange::GRange()
			:GObject("GRange")
		{
			setMinDigits(DEFMINDIGITS);
			setIsActive(true);
		}

		/***********************************************************************/
		/**
		 * This constructor sets the lower and upper boundary and activates the range.
		 *
		 * @param lower The value of the lower boundary
		 * @param lwopen A value indicating whether lower is an open (true) or closed (false) boundary
		 * @param upper The value of the upper boundary
		 * @param upopen A value indicating whether upper is an open (true) or closed (false) boundary
		 */
		GRange::GRange(double lower, bool lwopen, double upper, bool upopen)
		:GObject("GRange")
		{
			lower_.setBoundary(lower,ISLOWER, lwopen);
			upper_.setBoundary(upper,ISUPPER, upopen);

			setIsActive(true);
		}

		/***********************************************************************/
		/**
		 * A standard copy constructor.
		 *
		 * @param cp Another GRange object
		 */
		GRange::GRange(const GRange& cp)
			:GObject(cp)
		{
			lower_ = cp.lower_;
			upper_ = cp.upper_;

			// This operation does more than just setting a variable
			setIsActive(cp.isactive_);
		}

		/***********************************************************************/
		/**
		 * The standard destructor. As we have no dynamically allocated data,
		 * there is nothing here to do.
		 */
		GRange::~GRange() {
			/* nothing */
		}

		/***********************************************************************/
		/**
		 * A standard assignment operator
		 *
		 * @param cp Another GRange object
		 */
		const GRange& GRange::operator=(const GRange& cp) {
			GRange::load(&cp);
			return *this;
		}

		/***********************************************************************/
		/**
		 * Resets the range to its initial values
		 */
		void GRange::reset() {
			setMinDigits(DEFMINDIGITS);
			setIsActive(false);

			GObject::reset();
		}

		/***********************************************************************/
		/**
		 * Loads the data of another GRange object, camouflaged as a GObject.
		 *
		 * @param cp Another GRange object, camouflaged as a GObject
		 */
		void GRange::load(const GObject * cp) {
			const GRange *gr_load = dynamic_cast<const GRange *> (cp);

			// dynamic_cast will emit a NULL pointer, if the conversion failed
			if (!gr_load) {
				std::ostringstream error;
				error << "In GRange::load(): Conversion error!" << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
			}

			// Check that this object is not accidentally assigned to itself.
			if (gr_load == this) {
				std::ostringstream error;
				error << "In GRange::load(): Error!" << std::endl
						<< "Tried to assign an object to itself." << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
				throw geneva_object_assigned_to_itself() << error_string(error.str());
			}

			// First load our parent class'es data ...
			GObject::load(cp);

			// ... and then our own
			lower_ = gr_load->lower_;
			upper_ = gr_load->upper_;
			setIsActive(gr_load->isactive_);
		}

		/***********************************************************************/
		/**
		 * Creates a deep copy of this object.
		 *
		 * @return A deep copy of this object, camouflaged as a GObject
		 */
		GObject *GRange::clone() {
			return new GRange(*this);
		}

		/***********************************************************************/
		/**
		 * Marks the range as active. This implies marking both boundaries as active.
		 *
		 * @param A parameter indicating whether the range is active (true) or inactive (false)
		 */
		void GRange::setIsActive(bool isactive) {
			// First mark the boundaries as active ...
			lower_.setIsActive(isactive);
			upper_.setIsActive(isactive);
			// ... and then ourself.
			isactive_ = isactive;
		}

		/***********************************************************************/
		/**
		 * Checks whether the range is active or inactive
		 *
		 * @return A value indicating whether the range is active (true) or inactive (false)
		 */
		bool GRange::isActive() const {
			// Do both boundaries have the same state as the isactive_ parameter ?
			// It is a serious error if this is not the case
			if((lower_.isActive()!=upper_.isActive()) || (lower_.isActive()!=isactive_)) {
				std::ostringstream error;
				error << "In GRange::isActive() : Bad active values. They should all be the same!" << std::endl
					  << "lower boundary " << lower_.isActive() << std::endl
					  << "upper boundary " << upper_.isActive() << std::endl
					  << "GRange         " << isactive_ << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_invalid_range_data() << error_string(error.str());
			}

			return isactive_;
		}

		/***********************************************************************/
		/**
		 * This function tries to limit the range in such a way that likely ndigits decimal
		 * places are available. This assumes that the mantissa has MAXDIGITS digits. Note that
		 * this means that part of the usual range of a double ( approx. -10^308:10^308) is
		 * unavailable. Also note that the limit imposed is not a strong one. The GRange object
		 * will have the same activity status as before the command.
		 *
		 * @param ndigits The number of desired decimal places
		 */
		void GRange::setMinDigits(boost::int16_t ndigits) {
			// We assume MAXDIGITS digits in a double. This is crude ...
			const boost::int16_t exponent = MAXDIGITS - ndigits;
			const double extreme = std::pow(10.,double(exponent));

			setLowerBoundary(-extreme,false);
			setUpperBoundary(+extreme,false);
		}

		/***********************************************************************/
		/**
		 * This function sets the upper and lower boundaries in one go. The GRange
		 * object will have the same activity status as before the call.
		 *
		 * @param lower The value of the lower boundary
		 * @param lwopen Indicates whether this is an open (true) or closed (false) boundary
		 * @param upper The value of the upper boundary
		 * @param upopen Indicates whether this is an open (true) or closed (false) boundary
		 */

		void GRange::setBoundaries(double lower, bool lwopen, double upper, bool upopen) {
			setLowerBoundary(lower,lwopen);
			setUpperBoundary(upper, upopen);
		}

		/***********************************************************************/
		/**
		 * Sets the lower boundary to a given value. This function is private, as
		 * it should not be called independently of the upper boundary.
		 *
		 * @param lower The value of the lower boundary
		 * @param isopen Indicates whether this is an open (true) or closed (false) boundary
		 */
		void GRange::setLowerBoundary(double lower, bool isopen) {
			lower_.setBoundary(lower,ISLOWER,isopen);
		}

		/***********************************************************************/
		/**
		 * Sets the upper boundary to a given value. This function is private, as
		 * it should not be called independently of the lower boundary.
		 *
		 * @param upper The value of the upper boundary
		 * @param isopen Indicates whether this is an open (true) or closed (false) boundary
		 */
		void GRange::setUpperBoundary(double upper, bool isopen) {
			upper_.setBoundary(upper,ISUPPER,isopen);
		}

		/***********************************************************************/
		/**
		 * Retrieves the (corrected) value of the lower boundary
		 *
		 * @return The corrected value of the lower boundary
		 */
		double GRange::lowerBoundary() {
			return lower_.getBoundary();
		}

		/***********************************************************************/
		/**
		 * Retrieves the (corrected) value of the upper boundary
		 *
		 * @return The corrected value of the lower boundary
		 */
		double GRange::upperBoundary() {
			return upper_.getBoundary();
		}

		/***********************************************************************/
		/**
		 * Calculates the width of the range
		 *
		 * @return The width of the range
		 */
		double GRange::width() {
			double result;

			result=upperBoundary()-lowerBoundary();

			if(result < 0.) // This would be a rounding problem
			{
				std::ostringstream error;
				error << "In GRange::width(): Error!" << std::endl
					  << "Invalid range boundaries : " << std::endl
					  << "upper boundary: " << upperBoundary() << std::endl
					  << "lower boundary: " << lowerBoundary() << std::endl
					  << "width: " << result << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				// throw an exception. Add some information so that if the exception
				// is caught through a base object, no information is lost.
				throw geneva_invalid_range_data() << error_string(error.str());
			}

			return result;
		}

		/***********************************************************************/
		/**
		 * Checks whether a given value is in this range.
		 *
		 * @param val The value to be checked
		 * @return A boolean indicating whether or not the value is in the range
		 */
		bool GRange::isIn(double val) {
			if(val >= lowerBoundary() && val <= upperBoundary()) return true;
			return false;
		}

		/***********************************************************************/
		/**
		 *  Checks whether this GRange overlaps with another.
		 *
		 *  @param other A GRange object to check for a possible overlap
		 *  @return A boolean indicating whether there is an overlap
		 */
		bool GRange::overlaps(GRange& other) {
			// Check containment
			if(contains(other) || other.contains(*this)) return true;

			// Check partial overlap: one boundary is inside the others'
			// boundaries, while the other is not
			return ((lowerBoundary() <= other.lowerBoundary() && \
					 upperBoundary()> other.lowerBoundary() && \
					 upperBoundary() <= other.upperBoundary()) || \
					(other.lowerBoundary() <= lowerBoundary() && \
					 other.upperBoundary()> lowerBoundary() && \
					 other.upperBoundary() <= upperBoundary())) \
					 ?true \
					 :false;
		}

		/***********************************************************************/
		/**
		 * Checks whether this range contains another range.
		 *
		 * @param other A GRange object that is checked for containment
		 * @return A boolean indicating whether the other object is contained in this range
		 */
		bool GRange::contains(GRange& other) {
			return (other.lowerBoundary() >= this->lowerBoundary()
				 && other.upperBoundary() <= this->upperBoundary()) \
				  ?true \
				  :false;
		}

		/***********************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
