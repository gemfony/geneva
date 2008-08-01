/**
 * @file
 */

/* GMutableSetT.hpp
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
template <class T>
class GMutableSetT:
	public GIndividual,
	public std::vector<boost::shared_ptr<T> >
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GIndividual",boost::serialization::base_object<GIndividual>(*this));
      ar & make_nvp("vector_GParameterBase_ptr",boost::serialization::base_object<std::vector<boost::shared_ptr<T> > >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**
	 * Syntactic sugar.
	 */
	typedef boost::shared_ptr<T> tobj_ptr;

	/**********************************************************************************/
	/**
	 * The default constructor. No local data, hence nothing to do.
	 */
	GMutableSetT():
		GIndividual(),
		std::vector<tobj_ptr>()
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
		for(itc=cp.begin(); itc!=cp.end(); ++itc){
			// Extract a copy of the T object. Note that we assume that it has
			// the GObject interface and particularly the clone() function
			T* tobj = dynamic_cast<T *>((*itc)->clone());

			// Check whether the conversion has worked
			if(!tobj){
				std::ostringstream error;
				error << "In GMutableSetT(const GMutableSetT& cp):" << std::endl
					  << "Conversion error" << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
			}

			// We're safe and can attach the object to this class
			shared_ptr<T> p(tobj);
			this->push_back(p);
		}
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
	 * Resets the class to its initial state
	 */
	virtual void reset(){
		// No local data.

		// Reset our parent classes
		std::vector<tobj_ptr>::clear();
		GIndividual::reset();
	}

	/**********************************************************************************/
	/**
	 * Loads the data of another GParameterBase object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GMutableSetT object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
	    const GMutableSetT<T> *gms_load = checkConversion<GMutableSetT<T> >(cp, this);

	    // No local data - first load the GIndividual data
	    GIndividual::load(cp);

	    // For the vector we need to take into account several cases, as
	    // we do not want to assume anything for the GParameterBase objects

		// Copy all objects with identical type, then check how many ojects were copied
		typename std::vector<tobj_ptr>::iterator it_this;
		typename std::vector<tobj_ptr>::const_iterator it_gms;

		// Check that the types are the same and if so, copy the data over. In the majority of
		// cases the rest of the code of this function should not get touched.
		for (it_gms=gms_load->begin(), it_this=this->begin();
			(it_gms!=gms_load->end() && it_this!=this->end());
			++it_gms, ++it_this)
		{
			// The same ? Just copy it over
			if ( typeid(*(it_gms->get())) == typeid(*(it_this->get())) ) {
				(*it_this)->load(it_gms->get());
			}
			// Differing types ? Need to reset the target item
			else {
				// Extract a copy of the GParameterBase object
				T* tobj = dynamic_cast<T *>((*it_gms)->clone());

				// Check whether the conversion has worked
				if(!tobj){
					std::ostringstream error;
					error << "In GMutableSetT::load(const GObject*):" << std::endl
						  << "Conversion error" << std::endl;

					LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

					throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
				}

				// We're safe and can attach the object to this class
				tobj_ptr p(tobj);
				it_this->swap(p);
			}
		}

		// Now we know that min(gms_load->size(),this->size()) items have been copied

		// Likely the following is the most frequent case.
		if (gms_load->size() == this->size())	return;

		// As this code will only rarely be called, we store the sizes here and not above
		std::size_t gms_sz = gms_load->size(), this_sz = this->size();

		// gms_sz > this_sz ? Great, we can just copy the rest of the objects over
		if (gms_sz > this_sz) {
			for(it_gms=gms_load->begin() + this_sz; it_gms!=gms_load->end(); ++it_gms){
				// Extract a copy of the GParameterBase object
				T* tobj = dynamic_cast<T *>((*it_gms)->clone());

				// Check whether the conversion has worked
				if(!tobj){
					std::ostringstream error;
					error << "In GMutableSetT::load(const GObject*):" << std::endl
						  << "Conversion error" << std::endl;

					LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

					throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
				}

				// We're safe and can attach the object to this class
				tobj_ptr p(tobj);
				this->push_back(p);
			}
		}
		// this_sz > gms_sz ? We need to remove the surplus items. The
		// shared_ptr will take care of the item's deletion.
		else if (this_sz > gms_sz) this->resize(gms_sz);

		return;
	}

	/**********************************************************************************/
	/**
	 *	Creates a shared_ptr object from item and adds it to the set.
	 *
	 *	@param item An item to be added to the set
	 */
	void append(T *item){
		boost::shared_ptr<T> p(item);
		this->push_back(p);
	}

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
		for(it=this->begin(); it!=this->end(); ++it) (*it)->mutate();
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
    template<class T>
    struct is_abstract<Gem::GenEvA::GMutableSetT<T> > : boost::true_type {};
    template<class T>
    struct is_abstract< const Gem::GenEvA::GMutableSetT<T> > : boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GMUTABLESETT_HPP_ */
