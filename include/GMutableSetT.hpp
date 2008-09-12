/**
 * @file GMutableSetT.hpp
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
#include <vector>
#include <typeinfo>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GMUTABLESETT_HPP_
#define GMUTABLESETT_HPP_

#include "GIndividual.hpp"
#include "GParameterBase.hpp"
#include "GObject.hpp"

namespace Gem {
namespace GenEvA {

/******************************************************************************************/
/**
 * This class forms the basis for many user-defined individuals. It acts as a collection
 * of different parameter sets. User individuals can thus contain a mix of parameters of
 * different types, such as double, GenEvA::bit, long, ... . Derived classes must implement
 * a useful operator=(). It is also assumed that template arguments have the GObject and the
 * GMutableI interfaces, in particular the load(), clone() and mutate() functions.
 */
template <typename T>
class GMutableSetT:
	public GIndividual
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GIndividual",boost::serialization::base_object<GIndividual>(*this));
      ar & make_nvp("data",data);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**********************************************************************************/
	/**
	 * Syntactic sugar.
	 */
	typedef boost::shared_ptr<T> tobj_ptr;

	/**********************************************************************************/
	/**
	 * The default constructor. No local data, hence nothing to do.
	 */
	GMutableSetT() :GIndividual()
	{ /* nothing */	}

	/**********************************************************************************/
	/**
	 * The copy constructor. Note that we cannot rely on the operator=() of the vector
	 * here, as we do not know the actual type of T objects.
	 *
	 * @param cp A copy of another GMutableSetT<T> object
	 */
	GMutableSetT(const GMutableSetT<T>& cp):
		GIndividual(cp)
	{
		typename std::vector<tobj_ptr>::const_iterator itc;
		for(itc=cp.data.begin(); itc!=cp.data.end(); ++itc)
			data.push_back((*itc)->clone_bptr_cast<T>());
	}

	/**********************************************************************************/
	/**
	 * The destructor. As we use smart pointers, the std::vector will take care of
	 * clearing the vector.
	 */
	virtual ~GMutableSetT()
	{ /* nothing */ }

	/**********************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() = 0;

	/**********************************************************************************/
	/**
	 * Loads the data of another GParameterBase object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GMutableSetT object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// Convert cp into local format
	    const GMutableSetT<T> *gms_load = this->conversion_cast(cp, this);

	    // No local data - first load the GIndividual data
	    GIndividual::load(cp);

	    // For the vector we need to take into account several cases, as
	    // we do not want to assume anything for the GParameterBase objects

		// Copy all objects with identical type, then check how many objects were copied
		typename std::vector<tobj_ptr>::iterator it_this;
		typename std::vector<tobj_ptr>::const_iterator it_gms;

		// Check that the types are the same and if so, copy the data over. In the majority of
		// cases the rest of the code of this function should not get touched.
		for (it_gms=gms_load->data.begin(), it_this=data.begin();
			(it_gms!=gms_load->data.end() && it_this!=data.end());
			++it_gms, ++it_this)
		{
			// The same ? Just copy it over
			if ( typeid(*(it_gms->get())) == typeid(*(it_this->get())) ) {
				(*it_this)->load(it_gms->get());
			}
			// Differing types ? Need to reset the target item
			else {
				*it_this = (*it_gms)->clone_bptr_cast<T>();
			}
		}

		// Now we know that min(gms_load->size(),this->size()) items have been copied

		// Likely the following is the most frequent case.
		if (gms_load->data.size() == data.size())	return;

		// As this code will only rarely be called, we store the sizes here and not above
		std::size_t gms_sz = gms_load->data.size(), this_sz = data.size();

		// gms_sz > this_sz ? Great, we can just copy the rest of the objects over
		if (gms_sz > this_sz) {
			for(it_gms=gms_load->data.begin() + this_sz; it_gms!=gms_load->data.end(); ++it_gms){
				data.push_back((*it_gms)->clone_bptr_cast<T>());
			}
		}
		// this_sz > gms_sz ? We need to remove the surplus items. The
		// boost::shared_ptr will take care of the item's deletion.
		else if (this_sz > gms_sz) data.resize(gms_sz);

		return;
	}

	/**********************************************************************************/
	/**
	 *	Adds a boost::shared_ptr<T> to the set and checks its content.
	 *
	 *	@param item An item to be added to the set
	 */
	void append(const boost::shared_ptr<T>& item){
		if(item) data.push_back(item);
		else {
			std::ostringstream error;
			error << "In GMutableSetT::append():" << std::endl
			      << "Tried to add empty item" << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			throw geneva_error_condition() << error_string(error.str());
		}
	}

	/**********************************************************************************/
	/**
	 * The main data set stored in this class. This class was derived from std::vector<boost::shared_ptr<T> >
	 * in older versions of the GenEvA library. However, we want to avoid multiple inheritance in order to
	 * to allow an easier implementation of this library in other languages, such as C# or Java. And
	 * std::vector has a non-virtual destructor. Hence deriving from it is a) bad style and b) dangerous.
	 * Just like in the older setting, however, access to the data shall not be obstructed in any way.
	 * Providing the same interface without derivation or containment with this class would be
	 * error-prone and can be considered "syntactic sugar". Hence we do not follow this path.
	 */
	std::vector<tobj_ptr> data;

protected:
	/**********************************************************************************/
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() = 0;

	/**********************************************************************************/
	/**
	 * The actual mutation operations. Easy, as we know that all GParameterBase objects
	 * in this object must implement the mutate() function.
	 */
	virtual void customMutations(){
		typename std::vector<tobj_ptr>::iterator it;
		for(it=data.begin(); it!=data.end(); ++it) (*it)->mutate();
	}

	/**********************************************************************************/
};

}} /* namespace Gem::GenEvA */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GMutableSetT<T> > : boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GMutableSetT<T> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GMUTABLESETT_HPP_ */
