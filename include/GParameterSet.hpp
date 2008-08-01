/**
 * @file
 */

/* GParameterSet.hpp
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

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>

#ifndef GPARAMETERSET_HPP_
#define GPARAMETERSET_HPP_

// GenEvA headers go here

#include "GObject.hpp"
#include "GMutableSetT.hpp"
#include "GParameterBase.hpp"
#include "GEvaluator.hpp"
#include "GHelperFunctionsT.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/**************************************************************************/
/**
 * This class implements a collection of GParameterBase objects. It
 * will form the basis of many user-defined individuals.
 */
class GParameterSet:
	public GMutableSetT<Gem::GenEvA::GParameterBase>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GMutableSetT_GParameterBase",
			  boost::serialization::base_object<GMutableSetT<Gem::GenEvA::GParameterBase> >(*this));
	}
	///////////////////////////////////////////////////////////////////////
public:
	/** @brief The default constructor */
	GParameterSet();
	/** @brief The copy constructor */
	GParameterSet(const GParameterSet&);
	/** @brief The destructor */
	virtual ~GParameterSet();

	/** @brief Standard assignment operator */
	const GParameterSet& operator=(const GParameterSet&);

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone();
	/** @brief Resets the class to its initial state */
	virtual void reset();
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/** @brief Registers a GEvaluator object */
	void registerEvaluator(GEvaluator *);

protected:
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation();

	boost::shared_ptr<GEvaluator> eval_;
};

}} /* namespace Gem::GenEvA */

#endif /* GPARAMETERSET_HPP_ */
