/**
 * @file
 */

/* GRange.hpp
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

// Standard headers go here
#include <cmath>
#include <sstream>

// Boost headers go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GRANGE_HPP_
#define GRANGE_HPP_

// Geneva headers go here
#include "GObject.hpp"
#include "GBoundary.hpp"
#include "GHelperFunctionsT.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/***********************************************************************/

const boost::int16_t DEFMINDIGITS = 5;
const boost::int16_t MAXDIGITS = 16;

const bool ISOPEN = true;
const bool ISCLOSED = false;
const bool ISUPPER = true;
const bool ISLOWER = false;

const double DEFAULTRANGE = 10.;

/**
 * A GRange represents a range of floating point values, with uper and lower,
 * open or closed boundaries (see the GBoundary class for a more detailed explanation).
 * A GRange object can be either active or inactive. This class represents this concept.
 * The class is mainly used in the context of the GDouble class.
 */
class GRange
	:public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GObject", boost::serialization::base_object<GObject>(
				*this));
		ar & make_nvp("lower_", GRange::lower_);
		ar & make_nvp("upper_", GRange::upper_);
		ar & make_nvp("isactive_", isactive_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GRange();
	/** @brief Allows to set all relevant values in one go */
	GRange(double, bool, double, bool);
	/** @brief A standard copy constructor */
	GRange(const GRange& cp);
	/** @brief The standard destructor */
	virtual ~GRange();

	/** @brief A standard assignment operator */
	const GRange& operator=(const GRange&);

	/** @brief Loads the data of another GRange object */
	virtual void load(const GObject *);
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone();
	/** @brief Resets the object to its initial state */
	void reset();

	/** @brief Marks the range as active */
	void setIsActive(bool);
	/** @brief Checks whether the range is active */
	bool isActive() const;

	/** @brief Tries to limit the range in such a way that a minimum
	 * number of decimal places is present */
	void setMinDigits(int16_t);

	/** @brief Sets the upper and lower limits in one go */
	void setBoundaries(double, bool, double, bool);

	/** @brief Retrieves the value of the lower boundary */
	double lowerBoundary();
	/** @brief Retrieves the value of the upper boundary */
	double upperBoundary();
	/** @brief Retrieves the width of the boundary */
	double width();

	/** @brief Checks whether a value is in the range */
	bool isIn(double);
	/** @brief Checks whether another range overlaps with this range */
	bool overlaps(GRange&);
	/** @brief Checks whether another range is contained in this range */
	bool contains(GRange&);

private:
	/** @brief Sets the lower boundary of the range */
	void setLowerBoundary(double, bool);
	/** @brief Sets the upper boundary of the range */
	void setUpperBoundary(double, bool);

	GBoundary lower_, upper_; ///< The lower and upper boundaries of the range
	bool isactive_; ///< A variable indicating whether the range is active
};

} /* namespace GenEvA */
} /* namespace Gem */
/***********************************************************************/

#endif /*GRANGE_HPP_*/
