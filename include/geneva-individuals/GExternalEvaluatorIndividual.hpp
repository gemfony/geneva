/**
 * @file GExternalEvaluatorIndividual.hpp
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/exception/all.hpp>

#ifndef GEXTERNALEVALUATORINDIVIDUAL_HPP_
#define GEXTERNALEVALUATORINDIVIDUAL_HPP_

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GFactoryT.hpp"
#include "common/GParserBuilder.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GConstrainedDoubleCollection.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleBiGaussAdaptor.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GParameterSetMultiConstraint.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
// A number of default settings for the factory
const double GEEI_DEF_ADPROB = 1.0;
const double GEEI_DEF_ADAPTADPROB = 0.1;
const double GEEI_DEF_MINADPROB = 0.05;
const double GEEI_DEF_MAXADPROB = 1.;
const boost::uint32_t GEEI_DEF_ADAPTIONTHRESHOLD = 1;
const bool GEEI_DEF_USEBIGAUSSIAN = false;
const double GEEI_DEF_SIGMA1 = 0.025;
const double GEEI_DEF_SIGMASIGMA1 = 0.2;
const double GEEI_DEF_MINSIGMA1 = 0.001;
const double GEEI_DEF_MAXSIGMA1 = 1;
const double GEEI_DEF_SIGMA2 = 0.025;
const double GEEI_DEF_SIGMASIGMA2 = 0.2;
const double GEEI_DEF_MINSIGMA2 = 0.001;
const double GEEI_DEF_MAXSIGMA2 = 1;
const double GEEI_DEF_DELTA = 0.2;
const double GEEI_DEF_SIGMADELTA = 0.2;
const double GEEI_DEF_MINDELTA = 0.001;
const double GEEI_DEF_MAXDELTA = 1.;
const std::size_t GEEI_DEF_PARDIM = 2;
const double GEEI_DEF_MINVAR = -10.;
const double GEEI_DEF_MAXVAR = 10.;
const bool GEEI_DEF_USECONSTRAINEDDOUBLECOLLECTION = false;
const std::string GEEI_DEF_PROGNAME = "./evaluator/evaluator.py";
const std::string GEEI_DEF_CUSTOMOPTIONS = "empty";
const std::string GEEI_DEF_PARFILEBASENAME = "parameterFile";
const std::size_t GEEI_DEF_NRESULTS = 1;
const std::string GEEI_DEF_STARTMODE = "predef";
const std::string GEEI_DEF_DATATYPE = "setup_data";
const std::string GEEI_DEF_RUNID = "empty";
const bool GEEI_DEF_REMOVETEMPORARIES = "true";

/******************************************************************************/
// Forward declaration so we can expose the factory type to the public
// from within the GExternalEvaluatorIndividual
class GExternalEvaluatorIndividualFactory;

/******************************************************************************/
/**
 * This individual calls an external program to evaluate a given set of parameters.
 * Data exchange happens partially through the GNumericParameterT class. The
 * structure of the individual is determined from information given by the external
 * program. Currently double-, bool- and boost::int32_t values are supported.
 *
 * External programs should understand at least the following command line
 * arguments with obvious meanings
 *
 * --init
 * --setup --initValues=[min/max/random] --output="setupFile.xml"
 * --evaluate --input="paramsFile.xml"   --output="resultFile.xml"
 * --archive  --input="archiveFile.xml"
 * --finalize
 *
 * The xml parameter files are created using boost::property_tree and its write_xml
 * utility. Hence the external program needs to understand the XML format.
 */
class GExternalEvaluatorIndividual :public GParameterSet
{
	///////////////////////////////////////////////////////////////////////

   friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		& BOOST_SERIALIZATION_NVP(programName_)
		& BOOST_SERIALIZATION_NVP(customOptions_)
		& BOOST_SERIALIZATION_NVP(parameterFileBaseName_)
		& BOOST_SERIALIZATION_NVP(nResults_)
		& BOOST_SERIALIZATION_NVP(removeExecTemporaries_);
	}

	///////////////////////////////////////////////////////////////////////

 public:
	typedef GExternalEvaluatorIndividualFactory FACTORYTYPE;

   /** @brief The default constructor */
	G_API_INDIVIDUALS GExternalEvaluatorIndividual();
	/** @brief A standard copy constructor */
	G_API_INDIVIDUALS GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual&);
	/** @brief The standard destructor */
	virtual G_API_INDIVIDUALS ~GExternalEvaluatorIndividual();

	/** @brief A standard assignment operator */
	G_API_INDIVIDUALS const GExternalEvaluatorIndividual& operator=(const GExternalEvaluatorIndividual&);

	/** @brief Checks for equality with another GExternalEvaluatorIndividual object */
	G_API_INDIVIDUALS bool operator==(const GExternalEvaluatorIndividual&) const;
	/** @brief Checks for inequality with another GExternalEvaluatorIndividual object */
	G_API_INDIVIDUALS bool operator!=(const GExternalEvaluatorIndividual&) const;

   /** @brief Searches for compliance with expectations with respect to another object of the same type */
   virtual G_API_INDIVIDUALS void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
   ) const OVERRIDE;

   /** @brief Sets the name of the external evaluation program */
   G_API_INDIVIDUALS void setProgramName(const std::string&);
   /** @brief Retrieves the name of the external evaluation program */
   G_API_INDIVIDUALS std::string getProgramName() const;

   /** @brief Sets any custom options that need to be passed to the external evaluation program */
   G_API_INDIVIDUALS void setCustomOptions(const std::string&);
   /** @brief Retrieves any custom options that need to be passed to the external evaluation program */
   G_API_INDIVIDUALS std::string getCustomOptions() const;

	/** @brief Sets the base name of the data exchange file */
   G_API_INDIVIDUALS void setExchangeBaseName(const std::string&);
	/** @brief Retrieves the current value of the parameterFileBaseName_ variable */
   G_API_INDIVIDUALS std::string getExchangeBaseName() const;

	/** @brief Sets the number of results to be expected from the external evaluation program */
   G_API_INDIVIDUALS void setNExpectedResults(const std::size_t&);
	/** @brief Retrieves the number of results to be expected from the external evaluation program */
   G_API_INDIVIDUALS std::size_t getNExpectedResults() const;

	/** @brief Allows to set the data type of this individual */
   G_API_INDIVIDUALS void setDataType(std::string);
	/** @brief Allows to retrieve the data type of this individual */
   G_API_INDIVIDUALS std::string getDataType() const;

	/** @brief Allows to assign a run-id to this individual */
   G_API_INDIVIDUALS void setRunId(std::string);
	/** @brief Allows to retrieve the run-id assigned to this individual */
   G_API_INDIVIDUALS std::string getRunId() const;

	/** @brief Allows to specify whether temporary files should be removed */
   G_API_INDIVIDUALS void setRemoveExecTemporaries(bool);
	/** @brief Allows to check whether temporaries should be removed */
   G_API_INDIVIDUALS bool getRemoveExecTemporaries() const;

 protected:
	/** @brief Loads the data of another GExternalEvaluatorIndividual */
	virtual G_API_INDIVIDUALS void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual G_API_INDIVIDUALS GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here */
	virtual G_API_INDIVIDUALS double fitnessCalculation();

 private:
	/***************************************************************************/

	std::string programName_; ///< The name of the external program to be executed
	std::string customOptions_; ///< Any custom options that need to be provided to the external program
	std::string parameterFileBaseName_; ///< The base name to be assigned to the parameterFile
	std::size_t nResults_; ///< The number of results to be expected from the evaluation function
	std::string runID_; ///< Identifies this run with a unique id
	bool removeExecTemporaries_; ///< Indicates whether temporary files should be removed
 };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GExternalEvaluatorIndividual objects
 */
class GExternalEvaluatorIndividualFactory
   : public Gem::Common::GFactoryT<GParameterSet>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<class Archive>
   void serialize(Archive & ar, const unsigned int) {
      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Gem::Common::GFactoryT<GParameterSet>)
      & BOOST_SERIALIZATION_NVP(adProb_)
      & BOOST_SERIALIZATION_NVP(adaptAdProb_)
      & BOOST_SERIALIZATION_NVP(minAdProb_)
      & BOOST_SERIALIZATION_NVP(maxAdProb_)
      & BOOST_SERIALIZATION_NVP(adaptionThreshold_)
      & BOOST_SERIALIZATION_NVP(useBiGaussian_)
      & BOOST_SERIALIZATION_NVP(sigma1_)
      & BOOST_SERIALIZATION_NVP(sigmaSigma1_)
      & BOOST_SERIALIZATION_NVP(minSigma1_)
      & BOOST_SERIALIZATION_NVP(maxSigma1_)
      & BOOST_SERIALIZATION_NVP(sigma2_)
      & BOOST_SERIALIZATION_NVP(sigmaSigma2_)
      & BOOST_SERIALIZATION_NVP(minSigma2_)
      & BOOST_SERIALIZATION_NVP(maxSigma2_)
      & BOOST_SERIALIZATION_NVP(delta_)
      & BOOST_SERIALIZATION_NVP(sigmaDelta_)
      & BOOST_SERIALIZATION_NVP(minDelta_)
      & BOOST_SERIALIZATION_NVP(maxDelta_)
      & BOOST_SERIALIZATION_NVP(programName_)
      & BOOST_SERIALIZATION_NVP(customOptions_)
      & BOOST_SERIALIZATION_NVP(parameterFileBaseName_)
      & BOOST_SERIALIZATION_NVP(initValues_)
      & BOOST_SERIALIZATION_NVP(removeExecTemporaries_)
      & BOOST_SERIALIZATION_NVP(externalEvaluatorQueried_)
      & BOOST_SERIALIZATION_NVP(ptr_);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard constructor */
   G_API_INDIVIDUALS GExternalEvaluatorIndividualFactory(const std::string&);
   /** @brief The copy constructor */
   G_API_INDIVIDUALS GExternalEvaluatorIndividualFactory(const GExternalEvaluatorIndividualFactory&);
   /** @brief The destructor */
   virtual G_API_INDIVIDUALS ~GExternalEvaluatorIndividualFactory();

   /**************************************************************************/
   // Getters and setters

   /** @brief Allows to retrieve the adaptionThreshold_ variable */
   G_API_INDIVIDUALS boost::uint32_t getAdaptionThreshold() const;
   /** @brief Set the value of the adaptionThreshold_ variable */
   G_API_INDIVIDUALS void setAdaptionThreshold(boost::uint32_t adaptionThreshold);

   /** @brief Allows to retrieve the adProb_ variable */
   G_API_INDIVIDUALS double getAdProb() const;
   /** @brief Set the value of the adProb_ variable */
   G_API_INDIVIDUALS void setAdProb(double adProb);

   /** @brief Allows to retrieve the rate of evolutionary adaption of adProb_ */
   G_API_INDIVIDUALS double getAdaptAdProb() const;
   /** @brief Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature) */
   G_API_INDIVIDUALS void setAdaptAdProb(double adaptAdProb);

   /** @brief Allows to retrieve the allowed range for adProb_ variation */
   G_API_INDIVIDUALS boost::tuple<double,double> getAdProbRange() const;
   /** @brief Allows to set the allowed range for adaption probability variation */
   G_API_INDIVIDUALS void setAdProbRange(double minAdProb, double maxAdProb);

   /** @brief Allows to retrieve the useBiGaussian_ variable */
   G_API_INDIVIDUALS bool getUseBiGaussian() const;
   /** @brief Set the value of the useBiGaussian_ variable */
   G_API_INDIVIDUALS void setUseBiGaussian(bool useBiGaussian);

   /** @brief Allows to retrieve the delta_ variable */
   G_API_INDIVIDUALS double getDelta() const;
   /** @brief Set the value of the delta_ variable */
   G_API_INDIVIDUALS void setDelta(double delta);
   /** @brief Allows to retrieve the minDelta_ variable */
   G_API_INDIVIDUALS double getMinDelta() const;
   /** @brief Allows to retrieve the maxDelta_ variable */
   G_API_INDIVIDUALS double getMaxDelta() const;
   /** @brief Allows to retrieve the allowed value range of delta */
   G_API_INDIVIDUALS boost::tuple<double, double> getDeltaRange() const;
   /** @brief Allows to set the allowed value range of delta */
   G_API_INDIVIDUALS void setDeltaRange(boost::tuple<double, double>);

   /** @brief Allows to retrieve the minSigma1_ variable */
   G_API_INDIVIDUALS double getMinSigma1() const;
   /** @brief Allows to retrieve the maxSigma1_ variable */
   G_API_INDIVIDUALS double getMaxSigma1() const;
   /** @brief Allows to retrieve the allowed value range of sigma1_ */
   G_API_INDIVIDUALS boost::tuple<double, double> getSigma1Range() const;
   /** @brief Allows to set the allowed value range of sigma1_ */
   G_API_INDIVIDUALS void setSigma1Range(boost::tuple<double, double>);

   /** @brief Allows to retrieve the minSigma2_ variable */
   G_API_INDIVIDUALS double getMinSigma2() const;
   /** @brief Allows to retrieve the maxSigma2_ variable */
   G_API_INDIVIDUALS double getMaxSigma2() const;
   /** @brief Allows to retrieve the allowed value range of sigma2_ */
   G_API_INDIVIDUALS boost::tuple<double, double> getSigma2Range() const;
   /** @brief Allows to set the allowed value range of sigma2_ */
   G_API_INDIVIDUALS void setSigma2Range(boost::tuple<double, double>);

   /** @brief Allows to retrieve the sigma1_ variable */
   G_API_INDIVIDUALS double getSigma1() const;
   /** @brief Set the value of the sigma1_ variable */
   G_API_INDIVIDUALS void setSigma1(double sigma1);

   /** @brief Allows to retrieve the sigma2_ variable */
   G_API_INDIVIDUALS double getSigma2() const;
   /** @brief Set the value of the sigma2_ variable */
   G_API_INDIVIDUALS void setSigma2(double sigma2);

   /** @brief Allows to retrieve the sigmaDelta_ variable */
   G_API_INDIVIDUALS double getSigmaDelta() const;
   /** @brief Set the value of the sigmaDelta_ variable */
   G_API_INDIVIDUALS void setSigmaDelta(double sigmaDelta);

   /** @brief Allows to retrieve the sigmaSigma1_ variable */
   G_API_INDIVIDUALS double getSigmaSigma1() const;
   /** @brief Set the value of the sigmaSigma1_ variable */
   G_API_INDIVIDUALS void setSigmaSigma1(double sigmaSigma1);

   /** @brief Allows to retrieve the sigmaSigma2_ variable */
   G_API_INDIVIDUALS double getSigmaSigma2() const;
   /** @brief Set the value of the sigmaSigma2_ variable */
   G_API_INDIVIDUALS void setSigmaSigma2(double sigmaSigma2);

   /** @brief Allows to set the name and path of the external program */
   G_API_INDIVIDUALS void setProgramName(std::string);
   /** @brief Allows to retrieve the name of the external program */
   G_API_INDIVIDUALS std::string getProgramName() const;

   /** @brief Sets any custom options that need to be passed to the external evaluation program */
   G_API_INDIVIDUALS void setCustomOptions(const std::string);
   /** @brief Retrieves any custom options that need to be passed to the external evaluation program */
   G_API_INDIVIDUALS std::string getCustomOptions() const;

   /** @brief Allows to set the base name of the parameter file */
   G_API_INDIVIDUALS void setParameterFileBaseName(std::string);
   /** @brief Allows to retrieve the base name of the parameter file */
   G_API_INDIVIDUALS std::string getParameterFileBaseName() const;

   /** @brief Indicates the initialization mode */
   G_API_INDIVIDUALS void setInitValues(std::string);
   /** @brief Allows to retrieve the initialization mode */
   G_API_INDIVIDUALS std::string getInitValues() const;


   /** @brief Allows to specify whether temporary files should be removed */
   G_API_INDIVIDUALS void setRemoveExecTemporaries(bool);
   /** @brief Allows to check whether temporaries should be removed */
   G_API_INDIVIDUALS bool getRemoveExecTemporaries() const;

   // End of public getters and setters
   /**************************************************************************/

   /** @brief Submit work items to the external executable for archiving */
   G_API_INDIVIDUALS void archive(const std::vector<boost::shared_ptr<GExternalEvaluatorIndividual> >& arch) const;

   /** @brief Loads the data of another GFunctionIndividualFactory object */
   virtual G_API_INDIVIDUALS void load(boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> >);
   /** @brief Creates a deep clone of this object */
   virtual G_API_INDIVIDUALS boost::shared_ptr<Gem::Common::GFactoryT<GParameterSet> > clone() const;


protected:
   /** @brief Creates individuals of this type */
   virtual G_API_INDIVIDUALS boost::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
   /** @brief Allows to describe local configuration options in derived classes */
   virtual G_API_INDIVIDUALS void describeLocalOptions_(Gem::Common::GParserBuilder&);
   /** @brief Allows to act on the configuration options received from the configuration file */
   virtual G_API_INDIVIDUALS void postProcess_(boost::shared_ptr<GParameterSet>&);

private:
   /** @brief Sets up the boost property object holding information about the individual structure */
   void setUpPropertyTree();

   /** @brief Set the value of the minDelta_ variable */
   void setMinDelta(double minDelta);
   /** @brief Set the value of the maxDelta_ variable */
   void setMaxDelta(double maxDelta);

   /** @brief Set the value of the minSigma1_ variable */
   void setMinSigma1(double minSigma1);
   /** @brief Set the value of the maxSigma1_ variable */
   void setMaxSigma1(double maxSigma1);

   /** @brief Set the value of the minSigma2_ variable */
   void setMinSigma2(double minSigma2);
   /** @brief Set the value of the maxSigma2_ variable */
   void setMaxSigma2(double maxSigma2);

   /** @brief The default constructor; Only needed for (de-)serialization purposes, hence empty. */
   GExternalEvaluatorIndividualFactory();

   Gem::Common::GOneTimeRefParameterT<double> adProb_;
   Gem::Common::GOneTimeRefParameterT<double> adaptAdProb_;
   Gem::Common::GOneTimeRefParameterT<double> minAdProb_;
   Gem::Common::GOneTimeRefParameterT<double> maxAdProb_;
   Gem::Common::GOneTimeRefParameterT<boost::uint32_t> adaptionThreshold_;
   Gem::Common::GOneTimeRefParameterT<bool> useBiGaussian_;
   Gem::Common::GOneTimeRefParameterT<double> sigma1_;
   Gem::Common::GOneTimeRefParameterT<double> sigmaSigma1_;
   Gem::Common::GOneTimeRefParameterT<double> minSigma1_;
   Gem::Common::GOneTimeRefParameterT<double> maxSigma1_;
   Gem::Common::GOneTimeRefParameterT<double> sigma2_;
   Gem::Common::GOneTimeRefParameterT<double> sigmaSigma2_;
   Gem::Common::GOneTimeRefParameterT<double> minSigma2_;
   Gem::Common::GOneTimeRefParameterT<double> maxSigma2_;
   Gem::Common::GOneTimeRefParameterT<double> delta_;
   Gem::Common::GOneTimeRefParameterT<double> sigmaDelta_;
   Gem::Common::GOneTimeRefParameterT<double> minDelta_;
   Gem::Common::GOneTimeRefParameterT<double> maxDelta_;

   Gem::Common::GOneTimeRefParameterT<std::string> programName_;
   Gem::Common::GOneTimeRefParameterT<std::string> customOptions_;
   Gem::Common::GOneTimeRefParameterT<std::string> parameterFileBaseName_;
   Gem::Common::GOneTimeRefParameterT<std::string> initValues_;

   Gem::Common::GOneTimeRefParameterT<bool> removeExecTemporaries_;

   bool externalEvaluatorQueried_; ///< Specifies whether the external evaluator program has already been queried for setup information
   pt::ptree ptr_; ///< Holds setup information for individuals, as provided by the external evaluator program
};

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GExternalEvaluatorIndividual)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GExternalEvaluatorIndividualFactory)

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#endif /* GEXTERNALEVALUATORINDIVIDUAL_HPP_ */
