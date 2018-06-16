/**
 * @file GParameterSetFactory.hpp
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

// Standard heders go here

// Boosrt headers go here

// Geneva headers go here
#include "common/GFactoryT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GPostProcessorT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class facilitates handling of factories for GParameterSet-derivatives.
 * In particular it allows to register pre- and post-procesing objects
 */
class GParameterSetFactory
	: public Gem::Common::GFactoryT<GParameterSet>
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Gem::Common::GFactoryT<GParameterSet>)
		 & BOOST_SERIALIZATION_NVP(m_preProcessor)
		 & BOOST_SERIALIZATION_NVP(m_postProcessor);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The standard constructor
	  *
	  * @param configFile The name of a configuration file holding information about objects of type T
	  */
	 GParameterSetFactory(const std::string &configFile)
		 : Gem::Common::GFactoryT<GParameterSet>(configFile)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GParameterSetFactory(const GParameterSetFactory& cp)
		 : Gem::Common::GFactoryT<GParameterSet>(cp)
	 {
		 Gem::Common::copyCloneableSmartPointer(cp.m_postProcessor, m_postProcessor);
		 Gem::Common::copyCloneableSmartPointer(cp.m_postProcessor, m_postProcessor);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor.
	  */
	 virtual ~GParameterSetFactory()
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Registration of pre-processor function objects
	  */
	 void registerPreProcessor(std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> p) {
		 if(p) {
			 m_preProcessor = p;
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterSetFactory::registerPreProcessor(): Error!" << std::endl
					 << "Got empty pre-processor" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Registration of post-processor function objects
	  */
	 void registerPostProcessor(std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> p) {
		 if(p) {
			 m_postProcessor = p;
		 } else {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterSetFactory::registerPostProcessor(): Error!" << std::endl
					 << "Got empty post-processor" << std::endl
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Production of GParameterSet-derivatives
	  */
	 std::shared_ptr<GParameterSet> get() override {
		 std::shared_ptr<GParameterSet> p = GFactoryT<GParameterSet>::get();

		 if(m_preProcessor) {
			 p->registerPreProcessor(m_preProcessor->clone());
		 }

		 if(m_postProcessor) {
			 p->registerPostProcessor(m_postProcessor->clone());
		 }

		 return p;
	 }

protected:
	 /** @brief A pre-processor for GParameterSet-derivatives */
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> m_preProcessor;

	 /** @brief A post-processor for GParameterSet-derivatives */
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> m_postProcessor;

private:
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

