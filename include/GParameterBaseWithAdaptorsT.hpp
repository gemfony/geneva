/**
 * @file GParameterBaseWithAdaptorsT.hpp
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
#include <algorithm>
#include <string>
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GPARAMETERBASEWITHADAPTORST_HPP_
#define GPARAMETERBASEWITHADAPTORST_HPP_

// GenEvA headers go here
#include "GParameterBase.hpp"
#include "GObject.hpp"
#include "GAdaptorT.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************************************/
/**
 * This is a templatized version of the GParameterBase class. Its main
 * addition over that class is the storage of adaptors, which allow the
 * mutation of parameters. As this functionality has to be type specific,
 * this class is also implemented as a template. Storing the adaptors in
 * the GParameterBase class would not have been possible, as it cannot be
 * templatized - it serves as a base class for the objects stored in the
 * GParameterSet collections.
 */
template <typename T>
class GParameterBaseWithAdaptorsT:
	public GParameterBase
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterBase", boost::serialization::base_object<GParameterBase>(*this));
		ar & make_nvp("adaptors_", adaptors_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	typedef std::vector<boost::shared_ptr<GAdaptorT<T> > > GATvec; ///< Readability

	/*******************************************************************************************/
	/**
	 * The default constructor.
	 */
	GParameterBaseWithAdaptorsT() :GParameterBase()
	{ /* nothing */	}

	/*******************************************************************************************/
	/**
	 * The copy constructor.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT object
	 */
	GParameterBaseWithAdaptorsT(const GParameterBaseWithAdaptorsT<T>& cp)
		:GParameterBase(cp)
	{
		copyAdaptors(cp.adaptors_, adaptors_);
	}

	/*******************************************************************************************/
	/**
	 * The standard destructor.
	 */
	virtual ~GParameterBaseWithAdaptorsT(){
		// boost::shared_ptr takes care of the deletion of
		// adaptors in the vector.
		adaptors_.clear();
	}

	/*******************************************************************************************/
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() = 0;

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GParameterBaseWithAdaptorsT<T>& cp) const {
		return GParameterBaseWithAdaptorsT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for inequality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GParameterBaseWithAdaptorsT<T>& cp) const {
		return !GParameterBaseWithAdaptorsT<T>::isEqualTo(cp);
	}

	/*******************************************************************************************/
	/**
	 * Checks for equality with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool isEqualTo(const GParameterBaseWithAdaptorsT<T>& cp) const {
		// Check equality of the parent class
		if(!GParameterBase::isEqualTo(cp)) return false;

		// Check sizes of the local containers
		if(adaptors_.size() != cp.adaptors_.size()) return false;

		// Then check individual items
		typename GATvec::const_iterator it;
		typename GATvec::const_iterator cp_it;
		for(it=adaptors_.begin(), cp_it=cp.adaptors_.begin();
			  it!=adaptors_.end(), cp_it!=cp.adaptors_.end(); ++it, ++cp_it)
		{
			if(!(*it)->isEqualTo(**cp_it)) return false;
		}

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Checks for similarity with another GParameterBaseWithAdaptorsT<T> object
	 *
	 * @param  cp A constant reference to another GParameterBaseWithAdaptorsT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	bool isSimilarTo(const GParameterBaseWithAdaptorsT<T>& cp, const double& limit=0) const {
		// Check similarity of the parent class
		if(!GParameterBase::isSimilarTo(cp, limit)) return false;

		// Check sizes of local containers
		if(adaptors_.size() != cp.adaptors_.size()) return false;

		// Then check individual items
		typename GATvec::const_iterator it;
		typename GATvec::const_iterator cp_it;
		for(it=adaptors_.begin(), cp_it=cp.adaptors_.begin();
			  it!=adaptors_.end(), cp_it!=cp.adaptors_.end(); ++it, ++cp_it)
		{
			if(!(*it)->isSimilarTo(**cp_it,limit)) return false;
		}

		return true;
	}

	/*******************************************************************************************/
	/**
	 * Loads the data of another GParameterBaseWithAdaptorsT object, which
	 * is camouflaged as a GObject.
	 *
	 * @param cp A copy of another GParameterBaseWithAdaptorsT, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// Convert cp into local format
		const  GParameterBaseWithAdaptorsT<T> *gpbwa = this->conversion_cast(cp, this);

		// Load our parent class'es data ...
		GParameterBase::load(cp);

		// ... and then our own data
		copyAdaptors(gpbwa->adaptors_, adaptors_);
	}

	/*******************************************************************************************/
	/**
	 * Adds an adaptor to the list. Please note that this class takes ownership of the adaptor.
	 * Thus, at the end of the lifetime, the adaptor will be destroyed.
	 *
	 * @param gat A boost::shared_ptr to an adaptor
	 */
	void addAdaptor(const boost::shared_ptr<GAdaptorT<T> >& gat) {
		// Check that we have indeed been given an adaptor
		if(!gat){
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::addAdaptor():" << std::endl
				  << "Error: Empty adaptor provided." << std::endl;

			throw geneva_error_condition(error.str());
		}

		typename GATvec::iterator pos;

		// Check that an adaptor with this name has not yet been added
		if (findAdaptor(gat->name(), pos)) {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::addAdaptor():" << std::endl
				  << "Error: duplicate adaptors: " << gat->name() << std::endl;

			throw geneva_error_condition(error.str());
		}

		boost::shared_ptr<GAdaptorT<T> > p(gat);
		adaptors_.push_back(p);
	}

	/*******************************************************************************************/
	/**
	 * Retrieves an adaptor by name.
	 *
	 * Note that this function only returns a boost::shared_ptr storing a base pointer. E.g., if T==double,
	 * then this function will return a boost::shared_ptr<GAdaptorT<double> >, not a boost::shared_ptr
	 * holding e.g. a GDoubleGaussAdaptor, which in turn is derived from GAdaptorT<double> > .
	 * Thus, when you call this function, you need to make sure to make the appropriate
	 * conversions yourself. This will be facilitated once a typeof() operator is available as part of the
	 * C++ standard.
	 *
	 * You can test whether an adaptor with the requested name was found -- boost::shared_ptr implements operator
	 * bool . This function returns a dummy pointer if no adaptor was found. Hence you can say something like
	 * boost::shared_ptr<T> p; if(p = getAdaptor("myName")) ...
	 *
	 * @param adName The name of an adaptor
	 * @return A boost::shared_ptr to the adaptor if found, otherwise an empty boost::shared_ptr
	 */
	boost::shared_ptr<GAdaptorT<T> > getAdaptor(const std::string& adName) {
		typename GATvec::iterator pos;

		// check that an adaptor with this name exists
		if (!findAdaptor(adName, pos)) return boost::shared_ptr<GAdaptorT<T> >();

		// Great - we have found the adaptor
		return *pos;
	}

	/*******************************************************************************************/
	/**
	 * When compiled in debug mode, performs all necessary checks of the conversion of the
	 * adaptor to the target type. Otherwise uses a faster static cast.
	 *
	 * @param adName The name of an adaptor
	 * @return The desired adaptor instance
	 */
	template <typename adaptor_type>
	inline boost::shared_ptr<adaptor_type> adaptor_cast(const std::string& adName) {
		// Extract adaptor . Will throw if we have tried to access a position in the
		// vector that does not exist.
		boost::shared_ptr<GAdaptorT<T> > adaptor_base = this->getAdaptor(adName);

		if(!adaptor_base) { // This should not be!
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::adaptor_cast<adaptor_type> : No adaptor found!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

#ifdef DEBUG
		// Convert to the desired target type
		boost::shared_ptr<adaptor_type> p_adaptor = boost::dynamic_pointer_cast<adaptor_type>(adaptor_base);

		// Check that the conversion worked. dynamic_cast emits an empty pointer,
		// if this was not the case.
		if(!p_adaptor){
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT::adaptor_cast<adaptor_type>() : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		return p_adaptor;
#else
		return boost::static_pointer_cast<adaptor_type>(adaptor_base);
#endif /* DEBUG */
	}

	/*******************************************************************************************/
	/**
	 * This function searches an adaptor by name in the list and, if found, erases it. It returns
	 * true in this case, false otherwise.
	 *
	 * \param adName The name of an adaptor
	 * \return A boolean indicating whether or not the adaptor was found and erased
	 */
	bool deleteAdaptor(const std::string& adName) {
		typename GATvec::iterator pos;

		// We know that only one adaptor with this name can be present (this is enforced
		// by addAdaptor() ), so all we have to do is search for its position (if
		// there is any).
		if (findAdaptor(adName, pos)) {
			adaptors_.erase(pos);
			return true;
		}

		// Nothing was found
		return false;
	}

	/*******************************************************************************************/
	/**
	 * Informs all adaptors whether their init function should be called for all members of a sequence.
	 * This is only important for value collections
	 *
	 * \param val Specifies whether the init function should be called for all members of a sequence
	 */
	void setAlwaysInit(const bool& val) {
		typename GATvec::iterator it;

		for (it = adaptors_.begin(); it != adaptors_.end(); ++it) {
			(*it)->setAlwaysInit(val);
		}
	}

	/*******************************************************************************************/
	/** @brief The mutate interface */
	virtual void mutate(void) = 0;

	/*******************************************************************************************/
	/**
	 * Gives access to the number of adaptors registered in this class
	 *
	 * @return The number of adaptors stored in this class
	 */
	std::size_t numberOfAdaptors(){
		return adaptors_.size();
	}

protected:
	/*******************************************************************************************/
	/**
	 * This function applies the first adaptor of the sequence to a value. Note that the parameter of
	 * this function will get changed.
	 *
	 * @param value The parameter to be mutated
	 */
	void applyFirstAdaptor(T &value) {
		if (!adaptors_.empty()) {
			adaptors_[0]->mutate(value);
		} else {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyFirstAdaptor(T& value):" << std::endl
				  << "Error: No adaptors were found." << std::endl;

			throw geneva_error_condition(error.str());
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies all adaptors in sequence to a value. Note that the parameter
	 * will be changed by this function..
	 *
	 * @param value The parameter to be mutated
	 */
	void applyAllAdaptors(T &value) {
		typename GATvec::iterator it;
		if (!adaptors_.empty()) {
			for (it = adaptors_.begin(); it != adaptors_.end(); ++it) {
				(*it)->mutate(value);
			}
		} else {
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyAllAdaptors(T& value):" << std::endl
				  << "Error: No adaptors were found." << std::endl;

			throw geneva_error_condition(error.str());
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies a named adaptor to a value. Note that the second
	 * parameter of this function will get changed.
	 *
	 * @param adName The name of the adaptor to apply to the value
	 * @param value The parameter to be mutated
	 */
	void applyNamedAdaptor(const std::string& adName, T &value) {
		typename GATvec::iterator pos;

		if (findAdaptor(adName, pos)) {
			(*pos)->mutate(value);
		} else { // Error - bad adaptor called
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyNamedAdaptor(string adName, T& value):" << std::endl
				  << "Error: Named adaptor " << adName << " was not found." << std::endl;

			throw geneva_error_condition(error.str());
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies the first adaptor of the adaptor sequence to a collection of values.
	 * Note that the parameter of this function will get changed. The check for adaptors happens
	 * inside applyFirstAdaptor.
	 *
	 * @param collection A vector of values that shall be mutated
	 */
	void applyFirstAdaptor(std::vector<T> &collection) {
		typename std::vector<T>::iterator it;
		for (it = collection.begin(); it != collection.end(); ++it)	applyFirstAdaptor(*it);
	}

	/*******************************************************************************************/
	/**
	 * This function applies all adaptors of the adaptor sequence to a collection of values. Note that
	 * the parameter of this function will get changed.
	 *
	 * @param collection A vector of values that shall be mutated
	 */
	void applyAllAdaptors(std::vector<T> &collection) {
		typename std::vector<T>::iterator it;
		for (it = collection.begin(); it != collection.end(); ++it) applyAllAdaptors(*it);
	}

	/*******************************************************************************************/
	/**
	 * This function applies a named adaptor to a collection of values. Note that the parameter of
	 * this function will get changed.
	 *
	 * @param adName The name of the adaptor to apply to the collection
	 * @param collection The vector of values that shall be mutated
	 */
	void applyNamedAdaptor(const std::string& adName, std::vector<T> &collection) {
		typename GATvec::iterator pos;
		typename std::vector<T>::iterator it;

		// Search for the adaptor
		if (findAdaptor(adName, pos)) {
			// Now apply the adaptor to all values of the collection in sequence
			for (it = collection.begin(); it != collection.end(); ++it) {
				(*pos)->mutate(*it);
			}
		} else { // Error - bad adaptor called
			std::ostringstream error;
			error << "In GParameterBaseWithAdaptorsT<T>::applyNamedAdaptor(string adName, std::vector<T>& value):" << std::endl
				  << "Error: Named adaptor " << adName << " was not found." << std::endl;

			throw geneva_error_condition(error.str());
		}
	}

	/*******************************************************************************************/
	/**
	 * This function finds a named adaptor in the list and stores its position, if it was found. It
	 * returns true in this case, otherwise false.
	 *
	 * @param adName The name of an adaptor
	 * @param pos Its position in the list, if found
	 * @return A boolean indicating whether or not the adaptor was found
	 */
	bool findAdaptor(const std::string& adName, typename GATvec::iterator& pos) {
		typename GATvec::iterator ad;

		for (ad = adaptors_.begin(); ad != adaptors_.end(); ad++) {
			if (ad->get()->name() == adName) {
				pos = ad;
				return true;
			}
		}

		return false; // Nothing found
	}

private:
	/*******************************************************************************************/
	/**
	 * A private helper function that creates a copy of a vector of adaptors. The target vector
	 * is contained in another GParameterBaseWithAdaptorsT<T> object and thus will already contain
	 * other adaptors. Quite likely, however, as we are dealing with objects of the same origin, both
	 * sets of adaptors will have the same type. But even if they have the same type, they have local
	 * data, such as the stepwidth in the case of the GDoubleGaussAdaptor. We thus need to copy each
	 * adaptor over when we create a copy of this object. This function assumes that adaptors have
	 * unique names and uses this feature as a quick check to find out, whether adaptors in the
	 * same position have the same type.
	 *
	 * @param from The original vector that should be copied
	 * @param to The target vector
	 */
	void copyAdaptors(const GATvec& from, GATvec& to) {
		// Copy all adaptors with identical name, then check how many adaptors were copied
		typename GATvec::iterator itTo;
		typename GATvec::const_iterator itFrom;

		// Check that the names are the same and if so, copy the data over.
		for (itFrom = from.begin(), itTo = to.begin();
			(itFrom != from.end() && itTo != to.end());
			++itFrom, ++itTo) {

			// Identical type
			if ((*itFrom)->name() == (*itTo)->name()) {
				(*itTo)->load(itFrom->get());
			}
			// Different type - need to convert
			else {
				// Note to myself: The effectiveness of this command should be checked by a test case!!
				*itTo = (*itFrom)->clone_bptr_cast<GAdaptorT<T> >();
			}
		}

		if (from.size() == to.size()) return; // We're done. Likely the most frequent case.

		// As this code will only rarely be called, we store the sizes here and not above
		std::size_t fromSize = from.size(), toSize = to.size();

		// fromSize > toSize ? Great, we can just copy the rest of the adaptors over
		if (fromSize > toSize) {
			for (itFrom = from.begin() + toSize; itFrom != from.end(); ++itFrom) {
				to.push_back((*itFrom)->clone_bptr_cast<GAdaptorT<T> >());
			}
		}
		// toSize > fromSize ? We need to remove the surplus items. The
		// boost::shared_ptr will take care of the item's deletion.
		else if (toSize > fromSize)	to.resize(fromSize);

		return;
	}

	/*******************************************************************************************/
	/**
	 * This vector contains the adaptors used for mutations of the values stored
	 * in derived classes.
	 */
	GATvec adaptors_;
};

// Declaration of specializations for std::vector<bool>
template<> void GParameterBaseWithAdaptorsT<bool>::applyFirstAdaptor(std::vector<bool> &);
template<> void GParameterBaseWithAdaptorsT<bool>::applyAllAdaptors(std::vector<bool> &);
template<> void GParameterBaseWithAdaptorsT<bool>::applyNamedAdaptor(const std::string&, std::vector<bool> &);

} /* namespace GenEvA */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GParameterBaseWithAdaptorsT<T> > : boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GParameterBaseWithAdaptorsT<T> > : boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GPARAMETERBASEWITHADAPTORST_HPP_ */
