/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>

// Boost header files go here
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva header files go here
#include "common/GParserBuilder.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonHelperFunctionsT.hpp"

namespace Gem {
namespace Common {

/******************************************************************************/

const std::size_t GFACTTORYFIRSTID = std::size_t(1);
const std::size_t GFACTORYWRITEID = std::size_t(0);

/******************************************************************************/
/**
 * A factory class that returns objects of type prod_type . The class comprises a framework
 * for reading additional configuration options from a configuration file. The actual setup
 * work needs to be done in functions that are implemented in derived classes for each target
 * object individually, or in specializations of this class.
 */
template<typename prod_type>
class GFactoryT {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(m_configFile)
		 & BOOST_SERIALIZATION_NVP(m_id)
		 & BOOST_SERIALIZATION_NVP(m_initialized);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/

	 GFactoryT() = delete; ///< The default constructor. Intentionally undefined

	 /***************************************************************************/
	 /**
	  * The standard constructor
	  *
	  * @param configFile The name of a configuration file holding information about objects of type T
	  */
	 GFactoryT(const std::string &configFile)
		 : m_configFile(configFile)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GFactoryT(const GFactoryT<prod_type> &cp) = default;

	 /***************************************************************************/
	 /**
	  * The destructor.
	  */
	 virtual ~GFactoryT() = default;

	 /***************************************************************************/
	 /**
	  * Triggers the creation of objects of the desired type
	  *
	  * @return An individual of the desired type
	  */
	 std::shared_ptr <prod_type> operator()() {
		 return this->get();
	 }

	 /***************************************************************************/
	 /**
	  * Allows the creation of objects of the desired type.
	  */
	 virtual std::shared_ptr<prod_type> get() BASE {
		 // Make sure the initialization code has been executed.
		 // This function will do nothing when called more than once
		 this->globalInit();

		 // Create a parser builder object. It will be destroyed at
		 // the end of this function and thus cannot cause trouble
		 // due to registered call-backs and references
		 Gem::Common::GParserBuilder gpb;

		 // Add specific configuration options for the derived factory.
		 // These may correspond to local variables
		 this->describeLocalOptions_(gpb);

		 // Retrieve the actual object. It may, in the process of its
		 // creation, add further configuration options and call-backs to
		 // the parser
		 std::shared_ptr<prod_type> p = this->getObject_(gpb, m_id);

		 // Read the configuration parameters from file
		 if (not gpb.parseConfigFile(m_configFile)) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFactoryT<prod_type>::operator(): Error!" << std::endl
					 << "Could not parse configuration file " << m_configFile << std::endl
			 );
		 }

		 // Allow the factory to act on configuration options received
		 // in the parsing process.
		 this->postProcess_(p);

		 // Update the id
		 m_id++;

		 // Let the audience know
		 return p;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the name of the config file
	  *
	  * @return The name of the config-file
	  */
	 std::string getConfigFile() const {
		 return m_configFile;
	 }

	 /***************************************************************************/
	 /**
	  * Sets a new name for the configuration file. Will only have an effect for
	  * the next individual
	  */
	 void setConfigFile(std::string configFile) {
		 m_configFile = configFile;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves an object of the desired type and converts it to a target type,
	  * if possible.
	  */
	 template<typename tT>
	 // "tT" stands for "target type"
	 std::shared_ptr<tT> get_as() {
		 std::shared_ptr <prod_type> p = this->get();
		 if (p) {
			 return Gem::Common::convertSmartPointer<prod_type, tT>(p);
		 } else {
			 return std::shared_ptr<tT>(); // Just return an empty pointer
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Writes a configuration file to disk
	  *
	  * @param configFile The name of the configuration file to be written
	  * @param header A header to be prepended to the configuration file
	  */
	 void writeConfigFile(const std::string &header) {
		 // Make sure the initialization code has been executed.
		 // This function will do nothing when called more than once
		 this->globalInit();

		 // Create a parser builder object. It will be destroyed at
		 // the end of this function and thus cannot cause trouble
		 // due to registered call-backs and references
		 Gem::Common::GParserBuilder gpb;

		 // Add the user-defined configuration specifications, local to the factory
		 this->describeLocalOptions_(gpb);

		 // Retrieve an object (will be discarded at the end of this function)
		 // Here, further options may be added to the parser builder.
		 std::shared_ptr <prod_type> p = this->getObject_(gpb, GFACTORYWRITEID);

		 // Allow the factory to act on configuration options received
		 // in the parsing process.
		 this->postProcess_(p);

		 // Write out the configuration file, if options have been registered
		 if (gpb.numberOfFileOptions() > 0) {
			 gpb.writeConfigFile(m_configFile, header, true);
		 } else {
			 std::cout
				 << "Warning: An attempt was made to write out configuration file " << m_configFile << std::endl
				 << "even though no configuration options were registered. Doing nothing." << std::endl;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Loads the data of another GFactoryT<> object
	  */
	 virtual void load(std::shared_ptr<GFactoryT<prod_type>> cp) BASE {
		 m_configFile = cp->m_configFile;
		 m_id = cp->m_id;
		 m_initialized = cp->m_initialized;
	 }

	 /***************************************************************************/
	 /**
	  * Creates a deep clone of this object. This function is a trap. Factories
	  * wishing to use this functionality need to overload this function.
	  * Others don't have to due to this "pseudo-implementation".
	  */
	 virtual std::shared_ptr <GFactoryT<prod_type>> clone() const BASE {
		 throw gemfony_exception(
			 g_error_streamer(DO_LOG, time_and_place)
				 << "In GFactoryT<prod_type>::clone(): Error!" << std::endl
				 << "Function was called when it shouldn't be." << std::endl
				 << "This function is a trap." << std::endl
		 );

		 // Make the compiler happy
		 return std::shared_ptr<GFactoryT<prod_type>>();
	 }

protected:
	 /***************************************************************************/
	 /** @brief Performs necessary initialization work */
	 virtual void init_() BASE { /* nothing */ }

	 /** @brief Allows to describe local configuration options in derived classes */
	 virtual void describeLocalOptions_(Gem::Common::GParserBuilder &gpb) BASE { /* nothing */ };

	 /** @brief Creates individuals of the desired type */
	 virtual std::shared_ptr <prod_type> getObject_(Gem::Common::GParserBuilder &, const std::size_t &) BASE = 0;

	 /** @brief Allows to act on the configuration options received from the configuration file */
	 virtual void postProcess_(std::shared_ptr <prod_type> &) BASE = 0;

	 /***************************************************************************/
	 /**
	  * Retrieve the current value of the id_ variable
	  */
	 std::size_t getId() const {
		 return m_id;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Performs necessary global initialization work. This function is meant for
	  * initialization work performed just prior to the creation of the first
	  * item. It will do nothing when called more than once. All real work is done
	  * in the "init_()" function, which may be overloaded by the user.
	  */
	 void globalInit() {
		 if (not m_initialized) {
			 // Perform the user-defined initialization work
			 this->init_();
			 m_initialized = true;
		 }
	 }

	 /***************************************************************************/

	 std::string m_configFile; ///< The name of the configuration file
	 std::size_t m_id = GFACTTORYFIRSTID; ///< The id/number of the individual currently being created
	 bool m_initialized = false; ///< Indicates whether the initialization work has already been done
 };

 /******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */

namespace boost {
namespace serialization {
template<typename T>
struct is_abstract<Gem::Common::GFactoryT<T>> : public boost::true_type {
};
template<typename T>
struct is_abstract<const Gem::Common::GFactoryT<T>> : public boost::true_type {
};
}
}

/******************************************************************************/
