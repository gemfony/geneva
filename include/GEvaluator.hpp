/**
 * @file
 */

/* GEvaluator.hpp
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
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/function.hpp>

#ifndef GEVALUATOR_HPP_
#define GEVALUATOR_HPP_

// GenEvA headers go here
#include "GObject.hpp"
#include "GLogger.hpp"

namespace Gem {
namespace GenEvA {

/** @brief Forward declaration of GParameterSet */
class GParameterSet;

/**************************************************************************/
/**
 * An interface class which can be used to evaluate GParameterSet objects.
 * Note that evaluation functions will only get access to public, const
 * functions of the GParameterSet objects. Essentially this means using constant
 * vector functions. The class has been derived from GObject, so that it
 * has the clone() and load() functions and is serializable. A non-abstract
 * implementation is provided so that users have faster access to results.
 * Note, though, that Boost.function objects cannot be serialized, so this mode
 * cannot be used in a networked environment. You need to overload the eval()
 * function instead in this case.
 */
class GEvaluator
	:public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GObject", boost::serialization::base_object<GObject>(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GEvaluator();
	/** @brief The copy constructor */
	GEvaluator(const GEvaluator&);
	/** @brief Indicate that derivatives are virtual */
	virtual ~GEvaluator();

	/** @brief A standard assignment operator */
	const GEvaluator& operator=(const GEvaluator&);

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone();
	/** @brief Loads the data of another GEvaluator, camouflaged as a GObject */
	virtual void load(const GObject*);

	/** @brief Overload this function to allow evaluation of GParameterSet objects */
	virtual double eval(const GParameterSet&);

	/** @brief Registers an evaluation function */
	void registerEvalFunction(const boost::function<double (const GParameterSet&)>);

private:
	/**
	 * Allows to store a function object with the GEvaluator interface.
	 */
	boost::function<double (const GParameterSet&)> eval_;
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GEVALUATOR_HPP_ */
