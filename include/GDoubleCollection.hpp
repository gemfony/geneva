/**
 * @file
 */

/* GDoubleCollection.hpp
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

// Standard header files go here
#include <string>
#include <sstream>
#include <vector>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GDOUBLECOLLECTION_HPP_
#define GDOUBLECOLLECTION_HPP_

// GenEvA header files go here

#include "GObject.hpp"
#include "GParameterCollectionT.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GRandom.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

const double DEFINIT = 100.;

/**********************************************************************/
/**
 * This class represents a collection of double values, all modified
 * using the same algorithm. Using the framework provided by GTemplateValueCollection
 * and GDoubleGaussAdaptor, this class becomes rather simple.
 */
class GDoubleCollection
	: public GParameterCollectionT<double>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterCollectionT",	boost::serialization::base_object<GParameterCollectionT<double> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDoubleCollection();
	/** @brief Initialize with a number of random numbers */
	explicit GDoubleCollection(std::size_t);
	/** @brief Initialize with a number of random values
	 * within given boundaries */
	GDoubleCollection(std::size_t, double, double);
	/** @brief The standard copy constructor */
	GDoubleCollection(const GDoubleCollection&);
	/** @brief The standard destructor */
	virtual ~GDoubleCollection();

	/** @brief The standard assignment operator */
	const GDoubleCollection& operator=(const GDoubleCollection&);

	/** @brief Creates a deep copy of this object. */
	virtual GObject *clone();
	/** @brief Loads the data fron another GDoubleCollection object */
	virtual void load(const GObject *);

	/** @brief Appends double values in a given range */
	void addData(std::size_t, double, double);
};

/**********************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /*GDOUBLECOLLECTION_HPP_*/
