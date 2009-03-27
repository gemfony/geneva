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
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
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
	 * The destructor. As we use smart pointers, we do not need to take care of the
	 * data in the vector ourselves.
	 */
	virtual ~GMutableSetT() {
		data.clear();
	}

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
	/////////////////// std::vector<tobj_ptr> interface (incomplete) ///////////////////
	/**********************************************************************************/
	// Typedefs
	typedef typename std::vector<tobj_ptr>::value_type value_type;
	typedef typename std::vector<tobj_ptr>::reference reference;
	typedef typename std::vector<tobj_ptr>::const_reference const_reference;
	typedef typename std::vector<tobj_ptr>::iterator iterator;
	typedef typename std::vector<tobj_ptr>::const_iterator const_iterator;
	typedef typename std::vector<tobj_ptr>::reverse_iterator reverse_iterator;
	typedef typename std::vector<tobj_ptr>::const_reverse_iterator const_reverse_iterator;
	typedef typename std::vector<tobj_ptr>::size_type size_type;

	// Non modifying access
	inline size_type size() const { return data.size(); }
	inline bool empty() const { return data.empty(); }
	inline size_type max_size() const { return data.max_size(); }

	inline size_type capacity() const { return data.capacity(); }
	inline void reserve(size_type amount) { data.reserve(amount); }

	// Access to elements (unchecked / checked)
	inline reference operator[](std::size_t pos) { return data[pos]; }
	inline const reference operator[](std::size_t pos) const { return data[pos]; }
	inline reference at(std::size_t pos) { return data.at(pos); }
	inline const reference at(std::size_t pos) const { return data.at(pos); }

	inline reference front() { return data.front(); }
	inline const_reference front() const { return data.front(); }

	inline reference back() { return data.back(); }
	inline const_reference back() const { return data.back(); }

	// Iterators
	inline iterator begin() { return data.begin(); }
	inline const_iterator begin() const { return data.begin(); }

	inline iterator end() { return data.end(); }
	inline const_iterator end() const { return data.end(); }

	inline iterator rbegin() { return data.rbegin(); }
	inline const_iterator rbegin() const { return data.rbegin(); }

	inline iterator rend() { return data.rend(); }
	inline const_iterator rend() const { return data.rend(); }

	// Adding to the  back of the vector
	inline void push_back(const tobj_ptr& item){
		if(!item){
			std::ostringstream error;
			error << "In GMutableSetT::push_back(const tobj_ptr&):" << std::endl
			      << "Tried to add empty item" << std::endl;

			throw geneva_error_condition(error.str());
		}

		data.push_back(item);
	}

	// Removing an element from the end of the vector
	inline void pop_back(){ data.pop_back(); }

	// Removal at a given position or in a range
	inline iterator erase(iterator pos) { return data.erase(pos); }
	inline iterator erase(iterator from, iterator to) { return data.erase(from,to); }

	// Resizing the vector, adding copies of item if necessary
	inline void resize(size_type amount, tobj_ptr item) {
		if(amount <= data.size())
			data.resize(amount);
		else {
			if(!item){
				std::ostringstream error;
				error << "In GMutableSetT::resize(size_type, tobj_ptr):" << std::endl
				      << "Tried to add empty item" << std::endl;

				throw geneva_error_condition(error.str());
			}

			for(size_type i=0; i<(amount - data.size()); i++) data.push_back(boost::shared_ptr<T>(new T(*(item.get()))));
		}
	}

	// Resizing to size 0
	inline void clear() { data.clear(); }

	/**********************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////
	/**********************************************************************************/

protected:
	/**********************************************************************************/
	/**
	 * The main data set stored in this class. This class was derived from std::vector<boost::shared_ptr<T> >
	 * in older versions of the GenEvA library. However, we want to avoid multiple inheritance in order to
	 * to allow an easier implementation of this library in other languages, such as C# or Java. And
	 * std::vector has a non-virtual destructor. Hence deriving from it is a) bad style and b) dangerous.
	 * Just like in the older setting, however, access to the data shall not be obstructed in any way.
	 * Hence we provide a (so far still incomplete) set of access functions with the std::vector API. Derived
	 * classes get free access.
	 */
	std::vector<tobj_ptr> data;

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

} /* namespace GenEvA */
} /* namespace Gem */

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
