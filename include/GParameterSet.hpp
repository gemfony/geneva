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

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>

#ifndef GPARAMETERSET_HPP_
#define GPARAMETERSET_HPP_

// GenEvA headers go here

#include "GObject.hpp"
#include "GMutableSetT.hpp"
#include "GParameterBase.hpp"
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

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
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

	/** @brief Checks for equality with another GParameterSet object */
	bool operator==(const GParameterSet&) const;
	/** @brief Checks for inequality with another GParameterSet object */
	bool operator!=(const GParameterSet&) const;
	/** @brief Checks for equality with another GParameterSet object */
	virtual bool isEqualTo(const GObject&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GParameterSet object */
	virtual bool isSimilarTo(const GObject&, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() const;
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/** @brief Registers an evaluation function */
	void registerEvaluator(const boost::function<double (const GParameterSet&)>&);

	/**********************************************************************/
	/**
	 * If compiled in debug mode, this function performs all necessary error
	 * checks on the conversion from a GParameterBase base pointer  to the desired
	 * parameter type (usually a collection). Note that, if compiled in debug mode, this
	 * function will throw. Otherwise a segfault may be the result if a faulty conversion was attempted.
	 * Hence it is suggested to test your programs in debug mode before using it in a
	 * production environment. "pc" stands for "parameter collection".
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GParameterBase object, as required by the user
	 */
	template <typename parameter_type>
	inline const boost::shared_ptr<parameter_type> pc_at(const std::size_t& pos)  const {
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
			error << "In GParameterSet::at<parameter_type>(pos) : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
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
