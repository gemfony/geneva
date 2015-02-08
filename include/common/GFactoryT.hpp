/**
 * @file GFactoryT.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>

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

#ifndef GFACTORYT_HPP_
#define GFACTORYT_HPP_

// Geneva header files go here
#include <common/GParserBuilder.hpp>
#include <common/GExceptions.hpp>
#include <common/GHelperFunctionsT.hpp>

namespace Gem {
namespace Common {

/******************************************************************************/

const std::size_t GFACTTORYFIRSTID=std::size_t(1);
const std::size_t GFACTORYWRITEID =std::size_t(0);

/******************************************************************************/
/**
 * A factory class that returns objects of type prod_type . The class comprises a framework
 * for reading additional configuration options from a configuration file. The actual setup
 * work needs to be done in functions that are implemented in derived classes for each target
 * object individually, or in specializations of this class.
 */
template <typename prod_type>
class GFactoryT {
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int)  {
     using boost::serialization::make_nvp;

     ar
     & BOOST_SERIALIZATION_NVP(configFile_)
     & BOOST_SERIALIZATION_NVP(id_)
     & BOOST_SERIALIZATION_NVP(initialized_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
	/***************************************************************************/
	/**
	 * The standard constructor
	 *
	 * @param configFile The name of a configuration file holding information about objects of type T
	 */
	GFactoryT(const std::string& configFile)
		: configFile_(configFile)
		, id_(GFACTTORYFIRSTID)
		, initialized_(false)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 */
	GFactoryT(const GFactoryT<prod_type>& cp)
	   : configFile_(cp.configFile_)
	   , id_(cp.id_)
	   , initialized_(cp.initialized_)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor.
	 */
	virtual ~GFactoryT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Triggers the creation of objects of the desired type
	 *
	 * @return An individual of the desired type
	 */
	boost::shared_ptr<prod_type> operator()() {
	   return this->get();
	}

   /***************************************************************************/
	/**
	 * Allows the creation of objects of the desired type.
	 */
	virtual boost::shared_ptr<prod_type> get() {
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
      boost::shared_ptr<prod_type> p = this->getObject_(gpb, id_);

      // Read the configuration parameters from file
      if(!gpb.parseConfigFile(configFile_)) {
         glogger
         << "In GFactoryT<prod_type>::operator(): Error!" << std::endl
         << "Could not parse configuration file " << configFile_ << std::endl
         << GEXCEPTION;
      }

      // Allow the factory to act on configuration options received
      // in the parsing process.
      this->postProcess_(p);

      // Update the id
      id_++;

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
	   return configFile_;
	}

	/***************************************************************************/
	/**
	 * Sets a new name for the configuration file. Will only have an effect for
	 * the next individual
	 */
	void setConfigFile(std::string configFile) {
	   configFile_ = configFile;
	}

	/***************************************************************************/
	/**
	 * Retrieves an object of the desired type and converts it to a target type,
	 * if possible.
	 */
	template <typename tT> // "tT" stands for "target type"
	boost::shared_ptr<tT> get() {
	   boost::shared_ptr<prod_type> p = this->get();
	   if(p){
	      return Gem::Common::convertSmartPointer<prod_type, tT>(p);
	   } else {
	      return boost::shared_ptr<tT>(); // Just return an empty pointer
	   }
	}

	/***************************************************************************/
	/**
	 * Writes a configuration file to disk
	 *
	 * @param configFile The name of the configuration file to be written
	 * @param header A header to be prepended to the configuration file
	 */
	void writeConfigFile(const std::string& header) {
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
		boost::shared_ptr<prod_type> p = this->getObject_(gpb, GFACTORYWRITEID);

		// Allow the factory to act on configuration options received
		// in the parsing process.
		this->postProcess_(p);

		// Write out the configuration file, if options have been registered
		if(gpb.numberOfFileOptions() > 0) {
			gpb.writeConfigFile(configFile_, header, true);
		} else {
			std::cout
				<< "Warning: An attempt was made to write out configuration file " << configFile_ << std::endl
				<< "even though no configuration options were registered. Doing nothing." << std::endl;
		}
	}

	/***************************************************************************/
	/**
	 * Loads the data of another GFactoryT<> object
	 */
	virtual void load(boost::shared_ptr<GFactoryT<prod_type> > cp) {
	   configFile_ = cp->configFile_;
	   id_ = cp->id_;
	   initialized_ = cp->initialized_;
	}

	/***************************************************************************/
	/**
	 * Creates a deep clone of this object. This function is a trap. Factories
	 * wishing to use this functionality need to overload this function.
	 * Others don't have to due to this "pseudo-implementation".
	 */
	virtual boost::shared_ptr<GFactoryT<prod_type> > clone() const {
	   glogger
	   << "In GFactoryT<prod_type>::clone(): Error!" << std::endl
	   << "Function was called when it shouldn't be." << std::endl
	   << "This function is a trap." << std::endl
	   << GEXCEPTION;

	   // Make the compiler happy
	   return boost::shared_ptr<GFactoryT<prod_type> >();
	}

protected:
	/***************************************************************************/
	/** @brief Performs necessary initialization work */
	virtual void init_() {}
	/** @brief Allows to describe local configuration options in derived classes */
	virtual void describeLocalOptions_(Gem::Common::GParserBuilder& gpb) {};
	/** @brief Creates individuals of the desired type */
	virtual boost::shared_ptr<prod_type> getObject_(Gem::Common::GParserBuilder&, const std::size_t&) = 0;
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<prod_type>&) = 0;

   /***************************************************************************/
	/**
	 * Retrieve the current value of the id_ variable
	 */
	std::size_t getId() const {
	   return id_;
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
      if(!initialized_) {
         // Perform the user-defined initialization work
         this->init_();
         initialized_ = true;
      }
   }

	/***************************************************************************/
	GFactoryT(); ///< The default constructor. Intentionally private and undefined

	std::string configFile_; ///< The name of the configuration file
	std::size_t id_; ///< The id/number of the individual currently being created
	bool initialized_; ///< Indicates whether the initialization work has already been done
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
      struct is_abstract<Gem::Common::GFactoryT<T> > : public boost::true_type {};
      template<typename T>
      struct is_abstract< const Gem::Common::GFactoryT<T> > : public boost::true_type {};
   }
}

/******************************************************************************/

#endif /* GFACTORYT_HPP_ */
