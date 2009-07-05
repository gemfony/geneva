/**
 * @file GIndividualSet.hpp
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

#include <boost/cstdint.hpp>


#ifndef GINDIVIDUALSET_HPP_
#define GINDIVIDUALSET_HPP_

// GenEvA headers go here
#include "GMutableSetT.hpp"
#include "GIndividual.hpp"
#include "GObject.hpp"
#include "GRandom.hpp"

namespace Gem {
namespace GenEvA {

/**
 * The default base name used for checkpointing. Derivatives of this
 * class can build distinguished filenames from this e.g. by adding
 * the current generation.
 */
const std::string DEFAULTCPBASENAME = "geneva.cp";

/**
 * The default directory used for checkpointing. We choose a directory
 * that will always exist.
 */
const std::string DEFAULTCPDIR = "./";

class GIndividualSet
	:public GMutableSetT<Gem::GenEvA::GIndividual>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GMutableSetT_GIndividual", boost::serialization::base_object<GMutableSetT<Gem::GenEvA::GIndividual> >(*this));
	  ar & make_nvp("gr", gr);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GIndividualSet();
	/** @brief The copy constructor */
	GIndividualSet(const GIndividualSet&);
	/** @brief The destructor */
	virtual ~GIndividualSet();

	/** @brief Saves the state of the class to disc */
	virtual void saveCheckpoint() const = 0;
	/** @brief Loads the state of the class from disc */
	virtual void loadCheckpoint(const std::string&) = 0;

	/** @brief Checks for equality with another GIndividualSet object */
	bool operator==(const GIndividualSet&) const;
	/** @brief Checks for inequality with another GIndividualSet object */
	bool operator!=(const GIndividualSet&) const;
	/** @brief Checks for equality with another GIndividualSet object */
	virtual bool isEqualTo(const GObject&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GIndividualSet object */
	virtual bool isSimilarTo(const GObject&, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Determines whether production of random numbers should happen remotely (RNRFACTORY) or locally (RNRLOCAL) */
	void setRnrGenerationMode(const Gem::Util::rnrGenerationMode&);
	/** @brief Retrieves the random number generators current generation mode. */
	Gem::Util::rnrGenerationMode getRnrGenerationMode() const;


	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() const = 0;
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/**********************************************************************/
	/**
	 * If compiled in debug mode, this function performs all necessary error
	 * checks on the conversion from GIndividual to the desired parameter type.
	 * Note that, if compiled in debug mode, this function will throw. Otherwise
	 * a segfault may be the result if a faulty conversion was attempted. Hence
	 * it is suggested to test your programs in debug mode before using it in a
	 * production environment.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GIndividual object, as required by the user
	 */
	template <typename parameter_type>
	boost::shared_ptr<parameter_type> individual_cast(const std::size_t& pos){
#ifdef DEBUG
		// Extract data. at() will throw if we have tried to access a position in the
		// vector that does not exist.
		boost::shared_ptr<GIndividual> data_base = data.at(pos);

		// Convert to the desired target type
		boost::shared_ptr<parameter_type> p_load = boost::dynamic_pointer_cast<parameter_type>(data_base);

		// Check that the conversion worked. dynamic_cast emits an empty pointer,
		// if this was not the case.
		if(!p_load){
			std::ostringstream error;
			error << "In GIndividualSet::individual_cast<parameter_type>() : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		return p_load;
#else
		return boost::static_pointer_cast<parameter_type>(data[pos]);
#endif /* DEBUG */
	}

protected:
	/***********************************************************************************/
    /**
     * A random number generator. Note that the actual calculation is possibly
     * done in a random number server. GRandom also has a local generator
     * in case the factory is unreachable, or local storage of random
     * number containers requires too much memory.
     */
	Gem::Util::GRandom gr;

	/**********************************************************************************/
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() = 0;
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GIndividualSet)

/**************************************************************************************************/

#endif /* GINDIVIDUALSET_HPP_ */
