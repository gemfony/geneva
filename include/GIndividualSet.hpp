/**
 * @file
 */

/* GIndividualSet.hpp
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


#ifndef GINDIVIDUALSET_HPP_
#define GINDIVIDUALSET_HPP_

// GenEvA headers go here
#include "GMutableSetT.hpp"
#include "GIndividual.hpp"
#include "GObject.hpp"

namespace Gem {
namespace GenEvA {

class GIndividualSet
	:public GMutableSetT<Gem::GenEvA::GIndividual>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GMutableSetT_GIndividual",
			  boost::serialization::base_object<GMutableSetT<Gem::GenEvA::GIndividual> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GIndividualSet();
	/** @brief The copy constructor */
	GIndividualSet(const GIndividualSet&);
	/** @brief The destructor */
	virtual ~GIndividualSet();

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() = 0;
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

protected:
	/**********************************************************************************/
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() = 0;
};

}} /* namespace Gem::GenEvA */

/**************************************************************************************************/
/**
 * Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GIndividualSet)

/**************************************************************************************************/

#endif /* GINDIVIDUALSET_HPP_ */
