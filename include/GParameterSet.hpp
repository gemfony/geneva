/**
 * @file GParameterSet.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
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
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/** @brief Registers an evaluation function */
	void registerEvaluator(const boost::function<double (const GParameterSet&)>&);

	/**********************************************************************/
	/**
	 * If compiled in debug mode, this function performs all necessary error
	 * checks on the conversion from GParameterBase to the desired parameter type.
	 * Note that, if compiled in debug mode, this function will throw. Otherwise
	 * a segfault may be the result if a faulty conversion was attempted. Hence
	 * it is suggested to test your programs in debug mode before using it in a
	 * production environment.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GParameterBase object, as required by the user
	 */
	template <class parameter_type>
	inline boost::shared_ptr<parameter_type> parameterbase_cast(std::size_t pos){
#ifdef DEBUG
		// Extract data. at() will throw if we have tried to access a position in the
		// vector that does not exist.
		boost::shared_ptr<GParameterBase> data_base = data.at(pos);

		// Convert to the desired target type
		boost::shared_ptr<parameter_type> p_load = boost::dynamic_pointer_cast<parameter_type>(data_base);

		// Check that the conversion worked. dynamic_cast emits an empty pointer,
		// if this was not the case.
		if(!p_load){
			std::ostringstream error;
			error << "In GParameterSet::parameterbase_case<parameter_type>() : Conversion error!" << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
		}

		return p_load;
#else
		return boost::static_pointer_cast<parameter_type>(data[pos]);
#endif /* DEBUG */
	}

	/**********************************************************************/

protected:
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation();

	/** @brief Allows to store an evaluation function for this object */
	boost::function<double (const GParameterSet&)> eval_;
};

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GPARAMETERSET_HPP_ */
