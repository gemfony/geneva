/**
 * \file
 */

/**
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

#include <vector>
#include <algorithm>
#include <string>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#ifndef GMUTABLE_H_
#define GMUTABLE_H_

using namespace std;
using namespace boost;

#include "GObject.hpp"
#include "GMember.hpp"
#include "GTemplateAdaptor.hpp"

namespace GenEvA {

/****************************************************************************************************/
/**
 * This is the base class for all GenEvA values and value collections.
 * Its main feature is a collection of adaptors that can be applied to
 * the value(s). The class is implemented as a derivative of GMember
 * (rather than implementing the functionality in GMember itself) so that
 * it is more easily possible to store adaptors of arbitrary type through
 * the template mechanism.
 *
 * \todo Need to add throw() statements where possible. Need to cross-check
 * virtual statements
 */
template<class T>
class GMutable: public GMember {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GMember", boost::serialization::base_object<GMember>(
				*this));
		ar & make_nvp("adaptors_", adaptors_);
	}
	///////////////////////////////////////////////////////////////////////

public:

	typedef vector<boost::shared_ptr<GTemplateAdaptor<T> > > GTAvec;

	/*******************************************************************************************/
	/**
	 * The default constructor. As adaptors are given to us from the outside, no work has to be
	 * done here except for the initialisation of the parent class GMember.
	 */
	GMutable(void) throw() :
		GMember() { /* nothing */
	}

	/*******************************************************************************************/
	/**
	 * A standard copy constructor. The main data of this class is the list of adaptors.
	 * It may happen that the lists of adaptors are not identical. In this case we have
	 * to reset our entire list.
	 *
	 * \param cp Another GMutable<T> object
	 */
	GMutable(const GMutable<T>& cp) :
		GMember(cp) {
		GMutable<T>::copyAdaptors(cp.adaptors_, adaptors_);
	}

	/*******************************************************************************************/
	/**
	 * The destructor. As we are using smart pointers internally for the adaptors,
	 * we do not have to take care of their deletion.
	 */
	virtual ~GMutable() {
		adaptors_.clear();
	}

	/*******************************************************************************************/
	/**
	 * A standard assignment operator for GMutable<T> objects,
	 *
	 * \param cp A copy of another GMutable<T> object
	 */
	const GMutable<T>& operator=(const GMutable<T>& cp) {
		GMutable<T>::load(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * A standard assignment operator for GObject objects, camouflaged as a GObject.
	 *
	 * \param cp A copy of another GMutable<T> object, camouflaged as a GObject.
	 */
	virtual const GObject& operator=(const GObject& cp) {
		GMutable<T>::load(&cp);
		return *this;
	}

	/*******************************************************************************************/
	/**
	 * Resets the object to its initial state.
	 */
	virtual void reset(void) {
		// first reset our adaptor list ...
		adaptors_.clear();
		// ... and then the parent class
		GMember::reset();
	}

	/*******************************************************************************************/
	/**
	 * Here we load the data of another GMutable<T> object, camouflaged as a GObject.
	 *
	 * \param gb A pointer to another GMutable<T> object, camouflaged as a pointer to a GObject
	 */
	virtual void load(const GObject *gb) {
		// Convert argument to GMutable<T>
		const GMutable<T> *gm = dynamic_cast<const GMutable<T> *> (gb);

		// dynamic_cast will emit a NULL pointer, if the conversion failed
		if (!gm) {
			gls << "In GMutable<T>::load(): Conversion error!" << endl
					<< logLevel(CRITICAL);

			exit(1);
		}

		// Check that this object is not accidently assigned to itself.
		if (gm == this) {
			gls << "In GMutable<T>::load(): Error!" << endl
					<< "Tried to assign an object to itself." << endl
					<< logLevel(CRITICAL);

			exit(1);
		}

		// Now we can load the actual data
		GMember::load(gb);
		GMutable<T>::copyAdaptors(gm->adaptors_, adaptors_);
	}

	/*******************************************************************************************/
	/** \brief Creates a deep clone of the class (purely virtual) */
	virtual GObject *clone(void)=0;

	/*******************************************************************************************/
	/**
	 * Adds an adaptor to the list. Please note that the GMutable<T> takes ownership of the adaptor and
	 * stores it in a shared_ptr. Thus, at the end of the lifetime, the adaptor will be destroyed.
	 *
	 * \param gta A pointer to an adaptor
	 */
	void addAdaptor(GTemplateAdaptor<T>* gta) {
		typename GTAvec::iterator pos;

		// check that an adaptor with this name has not yet been added
		if (findAdaptor(gta->name(), pos)) {
			gls << "In GMutable<T>::addAdaptor():" << endl
					<< "Error: duplicate adaptors: " << gta->name() << endl
					<< logLevel(CRITICAL);

			exit(1);
		}

		boost::shared_ptr<GTemplateAdaptor<T> > p(gta);
		adaptors_.push_back(p);
	}

	/*******************************************************************************************/
	/**
	 * Note that this function only returns a shared_ptr storing a base pointer. E.g., if T==double,
	 * then this function will return a shared_ptr<GTemplateAdaptor<double> >, not (as required e.g.
	 * in GBitFlipAdaptor::setInternalMutationParameters(double,double,double) ) a shared_ptr
	 * holding a GDoubleGaussAdaptor, which in turn is derived from GTemplateAdaptor<double> > .
	 * Thus, when you call this function, you need to make sure to make the appropriate
	 * conversions yourself.
	 *
	 * You can test whether an adaptor with this name was found -- shared_ptr implements operator bool .
	 * This function returns a dummy pointer if no adaptor was found. Hence you can say something like
	 * shared_ptr<T> p; if(p = getAdaptor("myName")) ...
	 *
	 * \param adName The name of an adaptor
	 * \return A shared_ptr to the adaptor, if found, otherwise a dummy shared_ptr
	 */
	shared_ptr<GTemplateAdaptor<T> > getAdaptor(const string& adName) {
		typename GTAvec::iterator pos;

		// check that an adaptor with this name exists
		if (!findAdaptor(adName, pos)) {
			shared_ptr<GTemplateAdaptor<T> > dummy;
			return dummy;
		}

		// Great - we have found the adaptor
		return *pos;
	}

	/*******************************************************************************************/
	/**
	 * This function searches an adaptor by name in the list and, if found erases it. It returns true in
	 * this case, false otherwise.
	 *
	 * \param adName The name of an adaptor
	 * \return A boolean indicating whether or not the adaptor was found and erased
	 */
	bool deleteAdaptor(const string& adName) {
		typename GTAvec::iterator pos;

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
	 * This is only important for value collections and is used e.g. in the context of the GDoubleCollection
	 * class.
	 *
	 * \param val Specifies whether the init function should be called for all members of a sequence
	 */
	void setAlwaysInit(bool val) {
		typename GTAvec::iterator it;

		for (it = adaptors_.begin(); it != adaptors_.end(); ++it) {
			(*it)->setAlwaysInit(val);
		}
	}

	/*******************************************************************************************/
	/**
	 * This function assembles a report about the classes internal state and then calls the
	 * parent classes' assembleReport() function.
	 *
	 * \param indention The number of white spaces in front of each output line
	 * \return A string containing a report about the inner state of the object
	 */
	virtual string assembleReport(uint16_t indention) const {
		ostringstream oss;
		oss << ws(indention) << "GMutable<T>: " << this << endl
				<< ws(indention) << "-----> Report from adaptors" << endl;

		typename GTAvec::const_iterator it;
		int16_t count;
		for (it = adaptors_.begin(), count = 0; it != adaptors_.end(); ++it, ++count) {
			oss << ws(indention) << "++++++++++ adaptor " << count
					<< " +++++++++++" << endl << (*it)->assembleReport(
					indention + NINDENTION) << endl;
		}

		oss << ws(indention) << "-----> Report from parent class GMember : "
				<< endl << GMember::assembleReport(indention + NINDENTION)
				<< endl;

		return oss.str();
	}

	/*******************************************************************************************/

protected:

	/*******************************************************************************************/
	/**
	 * This function applies the first adaptor of the sequence to a value. Note that the parameter of
	 * this function will get changed.
	 *
	 * \param value The parameter to be mutated
	 */
	void applyFirstAdaptor(T &value) {
		if (adaptors_.size() > 0) {
			adaptors_[0]->mutate(value);
		} else {
			gls << "In GMutable<T>::applyFirstAdaptor():" << endl
					<< "Error: 0 adaptors found" << endl << logLevel(CRITICAL);

			exit(1);
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies all adaptors in sequence to a value. Note that the parameter of
	 * this function will get changed.
	 *
	 * \param value The parameter to be mutated
	 */
	void applyAllAdaptors(T &value) {
		typename GTAvec::iterator it;
		if (!adaptors_.empty()) {
			for (it = adaptors_.begin(); it != adaptors_.end(); ++it) {
				(*it)->mutate(value);
			}
		} else {
			gls << "In GMutable<T>::applyAllAdaptors():" << endl
					<< "Error: 0 adaptors found" << endl << logLevel(CRITICAL);

			exit(1);
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies a named adaptor to a value. Note that the second
	 * parameter of this function will get changed.
	 *
	 * \param adName The name of the adaptor to apply to the value
	 * \param value The parameter to be mutated
	 */
	void applyNamedAdaptor(string adName, T &value) {
		typename GTAvec::iterator pos;

		if (findAdaptor(adName, pos)) {
			(*pos)->mutate(value);
		} else { // Error - bad adaptor called
			gls << "In GMutable<T>::applyNamedAdaptor(): Error!" << endl
					<< "Desired adaptor not available: " << adName << endl
					<< logLevel(CRITICAL);

			exit(1);
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies the first adaptor of the adaptor sequence to a collection of values. Note that
	 * the parameter of this function will get changed.
	 *
	 * \param collection A vector of values that shall be mutated
	 */
	void applyFirstAdaptor(vector<T> &collection) {
		typename vector<T>::iterator it;
		if (!adaptors_.empty()) {
			for (it = collection.begin(); it != collection.end(); ++it) {
				applyFirstAdaptor(*it);
			}
		} else {
			gls << "In GMutable<T>::applyFirstAdaptor(vector<T>&):" << endl
					<< "Error: 0 adaptors found" << endl << logLevel(CRITICAL);

			exit(1);
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies all adaptors of the adaptor sequence to a collection of values. Note that
	 * the parameter of this function will get changed.
	 *
	 * \param collection A vector of values that shall be mutated
	 */
	void applyAllAdaptors(vector<T> &collection) {
		typename vector<T>::iterator it;
		if (!adaptors_.empty()) {
			for (it = collection.begin(); it != collection.end(); ++it) {
				applyAllAdaptors(*it);
			}
		} else {
			gls << "In GMutable<T>::applyAllAdaptors(vector<T>&):" << endl
					<< "Error: 0 adaptors found" << endl << logLevel(CRITICAL);

			exit(1);
		}
	}

	/*******************************************************************************************/
	/**
	 * This function applies a named adaptor to a collection of values. Note that the parameter of
	 * this function will get changed.
	 *
	 * \param adName The name of the adaptor to apply to the collection
	 * \param collection The vector of values that shall be mutated
	 */
	void applyNamedAdaptor(string adName, vector<T> &collection) {
		typename GTAvec::iterator pos;
		typename vector<T>::iterator it;

		// Search for the adaptor
		if (findAdaptor(adName, pos)) {
			// Now apply the adaptor to all values of the collection in sequence
			for (it = collection.begin(); it != collection.end(); ++it) {
				(*pos)->mutate(*it);
			}
		} else { // Error - bad adaptor called
			gls << "In GMutable<T>::applyNamedAdaptor(vector<T>&): Error!"
					<< endl << "Desired adaptor not available: " << adName
					<< endl << logLevel(CRITICAL);

			exit(1);
		}
	}

	/*******************************************************************************************/
	/**
	 * This function finds a named adaptor in the list and stores its position, if it was found. It
	 * returns true in this case, otherwise false.
	 *
	 * \param adName The name of an adaptor
	 * \param pos Its position in the list, if found
	 * \return A boolean indicating whether or not the adaptor was found
	 */
	bool findAdaptor(string adName, typename GTAvec::iterator& pos) {
		typename GTAvec::iterator ad;

		for (ad = adaptors_.begin(); ad != adaptors_.end(); ad++) {
			if (ad->get()->name() == adName) {
				pos = ad;
				return true;
			}
		}

		return false; // Nothing found
	}

	/*******************************************************************************************/

private:

	/*******************************************************************************************/
	/**
	 * A private helper function that creates a copy of a vector of adaptors. The target vector
	 * is contained in another GMutable<T> object and thus will already contain other adaptors.
	 * Quite likely, however, as we are dealing with objects of the same origin, both sets of
	 * adaptors will have the same type. But even if they have the same type, they have local data,
	 * such as the stepwidth in the case of the GDoubleGaussAdaptor. We thus need to copy each
	 * adaptor over when we create a copy of this object. The function assumes that adaptors have
	 * unique names and uses this feature as a quick check to find out, whether adaptors in the
	 * same position have the same type.
	 *
	 * \param from The original vector that should be copied
	 * \param to The target vector
	 */
	void copyAdaptors(const GTAvec& from, GTAvec& to) {
		// Copy all adaptors with identical name, then check how many adaptors were copied
		typename GTAvec::iterator itTo;
		typename GTAvec::const_iterator itFrom;

		// Check that the names are the same and if so, copy the data over. In the majority of
		// cases the rest of the code of this function should not get touched.
		for (itFrom = from.begin(), itTo = to.begin(); (itFrom != from.end()
				&& itTo != to.end()); ++itFrom, ++itTo) {

			// Identical type
			if ((*itFrom)->name() == (*itTo)->name()) {
				(*itTo)->load(itFrom->get());
			}
			// Different type - need to convert
			else {
				// clone() emits a GObject, so we need to use a dynamic_cast. It will emit
				// a null-pointer, if the conversion failed.
				GTemplateAdaptor<T>	*gtaptr = dynamic_cast<GTemplateAdaptor<T> *> ((*itFrom)->clone());
				if (gtaptr) {
					shared_ptr<GTemplateAdaptor<T> > p(gtaptr);
					itTo->swap(p);
				} else {
					gls << "In GMutable<T>::copyAdaptors(): Conversion error!"
						<< endl << logLevel(CRITICAL);

					exit(1);
				}
			}

		}

		if (from.size() == to.size()) return; // We're done. Likely the most frequent case.

		// As this code will only rarely be called, we store the sizes here and not above
		uint16_t fromSize = from.size(), toSize = to.size();

		// fromSize > toSize ? Great, we can just copy the rest of the adaptors over
		if (fromSize > toSize) {
			for (itFrom = from.begin() + toSize; itFrom != from.end(); ++itFrom) {
				// clone() emits a GObject, so we need to use a dynamic_cast. It will emit
				// a null-pointer, if the conversion failed.
				GTemplateAdaptor<T>
						*gtaptr =
								dynamic_cast<GTemplateAdaptor<T> *> ((*itFrom)->clone());
				if (gtaptr) {
					shared_ptr<GTemplateAdaptor<T> > p(gtaptr);
					to.push_back(p);
				} else {
					gls << "In GMutable<T>::copyAdaptors(): Conversion error!"
							<< endl << logLevel(CRITICAL);

					exit(1);
				}
			}

			return;
		}

		// toSize > fromSize ? We need to remove the surplus items. The
		// shared_ptr will take care of the item's deletion.
		if (toSize > fromSize) {
			to.resize(fromSize);
			return;
		}
	}

	/*******************************************************************************************/
	/**
	 * This vector contains the adaptors used for mutations of the values stored
	 * in derived classes.
	 */
	GTAvec adaptors_;
};

/****************************************************************************************************/

} /* namespace GenEvA */

/********************************************************************************************/

#if BOOST_VERSION <= 103500

/**
 * The macro BOOST_IS_ABSTRACT() of Boost's serialization framework
 * does not work for templatized classes. Hence we need to provide a suitable
 * replacement.
 */
namespace boost {
namespace serialization {
template<class T>
struct is_abstract<GenEvA::GMutable<T>> {
	typedef mpl::bool_<true> type;
	BOOST_STATIC_CONSTANT(bool, value = true);
};
}
}

#else

// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<class T>
		struct is_abstract<GenEvA::GMutable<T> > : boost::true_type {};
		template<class T>
		struct is_abstract< const GenEvA::GMutable<T> > : boost::true_type {};
	}}

#endif /* Serialization traits */

/********************************************************************************************/

#endif /*GMUTABLE_H_*/
