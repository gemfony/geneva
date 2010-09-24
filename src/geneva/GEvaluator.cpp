/**
 * @file GEvaluator.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "geneva/GEvaluator.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::Geneva::GEvaluator)

namespace Gem
{
namespace Geneva
{

/** @brief The default constructor. Creates some standard adaptors for different types */
GEvaluator::GEvaluator();
/** @brief A copy constructor */
GEvaluator::GEvaluator(const GEvaluator&);
/** @brief The destructor */
GEvaluator::~GEvaluator();

/** @brief Standard assignment operator */
const GEvaluator& operator=(const GEvaluator&);

/** @brief Checks for equality with another GEvaluator object */
bool operator==(const GEvaluator&) const;
/** @brief Checks for inequality with another GEvaluator object */
bool operator!=(const GEvaluator&) const;

/** @brief Checks whether this object fulfills a given expectation in relation to another object */
boost::optional<std::string> checkRelationshipWith(
		const GObject&
		, const Gem::Common::expectation&
		, const double&
		, const std::string&
		, const std::string&
		, const bool&
) const;

/** @brief Loads the data of another GObject */
void load_(const GObject*);
/** @brief Creates a deep clone of this object */
GObject* clone_() const;

template <>	void GEvaluator::registerFPParameter<double>();
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&);
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&, const double&);
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&, const double&, const double&);
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&, const double&, const double&, const double&);

template <> void   GEvaluator::setInitBoundaries<double>(const double&, const double&);
template <> double GEvaluator::getLowerInitBoundary<double>() const;
template <> double GEvaluator::getUpperInitBoundary<double>() const;

template <>	void GEvaluator::registerIntParameter<boost::int32_t>();
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&);
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&, const boost::int32_t&);
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&, const boost::int32_t&, const boost::int32_t&);
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&, const boost::int32_t&, const boost::int32_t&, const boost::int32_t&);

template <> void   GEvaluator::setInitBoundaries<boost::int32_t>(const boost::int32_t&, const boost::int32_t&);
template <> boost::int32_t GEvaluator::getLowerInitBoundary<boost::int32_t>() const;
template <> boost::int32_t GEvaluator::getUpperInitBoundary<boost::int32_t>() const;

/** @brief Adding a signle, randomly initialized boolean variable. */
void GEvaluator::registerBooleanParameter();

/** @brief Adding nPars randomly initialized boolean variable. */
void GEvaluator::registerBooleanParameter(const std::size_t&);

/** @brief Adding nPars boolean variable with defined start value */
void GEvaluator::registerBooleanParameter(const bool&, const std::size_t& nPars);


#ifdef GENEVATESTING
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	bool GEvaluator::modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	void GEvaluator::specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	void GEvaluator::specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */

} /* namespace Geneva */
} /* namespace Gem */
