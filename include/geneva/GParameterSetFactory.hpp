/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

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
	  * @param configFile path object of a configuration file holding information about objects of type T
	  */
	 explicit GParameterSetFactory(boost::filesystem::path const &configFile)
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
	 // Defaulted and deleted functions
	 ~GParameterSetFactory() override = default;

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

protected:
	 /** @brief A pre-processor for GParameterSet-derivatives */
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> m_preProcessor;

	 /** @brief A post-processor for GParameterSet-derivatives */
	 std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> m_postProcessor;

	/***************************************************************************/
	/**
     * Production of GParameterSet-derivatives
     */
	std::shared_ptr<GParameterSet> get_() override {
		std::shared_ptr<GParameterSet> p = GFactoryT<GParameterSet>::get_();

		if(m_preProcessor) {
			p->registerPreProcessor(m_preProcessor->clone());
		}

		if(m_postProcessor) {
			p->registerPostProcessor(m_postProcessor->clone());
		}

		return p;
	}

private:
	 // Only needed for (de-)serialization purposes, hence private
	 GParameterSetFactory() = default;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

