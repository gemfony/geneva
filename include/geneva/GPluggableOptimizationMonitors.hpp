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
#include <string>
#include <fstream>
#include <type_traits>
#include <chrono>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

// Geneva headers go here
#include "courtier/GExecutorT.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GLogger.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "geneva/GParameterPropertyParser.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/G_OptimizationAlgorithm_Base.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements the standard output common to all optimization algorithms.
 * It will usually already be registered as a pluggable optimization monitor, when
 * you instantiate a new optimization algorithm.
 */
class GStandardMonitor
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar & make_nvp(
			 "GBasePluggableOM"
			 , boost::serialization::base_object<GBasePluggableOM>(*this)
		 );
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/

	 /** @brief The default constructor */
	 G_API_GENEVA GStandardMonitor() = default;
	 /** @brief The copy constructor */
	 G_API_GENEVA GStandardMonitor(const GStandardMonitor& cp) = default;
	 /** @brief The destructor */
	 virtual G_API_GENEVA  ~GStandardMonitor() = default;

	 /** @brief Aggregates the work of all registered pluggable monitors */
	 virtual G_API_GENEVA  void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA  void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another object */
	 G_API_GENEVA  void load_(const GObject* cp) override;

private:
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA  std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA  GObject* clone_() const override;

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object. */
	 G_API_GENEVA  bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed. */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. */
	 G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests() override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to output fitness information for a given optimization run.
 * It takes care of successive runs and marks them in the output. Information
 * will be output in the same histogram both for the best individual(s) found so far
 * and for the best individual(s) of each iteration.
 */
class GFitnessMonitor
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM", boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(m_xDim)
		 & BOOST_SERIALIZATION_NVP(m_yDim)
		 & BOOST_SERIALIZATION_NVP(m_nMonitorInds)
		 & BOOST_SERIALIZATION_NVP(m_resultFile)
		 & BOOST_SERIALIZATION_NVP(m_infoInitRun)
		 & BOOST_SERIALIZATION_NVP(m_globalFitnessGraphVec)
		 & BOOST_SERIALIZATION_NVP(m_iterationFitnessGraphVec);
	 }

	 ///////////////////////////////////////////////////////////////////////

public:
	 /************************************************************************/

	 /** @brief The default constructor */
	 G_API_GENEVA GFitnessMonitor() = default;
	 /** @brief The copy constructor */
	 G_API_GENEVA GFitnessMonitor(const GFitnessMonitor& cp);
	 /** @brief The destructor */
	 virtual G_API_GENEVA  ~GFitnessMonitor() = default;

	 /** @brief Allows to specify a different name for the result file */
	 G_API_GENEVA void setResultFileName(const std::string &resultFile);
	 /** @brief Allows to retrieve the current value of the result file name */
	 G_API_GENEVA std::string getResultFileName() const;

	 /** @brief Allows to set the dimensions of the canvas */
	 G_API_GENEVA void setDims(const std::uint32_t &xDim, const std::uint32_t &yDim);
	 /** @brief Retrieve the dimensions as a tuple */
	 G_API_GENEVA std::tuple<std::uint32_t, std::uint32_t> getDims() const;
	 /** @brief Retrieves the dimension of the canvas in x-direction */
	 G_API_GENEVA std::uint32_t getXDim() const;
	 /** @brief Retrieves the dimension of the canvas in y-direction */
	 G_API_GENEVA std::uint32_t getYDim() const;

	 /** @brief Sets the number of individuals in the population that should be monitored */
	 G_API_GENEVA void setNMonitorIndividuals(const std::size_t &nMonitorInds);
	 /** @brief Retrieves the number of individuals that are being monitored */
	 G_API_GENEVA std::size_t getNMonitorIndividuals() const;

	 /** @brief Aggregates the work of all registered pluggable monitors */
	 virtual G_API_GENEVA  void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA  void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

protected:
	 /************************************************************************/
	 /** @brief Loads the data of another object */
	 G_API_GENEVA  void load_(const GObject* cp) override;

private:
	 /************************************************************************/
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA  std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA  GObject* clone_() const override;

	 /************************************************************************/

	 std::uint32_t m_xDim = DEFAULTXDIMOM; ///< The dimension of the canvas in x-direction
	 std::uint32_t m_yDim = DEFAULTYDIMOM; ///< The dimension of the canvas in y-direction
	 std::size_t m_nMonitorInds = DEFNMONITORINDS; ///< The number of individuals that should be monitored
	 std::string m_resultFile = DEFAULTROOTRESULTFILEOM; ///< The name of the file to which data is emitted

	 bool m_infoInitRun = false; ///< Allows to check whether the INFOINIT section of informationFunction has already been passed at least once
	 std::vector<std::shared_ptr<Gem::Common::GGraph2D>> m_globalFitnessGraphVec; ///< Will hold progress information for the globally best individual
	 std::vector<std::shared_ptr<Gem::Common::GGraph2D>> m_iterationFitnessGraphVec; ///< Will hold progress information for an iteration best's individual

public:
	 /***************************************************************************/
	 /** @brief Applies modifications to this object. This is needed for testing purposes */
	 G_API_GENEVA  bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class accepts a number of other pluggable monitors and executes them
 * in sequence.
 */
class GCollectiveMonitor
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this));

		 // Some preparation needed if this is a load operation.
		 // This is needed to work around a problem in Boost 1.58
		 if (Archive::is_loading::value) {
			 m_pluggable_monitors.clear();
		 }

		 ar
		 & BOOST_SERIALIZATION_NVP(m_pluggable_monitors);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /** @brief The default constructor */
	 G_API_GENEVA GCollectiveMonitor() = default;
	 /** @brief The copy constructor */
	 G_API_GENEVA GCollectiveMonitor(const GCollectiveMonitor& cp);
	 /** @brief The destructor */
	 virtual G_API_GENEVA  ~GCollectiveMonitor() = default;

	 /** @brief Aggregates the work of all registered pluggable monitors */
	 virtual G_API_GENEVA  void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override;

	 /** @brief Allows to register a new pluggable monitor */
	 G_API_GENEVA void registerPluggableOM(std::shared_ptr<Gem::Geneva::GBasePluggableOM> om_ptr);
	 /** @brief Checks if adaptors have been registered in the collective monitor */
	 G_API_GENEVA bool hasOptimizationMonitors() const;
	 /** @brief Allows to clear all registered monitors */
	 G_API_GENEVA void resetPluggbleOM();

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA  void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

protected:
	 /***************************************************************************/
	 /** @brief Loads the data of another object */
	 G_API_GENEVA  void load_(const GObject* cp) override;

private:
	 /***************************************************************************/
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA  std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA  GObject* clone_() const override;

	 std::vector<std::shared_ptr<Gem::Geneva::GBasePluggableOM>> m_pluggable_monitors; ///< The collection of monitors

public:
	 /***************************************************************************/
	 /** @brief Applies modifications to this object */
	 G_API_GENEVA  bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail */
	 void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to monitor a given set of variables inside of all or of the
 * best individuals of a population, creating a graphical output using ROOT. It
 * supports floating point types only. double and float values may not be mixed.
 */
template <typename fp_type>
class GProgressPlotterT
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(m_fp_profVarVec)
		 & BOOST_SERIALIZATION_NVP(m_gpd)
		 & BOOST_SERIALIZATION_NVP(m_progressPlotter2D_oa)
		 & BOOST_SERIALIZATION_NVP(m_progressPlotter3D_oa)
		 & BOOST_SERIALIZATION_NVP(m_progressPlotter4D_oa)
		 & BOOST_SERIALIZATION_NVP(m_fileName)
		 & BOOST_SERIALIZATION_NVP(m_canvasDimensions)
		 & BOOST_SERIALIZATION_NVP(m_monitorBestOnly)
		 & BOOST_SERIALIZATION_NVP(m_monitorValidOnly)
		 & BOOST_SERIALIZATION_NVP(m_observeBoundaries)
		 & BOOST_SERIALIZATION_NVP(m_addPrintCommand);
	 }
	 ///////////////////////////////////////////////////////////////////////

	 // Make sure this class can only be instantiated if fp_type really is a floating point type
	 static_assert(
		 std::is_floating_point<fp_type>::value
		 , "fp_type should be a floating point type"
	 );

public:
	 /***************************************************************************/
	 /**
	  * The default constructor. Some member variables may be initialized
	  * in the class body.
	  */
	 GProgressPlotterT()
		 : m_gpd("Progress information", 1, 1)
			, m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1024,768))
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Construction with the information whether only the best individuals
	  * should be monitored and whether only valid items should be recorded.
	  * Some member variables may be initialized in the class body.
	  */
	 GProgressPlotterT(
		 bool monitorBestOnly
		 , bool monitorValidOnly
	 )
		 : m_gpd("Progress information", 1, 1)
			, m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1024,768))
			, m_monitorBestOnly(monitorBestOnly)
			, m_monitorValidOnly(monitorValidOnly)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GProgressPlotterT(const GProgressPlotterT<fp_type>& cp)
		 : GBasePluggableOM(cp)
			, m_gpd(cp.m_gpd)
			, m_fileName(cp.m_fileName)
			, m_canvasDimensions(cp.m_canvasDimensions)
			, m_monitorBestOnly(cp.m_monitorBestOnly)
			, m_monitorValidOnly(cp.m_monitorValidOnly)
			, m_observeBoundaries(cp.m_observeBoundaries)
			, m_addPrintCommand(cp.m_addPrintCommand)
	 {
		 Gem::Common::copyCloneableSmartPointer(cp.m_progressPlotter2D_oa, m_progressPlotter2D_oa);
		 Gem::Common::copyCloneableSmartPointer(cp.m_progressPlotter3D_oa, m_progressPlotter3D_oa);
		 Gem::Common::copyCloneableSmartPointer(cp.m_progressPlotter4D_oa, m_progressPlotter4D_oa);
		 Gem::Common::copyCloneableObjectsContainer(cp.m_fp_profVarVec, m_fp_profVarVec);
	 }

	 /***************************************************************************/
	 /**
	  * The destuctor
	  */
	 virtual ~GProgressPlotterT()
	 { /* nothing */ }

	 /**************************************************************************/
	 /**
	  * Sets the specifications of the variables to be profiled. Note that
	  * boolean and integer variables specified in the argument will simply
	  * be ignored.
	  */
	 void setProfileSpec(std::string parStr) {
		 // Check that the parameter string isn't empty
		 if(parStr.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GPluggableOptimizationMonitors<>::setProfileSpec(std::string): Error!" << std::endl
					 << "Parameter string " << parStr << " is empty" << std::endl
			 );
		 }

		 //---------------------------------------------------------------------------
		 // Clear the parameter vectors
		 m_fp_profVarVec.clear();

		 // Parse the parameter string
		 GParameterPropertyParser ppp(parStr);

		 //---------------------------------------------------------------------------
		 // Retrieve the parameters

		 std::tuple<
			 typename std::vector<parPropSpec<fp_type>>::const_iterator
			 , typename std::vector<parPropSpec<fp_type>>::const_iterator
		 > t_d = ppp.getIterators<fp_type>();

		 typename std::vector<parPropSpec<fp_type>>::const_iterator fp_cit = std::get<0>(t_d);
		 typename std::vector<parPropSpec<fp_type>>::const_iterator d_end  = std::get<1>(t_d);
		 for(; fp_cit!=d_end; ++fp_cit) { // Note: fp_cit is already set to the begin of the double parameter arrays
			 m_fp_profVarVec.push_back(*fp_cit);
		 }

		 //---------------------------------------------------------------------------
	 }

	 /***************************************************************************/
	 /**
	  * Allows to specify whether only the best individuals should be monitored.
	  */
	 void setMonitorBestOnly(bool monitorBestOnly = true) {
		 m_monitorBestOnly = monitorBestOnly;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether only the best individuals should be monitored.
	  */
	 bool getMonitorBestOnly() const {
		 return m_monitorBestOnly;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to specify whether only valid individuals should be monitored.
	  */
	 void setMonitorValidOnly(bool monitorValidOnly = true) {
		 m_monitorValidOnly = monitorValidOnly;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether only valid individuals should be monitored.
	  */
	 bool getMonitorValidOnly() const {
		 return m_monitorValidOnly;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to spefify whether scan boundaries should be observed
	  */
	 void setObserveBoundaries(bool observeBoundaries) {
		 m_observeBoundaries = observeBoundaries;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether boundaries should be observed
	  */
	 bool getObserveBoundaries() const {
		 return m_observeBoundaries;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether parameters should be profiled
	  */
	 bool parameterProfileCreationRequested() const {
		 return not m_fp_profVarVec.empty();
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the number of variables that will be profiled
	  */
	 std::size_t nProfileVars() const {
		 return m_fp_profVarVec.size();
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions
	  */
	 void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		 m_canvasDimensions = canvasDimensions;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions using separate x and y values
	  */
	 void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		 m_canvasDimensions = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	 }

	 /***************************************************************************/
	 /**
	  * Gives access to the canvas dimensions
	  */
	 std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		 return m_canvasDimensions;
	 }

	 /******************************************************************************/
	 /**
	  * Allows to add a "Print" command to the end of the script so that picture files are created
	  */
	 void setAddPrintCommand(bool addPrintCommand) {
		 m_addPrintCommand = addPrintCommand;
	 }

	 /******************************************************************************/
	 /**
	  * Allows to retrieve the current value of the m_addPrintCommand variable
	  */
	 bool getAddPrintCommand() const {
		 return m_addPrintCommand;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the filename
	  */
	 void setFileName(const std::string& fileName) {
		 m_fileName = fileName;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current filename to which information will be emitted
	  */
	 std::string getFileName() const {
		 return m_fileName;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas label
	  */
	 void setCanvasLabel(const std::string& canvasLabel) {
		 m_gpd.setCanvasLabel(canvasLabel);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve the canvas label
	  */
	 std::string getCanvasLabel() const {
		 return m_gpd.getCanvasLabel();
	 }

	 /***************************************************************************/
	 /**
	  * Determines a suitable label for a given parPropSpec value
	  */
	 std::string getLabel(const parPropSpec<fp_type>& s) const {
		 std::string result;

		 std::size_t var_mode = std::get<0>(s.var);
		 std::string var_name = std::get<1>(s.var);
		 std::size_t var_pos  = std::get<2>(s.var);

		 switch(var_mode) {
			 //--------------------------------------------------------------------
			 case 0: // parameters are identified by id
			 {
				 result = std::string("variable id ") + Gem::Common::to_string(var_pos);
			 }
				 break;

				 //--------------------------------------------------------------------
			 case 1:
			 {
				 result = var_name + "[" + Gem::Common::to_string(var_pos) + "]";
			 }
				 break;

				 //--------------------------------------------------------------------
			 case 2:
			 {
				 result = var_name;
			 }
				 break;

				 //--------------------------------------------------------------------
			 default:
			 {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GProgressPlotterT<fp_type>::getLabel(): Error" << std::endl
						 << "Invalid mode " << var_mode << " requested" << std::endl
				 );
			 }
				 break;

				 //--------------------------------------------------------------------
		 };

		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to emit information in different stages of the information cycle
	  * (initialization, during each cycle and during finalization)
	  */
	 virtual void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override {
		 switch(im) {
			 case Gem::Geneva::infoMode::INFOINIT:
			 {
				 switch(this->nProfileVars()) {
					 case 1:
					 {
						 m_progressPlotter2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());

						 m_progressPlotter2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);
						 m_progressPlotter2D_oa->setPlotLabel("Fitness as a function of a parameter value");
						 m_progressPlotter2D_oa->setXAxisLabel(this->getLabel(m_fp_profVarVec[0]));
						 m_progressPlotter2D_oa->setYAxisLabel("Fitness");

						 m_gpd.registerPlotter(m_progressPlotter2D_oa);
					 }
						 break;
					 case 2:
					 {
						 m_progressPlotter3D_oa = std::shared_ptr<Gem::Common::GGraph3D>(new Gem::Common::GGraph3D());

						 m_progressPlotter3D_oa->setPlotLabel("Fitness as a function of parameter values");
						 m_progressPlotter3D_oa->setXAxisLabel(this->getLabel(m_fp_profVarVec[0]));
						 m_progressPlotter3D_oa->setYAxisLabel(this->getLabel(m_fp_profVarVec[1]));
						 m_progressPlotter3D_oa->setZAxisLabel("Fitness");

						 m_gpd.registerPlotter(m_progressPlotter3D_oa);
					 }
						 break;

					 case 3:
					 {
						 m_progressPlotter4D_oa = std::shared_ptr<Gem::Common::GGraph4D>(new Gem::Common::GGraph4D());

						 m_progressPlotter4D_oa->setPlotLabel("Fitness (color-coded) as a function of parameter values");
						 m_progressPlotter4D_oa->setXAxisLabel(this->getLabel(m_fp_profVarVec[0]));
						 m_progressPlotter4D_oa->setYAxisLabel(this->getLabel(m_fp_profVarVec[1]));
						 m_progressPlotter4D_oa->setZAxisLabel(this->getLabel(m_fp_profVarVec[2]));

						 m_gpd.registerPlotter(m_progressPlotter4D_oa);
					 }
						 break;

					 default:
					 {
						 glogger
							 << "NOTE: In GProgressPlotterT<fp_type>::informationFunction(infoMode::INFOINIT):" << std::endl
							 << "Number of profiling dimensions " << this->nProfileVars() << " can not be displayed." << std::endl
							 << "No graphical output will be created." << std::endl
							 << GLOGGING;
					 }
						 break;
				 }

				 m_gpd.setCanvasDimensions(m_canvasDimensions);
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOPROCESSING:
			 {
				 bool isDirty = true;
				 double primaryFitness;

				 if(m_monitorBestOnly) { // Monitor the best individuals only
					 std::shared_ptr<GParameterSet> p = goa->G_Interface_OptimizerT::template getBestGlobalIndividual<GParameterSet>();
					 if(GBasePluggableOM::m_useRawEvaluation) {
						 primaryFitness = p->raw_fitness(0);
					 } else {
						 primaryFitness = p->transformed_fitness(0);
					 }

					 if(not m_monitorValidOnly || p->isValid()) {
						 switch(this->nProfileVars()) {
							 case 1:
							 {
								 fp_type val0 = p->GParameterSet::getVarVal<fp_type>(m_fp_profVarVec[0].var);

								 if(m_observeBoundaries) {
									 if(
										 val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
										 ) {
										 m_progressPlotter2D_oa->add(double(val0), primaryFitness);
									 }
								 } else {
									 m_progressPlotter2D_oa->add(double(val0), primaryFitness);
								 }
							 }
								 break;

							 case 2:
							 {
								 fp_type val0 = p->GParameterSet::getVarVal<fp_type>(m_fp_profVarVec[0].var);
								 fp_type val1 = p->GParameterSet::getVarVal<fp_type>(m_fp_profVarVec[1].var);

								 if(m_observeBoundaries) {
									 if(
										 val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
										 && val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
										 ) {
										 m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
									 }
								 } else {
									 m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
								 }
							 }
								 break;

							 case 3:
							 {
								 fp_type val0 = p->GParameterSet::getVarVal<fp_type>(m_fp_profVarVec[0].var);
								 fp_type val1 = p->GParameterSet::getVarVal<fp_type>(m_fp_profVarVec[1].var);
								 fp_type val2 = p->GParameterSet::getVarVal<fp_type>(m_fp_profVarVec[2].var);

								 if(m_observeBoundaries) {
									 if(
										 val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
										 && val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
										 && val2 >= m_fp_profVarVec[2].lowerBoundary && val2 <= m_fp_profVarVec[2].upperBoundary
										 ) {
										 m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
									 }
								 } else {
									 m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
								 }
							 }
								 break;

							 default: // Do nothing by default. The number of profiling dimensions is too large
								 break;
						 }
					 }
				 } else { // Monitor all individuals
					 for(const auto& ind_ptr: *goa) {
						 if(GBasePluggableOM::m_useRawEvaluation) {
							 primaryFitness = ind_ptr->raw_fitness(0);
						 } else {
							 primaryFitness = ind_ptr->transformed_fitness(0);
						 }

						 if(not m_monitorValidOnly || ind_ptr->isValid()) {
							 switch(this->nProfileVars()) {
								 case 1:
								 {
									 fp_type val0    = ind_ptr->GParameterSet::template getVarVal<fp_type>(m_fp_profVarVec[0].var);

									 if(m_observeBoundaries) {
										 if(
											 val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
											 ) {
											 m_progressPlotter2D_oa->add(double(val0), primaryFitness);
										 }
									 } else {
										 m_progressPlotter2D_oa->add(double(val0), primaryFitness);
									 }
								 }
									 break;

								 case 2:
								 {
									 fp_type val0 = ind_ptr->GParameterSet::template getVarVal<fp_type>(m_fp_profVarVec[0].var);
									 fp_type val1 = ind_ptr->GParameterSet::template getVarVal<fp_type>(m_fp_profVarVec[1].var);

									 if(m_observeBoundaries) {
										 if(
											 val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
											 && val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
											 ) {
											 m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
										 }
									 } else {
										 m_progressPlotter3D_oa->add(std::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
									 }
								 }
									 break;

								 case 3:
								 {
									 fp_type val0 = ind_ptr->GParameterSet::template getVarVal<fp_type>(m_fp_profVarVec[0].var);
									 fp_type val1 = ind_ptr->GParameterSet::template getVarVal<fp_type>(m_fp_profVarVec[1].var);
									 fp_type val2 = ind_ptr->GParameterSet::template getVarVal<fp_type>(m_fp_profVarVec[2].var);

									 if(m_observeBoundaries) {
										 if(
											 val0 >= m_fp_profVarVec[0].lowerBoundary && val0 <= m_fp_profVarVec[0].upperBoundary
											 && val1 >= m_fp_profVarVec[1].lowerBoundary && val1 <= m_fp_profVarVec[1].upperBoundary
											 && val2 >= m_fp_profVarVec[2].lowerBoundary && val2 <= m_fp_profVarVec[2].upperBoundary
											 ) {
											 m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
										 }
									 } else {
										 m_progressPlotter4D_oa->add(std::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
									 }
								 }
									 break;

								 default: // Do nothing by default. The number of profiling dimensions is too large
									 break;
							 }
						 }
					 }
				 }
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOEND:
			 {
				 // Make sure 1-D data is sorted
				 if(1 == this->nProfileVars()) {
					 m_progressPlotter2D_oa->sortX();
				 }

				 // Inform the plot designer whether it should print png files
				 m_gpd.setAddPrintCommand(m_addPrintCommand);

				 // Write out the result.
				 m_gpd.writeToFile(m_fileName);

				 // Remove all plotters
				 m_gpd.resetPlotters();
				 m_progressPlotter2D_oa.reset();
				 m_progressPlotter3D_oa.reset();
				 m_progressPlotter4D_oa.reset();
			 }
				 break;
		 };
	 }

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GObject object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GProgressPlotterT<fp_type reference independent of this object and convert the pointer
		 const GProgressPlotterT<fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 GToken token("GProgressPlotterT<fp_type>", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBasePluggableOM>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_fp_profVarVec, p_load->m_fp_profVarVec), token);
		 compare_t(IDENTITY(m_gpd, p_load->m_gpd), token);
		 compare_t(IDENTITY(m_progressPlotter2D_oa, p_load->m_progressPlotter2D_oa), token);
		 compare_t(IDENTITY(m_progressPlotter3D_oa, p_load->m_progressPlotter3D_oa), token);
		 compare_t(IDENTITY(m_progressPlotter4D_oa, p_load->m_progressPlotter4D_oa), token);
		 compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
		 compare_t(IDENTITY(m_canvasDimensions, p_load->m_canvasDimensions), token);
		 compare_t(IDENTITY(m_monitorBestOnly, p_load->m_monitorBestOnly), token);
		 compare_t(IDENTITY(m_monitorValidOnly, p_load->m_monitorValidOnly), token);
		 compare_t(IDENTITY(m_observeBoundaries, p_load->m_observeBoundaries), token);
		 compare_t(IDENTITY(m_addPrintCommand, p_load->m_addPrintCommand), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

protected:
	 /************************************************************************/
	 /**
	  * Loads the data of another object
	  *
	  * cp A pointer to another GProgressPlotterTT<fp_type> object, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Check that we are dealing with a GProgressPlotterT<fp_type> reference independent of this object and convert the pointer
		 const GProgressPlotterT<fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

		 // Load the parent classes' data ...
		 GBasePluggableOM::load_(cp);

		 // ... and then our local data
		 Gem::Common::copyCloneableObjectsContainer(p_load->m_fp_profVarVec, m_fp_profVarVec);
		 m_gpd.load(p_load->m_gpd);
		 copyCloneableSmartPointer(p_load->m_progressPlotter2D_oa, m_progressPlotter2D_oa);
		 copyCloneableSmartPointer(p_load->m_progressPlotter3D_oa, m_progressPlotter3D_oa);
		 copyCloneableSmartPointer(p_load->m_progressPlotter4D_oa, m_progressPlotter4D_oa);
		 m_fileName = p_load->m_fileName;
		 m_canvasDimensions = p_load->m_canvasDimensions;
		 m_monitorBestOnly = p_load->m_monitorBestOnly;
		 m_monitorValidOnly = p_load->m_monitorValidOnly;
		 m_observeBoundaries = p_load->m_observeBoundaries;
		 m_addPrintCommand = p_load->m_addPrintCommand;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name_() const override {
		 return std::string("GProgressPlotterT<fp_type>");
	 }

	 /************************************************************************/
	 /**
	  * Creates a deep clone of this object
	  */
	 GObject* clone_() const override {
		 return new GProgressPlotterT<fp_type>(*this);
	 }

	 /************************************************************************/

	 std::vector<parPropSpec<fp_type>> m_fp_profVarVec; ///< Holds information about variables to be profiled

	 Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots

	 // These are temporaries
	 std::shared_ptr<Gem::Common::GGraph2D> m_progressPlotter2D_oa;
	 std::shared_ptr<Gem::Common::GGraph3D> m_progressPlotter3D_oa;
	 std::shared_ptr<Gem::Common::GGraph4D> m_progressPlotter4D_oa;

	 std::string m_fileName = std::string("progressScan.C"); ///< The name of the file the output should be written to. Note that the class will add the name of the algorithm it acts on
	 std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions; ///< The dimensions of the canvas

	 bool m_monitorBestOnly = false;  ///< Indicates whether only the best individuals should be monitored
	 bool m_monitorValidOnly = false; ///< Indicates whether only valid individuals should be plotted
	 bool m_observeBoundaries = false; ///< When set to true, the plotter will ignore values outside of a scan boundary

	 bool m_addPrintCommand = false; ///< Asks the GPlotDesigner to add a print command to result files

public:
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  *
	  * @return A boolean which indicates whether modifications were made
	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 bool result = false;

		 // Call the parent classes' functions
		 if(GBasePluggableOM::modify_GUnitTests()) {
			 result = true;
		 }

		 // no local data -- nothing to change

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GProgressPlotterT<fp_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GProgressPlotterT<fp_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GBasePluggableOM::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GProgressPlotterT<fp_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }
	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log all candidate solutions found to a file, including the parameetr
 * values. NOTE that the file may become very large! Results are output in the following format:
 * param1 param2 ... param_m eval1 eval2 ... eval_n . By default, no commas and
 * explanations are printed. If withNameAndType is set to true, the values are
 * prepended by a line with variable names and types. If withCommas is set to true,
 * commas will be printed in-between values. It is possible to filter the results by
 * asking the class to only log solutions better than a given set of values. What
 * is considered better depends on whether evaluation criteria are maximized or minimized
 * and is determined from the individual. Note that this class can only be instantiated
 * if individual_type is either a derivative of GParamterSet or is an object of the
 * GParameterSet class itself.
 */
class GAllSolutionFileLogger
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(m_fileName)
		 & BOOST_SERIALIZATION_NVP(m_boundaries)
		 & BOOST_SERIALIZATION_NVP(m_boundariesActive)
		 & BOOST_SERIALIZATION_NVP(m_withNameAndType)
		 & BOOST_SERIALIZATION_NVP(m_withCommas)
		 & BOOST_SERIALIZATION_NVP(m_useRawFitness)
		 & BOOST_SERIALIZATION_NVP(m_showValidity)
		 & BOOST_SERIALIZATION_NVP(m_printInitial)
		 & BOOST_SERIALIZATION_NVP(m_showIterationBoundaries);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/

	 /** @brief The default constructor */
	 G_API_GENEVA GAllSolutionFileLogger() = default;
	 /** @brief Initialization with a file name */
	 G_API_GENEVA GAllSolutionFileLogger(const std::string& fileName);
	 /** @brief Initialization with a file name and boundaries */
	 G_API_GENEVA GAllSolutionFileLogger(
		 const std::string& fileName
		 , const std::vector<double>& boundaries
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA GAllSolutionFileLogger(const GAllSolutionFileLogger& cp);
	 /** @brief The destructor */
	 virtual G_API_GENEVA  ~GAllSolutionFileLogger() = default;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA  void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

	 /** @brief Sets the file name */
	 G_API_GENEVA void setFileName(const std::string& fileName);
	 /** @brief Retrieves the current file name */
	 G_API_GENEVA std::string getFileName() const;

	 /** @brief Sets the boundaries */
	 G_API_GENEVA void setBoundaries(const std::vector<double>& boundaries);
	 /** @brief Allows to retrieve the boundaries */
	 G_API_GENEVA std::vector<double> getBoundaries() const;
	 /** @brief Allows to check whether boundaries are active */
	 G_API_GENEVA bool boundariesActive() const;
	 /** @brief Allows to inactivate boundaries */
	 G_API_GENEVA void setBoundariesInactive();

	 /** @brief  Allows to specify whether explanations should be printed for parameter- and fitness values. */
	 G_API_GENEVA void setPrintWithNameAndType(bool withNameAndType = true);
	 /** @brief Allows to check whether explanations should be printed for parameter-and fitness values */
	 G_API_GENEVA bool getPrintWithNameAndType() const;

	 /** @brief Allows to specify whether commas should be printed in-between values */
	 G_API_GENEVA void setPrintWithCommas(bool withCommas = true);
	 /** @brief Allows to check whether commas should be printed in-between values */
	 G_API_GENEVA bool getPrintWithCommas() const;

	 /** @brief Allows to specify whether the true (instead of the transformed) fitness should be shown */
	 G_API_GENEVA void setUseTrueFitness(bool useRawFitness = true);
	 /** @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown */
	 G_API_GENEVA bool getUseTrueFitness() const;

	 /** @brief Allows to specify whether the validity of a solution should be shown */
	 G_API_GENEVA void setShowValidity(bool showValidity = true);
	 /** @brief Allows to check whether the validity of a solution will be shown */
	 G_API_GENEVA bool getShowValidity() const;

	 /** @brief Allows to specifiy whether the initial population should be printed. */
	 G_API_GENEVA void setPrintInitial(bool printInitial = true);
	 /** @brief Allows to check whether the initial population should be printed. */
	 G_API_GENEVA bool getPrintInitial() const;

	 /** @brief Allows to specifiy whether a comment line should be inserted between iterations */
	 G_API_GENEVA void setShowIterationBoundaries(bool showIterationBoundaries = true);
	 /** @brief Allows to check whether a comment line should be inserted between iterations */
	 G_API_GENEVA bool getShowIterationBoundaries() const;

	 /** @brief Allows to emit information in different stages of the information cycle */
	 virtual G_API_GENEVA  void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override;

protected:
	 /************************************************************************/

	 /** @brief Loads the data of another object */
	 G_API_GENEVA  void load_(const GObject* cp) override;


private:
	 /***************************************************************************/
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA  std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA  GObject* clone_() const override;

	 /** @brief Does the actual printing */
	 G_API_GENEVA void printPopulation(
		 const std::string& iterationDescription
		 , G_OptimizationAlgorithm_Base *const goa
	 );

	 /***************************************************************************/
	 // Data

	 std::string m_fileName = "CompleteSolutionLog.txt"; ///< The name of the file to which solutions should be stored
	 std::vector<double> m_boundaries; ///< Value boundaries used to filter logged solutions
	 bool m_boundariesActive = false; ///< Set to true if boundaries have been set
	 bool m_withNameAndType = false; ///< When set to true, explanations for values are printed
	 bool m_withCommas = false; ///< When set to true, commas will be printed in-between values
	 bool m_useRawFitness = true; ///< Indicates whether true- or transformed fitness should be output
	 bool m_showValidity = true; ///< Indicates whether the validity of a solution should be shown
	 bool m_printInitial = false; ///< Indicates whether the initial population should be printed
	 bool m_showIterationBoundaries = false; ///< Indicates whether a comment indicating the end of an iteration should be printed

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object */
	 G_API_GENEVA  bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail */
	 G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class prints out all evaluations of each iteration. The format is
 * eval0_0, eval0_1, ... ,eval0_n, ..., evalm_0, evalm_1, ... ,evalm_n
 */
class GIterationResultsFileLogger
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(m_fileName)
		 & BOOST_SERIALIZATION_NVP(m_withCommas)
		 & BOOST_SERIALIZATION_NVP(m_useRawFitness);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/

	 /** @brief The default constructor */
	 G_API_GENEVA GIterationResultsFileLogger() = default;
	 /** @brief Initialization with a file name */
	 G_API_GENEVA GIterationResultsFileLogger(const std::string& fileName);
	 /** @brief The copy constructor */
	 G_API_GENEVA GIterationResultsFileLogger(const GIterationResultsFileLogger& cp);
	 /** @brief The destructor */
	 virtual G_API_GENEVA  ~GIterationResultsFileLogger() = default;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA  void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

	 /** @brief Sets the file name */
	 G_API_GENEVA void setFileName(const std::string& fileName);
	 /** @brief Retrieves the current file name */
	 G_API_GENEVA std::string getFileName() const;

	 /** @brief Allows to specify whether commas should be printed in-between values */
	 G_API_GENEVA void setPrintWithCommas(bool withCommas);
	 /** @brief Allows to check whether commas should be printed in-between values */
	 G_API_GENEVA bool getPrintWithCommas() const;

	 /** @brief Allows to specify whether the true (instead of the transformed) fitness should be shown */
	 G_API_GENEVA void setUseTrueFitness(bool useRawFitness);
	 /** @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown */
	 G_API_GENEVA bool getUseTrueFitness() const;

	 /** @brief Allows to emit information in different stages of the information cycle */
	 virtual G_API_GENEVA  void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override;

protected:
	 /************************************************************************/

	 /** @brief Loads the data of another object */
	 G_API_GENEVA  void load_(const GObject* cp) override;

private:
	 /***************************************************************************/
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA  std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA  GObject* clone_() const override;

	 std::string m_fileName = "IterationResultsLog.txt"; ///< The name of the file to which solutions should be stored
	 bool m_withCommas = true; ///< When set to true, commas will be printed in-between values
	 bool m_useRawFitness = false; ///< Indicates whether true- or transformed fitness should be output

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object */
	 G_API_GENEVA  bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log the number of adaptions made inside of adaptors
 * to a file. This is mostly needed for debugging and profiling purposes. The
 * number of adaptions made is a good measure for the adaption probability.
 */
class GNAdpationsLogger
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(m_fileName)
		 & BOOST_SERIALIZATION_NVP(m_canvasDimensions)
		 & BOOST_SERIALIZATION_NVP(m_gpd)
		 & BOOST_SERIALIZATION_NVP(m_nAdaptionsHist2D_oa)
		 & BOOST_SERIALIZATION_NVP(m_nAdaptionsGraph2D_oa)
		 & BOOST_SERIALIZATION_NVP(m_fitnessGraph2D_oa)
		 & BOOST_SERIALIZATION_NVP(m_monitorBestOnly)
		 & BOOST_SERIALIZATION_NVP(m_addPrintCommand)
		 & BOOST_SERIALIZATION_NVP(m_maxIteration)
		 & BOOST_SERIALIZATION_NVP(m_nIterationsRecorded)
		 & BOOST_SERIALIZATION_NVP(m_nAdaptionsStore);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/

	 /** @brief The default constructor */
	 G_API_GENEVA GNAdpationsLogger();
	 /** @brief Initialization with a file name */
	 explicit G_API_GENEVA GNAdpationsLogger(const std::string& fileName);
	 /** @brief The copy constructor */
	 G_API_GENEVA GNAdpationsLogger(const GNAdpationsLogger& cp);
	 /** @brief The destructor */
	 virtual G_API_GENEVA  ~GNAdpationsLogger() = default;

	 /** @brief Searches for compliance with expectations with respect to another object */
	 virtual G_API_GENEVA  void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

	 /** @brief Sets the file name */
	 G_API_GENEVA void setFileName(const std::string& fileName);
	 /** @brief Retrieves the current file name */
	 G_API_GENEVA std::string getFileName() const;

	 /** @brief Allows to specify whether only the best individuals should be monitored */
	 G_API_GENEVA void setMonitorBestOnly(bool monitorBestOnly = true);
	 /** @brief Allows to check whether only the best individuals should be monitored */
	 G_API_GENEVA bool getMonitorBestOnly() const;

	 /** @brief Allows to set the canvas dimensions */
	 G_API_GENEVA void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions);
	 /** @brief Allows to set the canvas dimensions using separate x and y values */
	 G_API_GENEVA void setCanvasDimensions(std::uint32_t x, std::uint32_t y);
	 /** @brief Gives access to the canvas dimensions */
	 G_API_GENEVA std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const;

	 /** @brief Allows to add a "Print" command to the end of the script so that picture files are created */
	 G_API_GENEVA void setAddPrintCommand(bool addPrintCommand);
	 /** @brief Allows to retrieve the current value of the m_addPrintCommand variable */
	 G_API_GENEVA bool getAddPrintCommand() const;

	 /** @brief Allows to emit information in different stages of the information cycle */
	 virtual G_API_GENEVA  void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override;

protected:
	 /************************************************************************/

	 /** @brief Loads the data of another object */
	 G_API_GENEVA  void load_(const GObject* cp) override;

private:
	 /***************************************************************************/
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA  GObject* clone_() const override;

	 std::string m_fileName = "NAdaptions.C"; ///< The name of the file to which solutions should be stored

	 std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions; ///< The dimensions of the canvas

	 Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots

	 std::shared_ptr<Gem::Common::GHistogram2D> m_nAdaptionsHist2D_oa;  ///< Holds the actual histogram
	 std::shared_ptr<Gem::Common::GGraph2D>     m_nAdaptionsGraph2D_oa; ///< Used if we only monitor the best solution in each iteration
	 std::shared_ptr<Gem::Common::GGraph2D>     m_fitnessGraph2D_oa;    ///< Lets us monitor the current fitness of the population

	 bool m_monitorBestOnly = false; ///< Indicates whether only the best individuals should be monitored
	 bool m_addPrintCommand = false; ///< Asks the GPlotDesigner to add a print command to result files

	 std::size_t m_maxIteration = 0; ///< Holds the largest iteration recorded for the algorithm
	 std::size_t m_nIterationsRecorded = 0; ///< Holds the number of iterations that were recorded (not necessarily == m_maxIteration

	 std::vector<std::tuple<double, double>> m_nAdaptionsStore; ///< Holds all information about the number of adaptions

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object */
	 G_API_GENEVA  bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log chosen properties of adaptors. Such properties
 * are limited to numeric entities, that may be converted to double
 */
template <typename num_type>
class GAdaptorPropertyLoggerT
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(m_fileName)
		 & BOOST_SERIALIZATION_NVP(m_adaptorName)
		 & BOOST_SERIALIZATION_NVP(m_property)
		 & BOOST_SERIALIZATION_NVP(m_canvasDimensions)
		 & BOOST_SERIALIZATION_NVP(m_gpd)
		 & BOOST_SERIALIZATION_NVP(m_adaptorPropertyHist2D_oa)
		 & BOOST_SERIALIZATION_NVP(m_fitnessGraph2D_oa)
		 & BOOST_SERIALIZATION_NVP(m_monitorBestOnly)
		 & BOOST_SERIALIZATION_NVP(m_addPrintCommand)
		 & BOOST_SERIALIZATION_NVP(m_maxIteration)
		 & BOOST_SERIALIZATION_NVP(m_nIterationsRecorded)
		 & BOOST_SERIALIZATION_NVP(m_adaptorPropertyStore);
	 }
	 ///////////////////////////////////////////////////////////////////////

	 static_assert(
		 std::is_arithmetic<num_type>::value
		 , "num_type should be an arithmetic type"
	 );

public:
	 /***************************************************************************/
	 /**
	  * The default constructor. Note that some parmeters may be initialized in
	  * the class body.
	  */
	 GAdaptorPropertyLoggerT()
		 : m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
			, m_gpd("Adaptor properties", 1, 2)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initialization with a file name
	  */
	 GAdaptorPropertyLoggerT(
		 const std::string& fileName
		 , const std::string& adaptorName
		 , const std::string& property
	 )
		 : m_fileName(fileName)
			, m_adaptorName(adaptorName)
			, m_property(property)
			, m_canvasDimensions(std::tuple<std::uint32_t,std::uint32_t>(1200,1600))
			, m_gpd("Adaptor properties", 1, 2)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GAdaptorPropertyLoggerT(const GAdaptorPropertyLoggerT<num_type>& cp)
		 : m_fileName(cp.m_fileName)
			, m_adaptorName(cp.m_adaptorName)
			, m_property(cp.m_property)
			, m_canvasDimensions(cp.m_canvasDimensions)
			, m_gpd(cp.m_gpd)
			, m_monitorBestOnly(cp.m_monitorBestOnly)
			, m_addPrintCommand(cp.m_addPrintCommand)
			, m_maxIteration(cp.m_maxIteration)
			, m_nIterationsRecorded(cp.m_nIterationsRecorded)
			, m_adaptorPropertyStore(cp.m_adaptorPropertyStore)
	 {
		 // Copy the smart pointers over
		 Gem::Common::copyCloneableSmartPointer(cp.m_adaptorPropertyHist2D_oa, m_adaptorPropertyHist2D_oa);
		 Gem::Common::copyCloneableSmartPointer(cp.m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
	 }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GAdaptorPropertyLoggerT() = default;

	 /***************************************************************************/
	 /**
	  * Searches for compliance with expectations with respect to another object
	  * of the same type
	  *
	  * @param cp A constant reference to another GObject object
	  * @param e The expected outcome of the comparison
	  * @param limit The maximum deviation for floating point values (important for similarity checks)
	  */
	 virtual void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override {
		 using namespace Gem::Common;

		 // Check that we are dealing with a GAdaptorPropertyLoggerT<num_type> reference independent of this object and convert the pointer
		 const GAdaptorPropertyLoggerT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerT<num_type>>(cp, this);

		 GToken token("GAdaptorPropertyLoggerT", e);

		 // Compare our parent data ...
		 Gem::Common::compare_base<GBasePluggableOM>(IDENTITY(*this, *p_load), token);

		 // ... and then our local data
		 compare_t(IDENTITY(m_fileName, p_load->m_fileName), token);
		 compare_t(IDENTITY(m_adaptorName, p_load->m_adaptorName), token);
		 compare_t(IDENTITY(m_property, p_load->m_property), token);
		 compare_t(IDENTITY(m_canvasDimensions, p_load->m_canvasDimensions), token);
		 compare_t(IDENTITY(m_gpd, p_load->m_gpd), token);
		 compare_t(IDENTITY(m_adaptorPropertyHist2D_oa, p_load->m_adaptorPropertyHist2D_oa), token);
		 compare_t(IDENTITY(m_fitnessGraph2D_oa, p_load->m_fitnessGraph2D_oa), token);
		 compare_t(IDENTITY(m_monitorBestOnly, p_load->m_monitorBestOnly), token);
		 compare_t(IDENTITY(m_addPrintCommand, p_load->m_addPrintCommand), token);
		 compare_t(IDENTITY(m_maxIteration, p_load->m_maxIteration), token);
		 compare_t(IDENTITY(m_nIterationsRecorded, p_load->m_nIterationsRecorded), token);
		 compare_t(IDENTITY(m_adaptorPropertyStore, p_load->m_adaptorPropertyStore), token);

		 // React on deviations from the expectation
		 token.evaluate();
	 }

	 /***************************************************************************/
	 /**
	  * Sets the file name
	  */
	 void setFileName(const std::string& fileName) {
		 m_fileName = fileName;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the current file name
	  */
	 std::string getFileName() const {
		 return m_fileName;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the name of the adaptor
	  */
	 void setAdaptorName(std::string adaptorName) {
		 m_adaptorName = adaptorName;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the name of the adaptor
	  */
	 std::string getAdaptorName() const {
		 return m_adaptorName;
	 }

	 /***************************************************************************/
	 /**
	  * Sets the name of the property
	  */
	 void setPropertyName(std::string property) {
		 m_property = property;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the name of the property
	  */
	 std::string getPropertyName() const {
		 return m_property;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to specify whether only the best individuals should be monitored.
	  */
	 void setMonitorBestOnly(bool monitorBestOnly = true) {
		 m_monitorBestOnly = monitorBestOnly;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether only the best individuals should be monitored.
	  */
	 bool getMonitorBestOnly() const {
		 return m_monitorBestOnly;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions
	  */
	 void setCanvasDimensions(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions) {
		 m_canvasDimensions = canvasDimensions;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to set the canvas dimensions using separate x and y values
	  */
	 void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
		 m_canvasDimensions = std::tuple<std::uint32_t,std::uint32_t>(x,y);
	 }

	 /***************************************************************************/
	 /**
	  * Gives access to the canvas dimensions
	  */
	 std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions() const {
		 return m_canvasDimensions;
	 }

	 /******************************************************************************/
	 /**
	  * Allows to add a "Print" command to the end of the script so that picture files are created
	  */
	 void setAddPrintCommand(bool addPrintCommand) {
		 m_addPrintCommand = addPrintCommand;
	 }

	 /******************************************************************************/
	 /**
	  * Allows to retrieve the current value of the m_addPrintCommand variable
	  */
	 bool getAddPrintCommand() const {
		 return m_addPrintCommand;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to emit information in different stages of the information cycle
	  * (initialization, during each cycle and during finalization)
	  */
	 virtual void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override {
		 using namespace Gem::Common;

		 switch(im) {
			 case Gem::Geneva::infoMode::INFOINIT:
			 {
				 // If the file pointed to by m_fileName already exists, make a back-up
				 if(bf::exists(m_fileName)) {
					 std::string newFileName = m_fileName + ".bak_" + Gem::Common::getMSSince1970();

					 glogger
						 << "In GAdaptorPropertyLoggerT::informationFunction(): Error!" << std::endl
						 << "Attempt to output information to file " << m_fileName << std::endl
						 << "which already exists. We will rename the old file to" << std::endl
						 << newFileName << std::endl
						 << GWARNING;

					 bf::rename(m_fileName, newFileName);
				 }

				 // Make sure the progress plotter has the desired size
				 m_gpd.setCanvasDimensions(m_canvasDimensions);

				 // Set up a graph to monitor the best fitness found
				 m_fitnessGraph2D_oa = std::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
				 m_fitnessGraph2D_oa->setXAxisLabel("Iteration");
				 m_fitnessGraph2D_oa->setYAxisLabel("Fitness");
				 m_fitnessGraph2D_oa->setPlotMode(Gem::Common::graphPlotMode::CURVE);
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOPROCESSING:
			 {
				 std::uint32_t iteration = goa->getIteration();

				 // Record the current fitness
				 std::shared_ptr<GParameterSet> p = goa->G_Interface_OptimizerT::template getBestGlobalIndividual<GParameterSet>();
				 (*m_fitnessGraph2D_oa) & std::tuple<double,double>(double(iteration), p->raw_fitness(0));

				 // Update the largest known iteration and the number of recorded iterations
				 m_maxIteration = iteration;
				 m_nIterationsRecorded++;

				 // Will hold the adaptor properties
				 std::vector<boost::any> data;

				 // Do the actual logging
				 if(m_monitorBestOnly) {
					 std::shared_ptr<GParameterSet> best = goa->G_Interface_OptimizerT::template getBestGlobalIndividual<GParameterSet>();

					 // Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
					 best->queryAdaptor(m_adaptorName, m_property, data);

					 // Attach the data to m_adaptorPropertyStore
					 std::vector<boost::any>::iterator prop_it;
					 for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
						 m_adaptorPropertyStore.push_back(std::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
					 }
				 } else { // Monitor all individuals
					 // Loop over all individuals of the algorithm.
					 for(std::size_t pos=0; pos<goa->size(); pos++) {
						 std::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

						 // Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
						 ind->queryAdaptor(m_adaptorName, m_property, data);

						 // Attach the data to m_adaptorPropertyStore
						 std::vector<boost::any>::iterator prop_it;
						 for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
							 m_adaptorPropertyStore.push_back(std::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
						 }
					 }
				 }
			 }
				 break;

			 case Gem::Geneva::infoMode::INFOEND:
			 {
				 std::vector<std::tuple<double, double>>::iterator it;

				 // Within m_adaptorPropertyStore, find the largest number of adaptions performed
				 double maxProperty = 0.;
				 for(it=m_adaptorPropertyStore.begin(); it!=m_adaptorPropertyStore.end(); ++it) {
					 if(std::get<1>(*it) > maxProperty) {
						 maxProperty = std::get<1>(*it);
					 }
				 }

				 // Create the histogram object
				 m_adaptorPropertyHist2D_oa = std::shared_ptr<GHistogram2D>(
					 new GHistogram2D(
						 m_nIterationsRecorded
						 , 100
						 , 0., double(m_maxIteration)
						 , 0., maxProperty
					 )
				 );

				 m_adaptorPropertyHist2D_oa->setXAxisLabel("Iteration");
				 m_adaptorPropertyHist2D_oa->setYAxisLabel(std::string("Adaptor-Name: ") + m_adaptorName + std::string(", Property: ") + m_property);
				 m_adaptorPropertyHist2D_oa->setDrawingArguments("BOX");

				 // Fill the object with data
				 for(it=m_adaptorPropertyStore.begin(); it!=m_adaptorPropertyStore.end(); ++it) {
					 (*m_adaptorPropertyHist2D_oa) & *it;
				 }

				 // Add the histogram to the plot designer
				 m_gpd.registerPlotter(m_adaptorPropertyHist2D_oa);

				 // Add the fitness monitor
				 m_gpd.registerPlotter(m_fitnessGraph2D_oa);

				 // Inform the plot designer whether it should print png files
				 m_gpd.setAddPrintCommand(m_addPrintCommand);

				 // Write out the result. Note that we add
				 m_gpd.writeToFile(m_fileName);

				 // Remove all plotters (they will survive inside of gpd)
				 m_gpd.resetPlotters();
				 m_adaptorPropertyHist2D_oa.reset();
			 }
				 break;

			 default:
			 {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GAdaptorPropertyLoggerT: Received invalid infoMode " << im << std::endl
				 );
			 }
				 break;
		 };
	 }

protected:
	 /************************************************************************/
	 /**
	  * Loads the data of another object
	  *
	  * cp A pointer to another GAdaptorPropertyLoggerTT<num_type object, camouflaged as a GObject
	  */
	 void load_(const GObject* cp) override {
		 // Check that we are dealing with a GAdaptorPropertyLoggerT<num_type> reference independent of this object and convert the pointer
		 const GAdaptorPropertyLoggerT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerT<num_type>>(cp, this);

		 // Load the parent classes' data ...
		 GBasePluggableOM::load_(cp);

		 // ... and then our local data
		 m_fileName = p_load->m_fileName;
		 m_adaptorName = p_load->m_adaptorName;
		 m_property = p_load->m_property;
		 m_canvasDimensions = p_load->m_canvasDimensions;
		 m_gpd = p_load->m_gpd;
		 Gem::Common::copyCloneableSmartPointer(p_load->m_adaptorPropertyHist2D_oa, m_adaptorPropertyHist2D_oa);
		 Gem::Common::copyCloneableSmartPointer(p_load->m_fitnessGraph2D_oa, m_fitnessGraph2D_oa);
		 m_monitorBestOnly = p_load->m_monitorBestOnly;
		 m_addPrintCommand = p_load->m_addPrintCommand;
		 m_maxIteration = p_load->m_maxIteration;
		 m_nIterationsRecorded = p_load->m_nIterationsRecorded;
		 m_adaptorPropertyStore = p_load->m_adaptorPropertyStore;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Emits a name for this class / object
	  */
	 std::string name_() const override {
		 return std::string("GAdaptorPropertyLoggerT");
	 }

	 /************************************************************************/
	 /**
	  * Creates a deep clone of this object
	  */
	 GObject* clone_() const override {
		 return new GAdaptorPropertyLoggerT<num_type>(*this);
	 }

	 /************************************************************************/

	 std::string m_fileName = "NAdaptions.C"; ///< The name of the file to which solutions should be stored

	 std::string m_adaptorName = "GDoubleGaussAdaptor"; ///< The  name of the adaptor for which properties should be logged
	 std::string m_property = "sigma"; ///< The name of the property to be logged

	 std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions; ///< The dimensions of the canvas

	 Gem::Common::GPlotDesigner m_gpd; ///< A wrapper for the plots

	 std::shared_ptr<Gem::Common::GHistogram2D> m_adaptorPropertyHist2D_oa;  ///< Holds the actual histogram
	 std::shared_ptr<Gem::Common::GGraph2D>     m_fitnessGraph2D_oa;    ///< Lets us monitor the current fitness of the population

	 bool m_monitorBestOnly = false; ///< Indicates whether only the best individuals should be monitored
	 bool m_addPrintCommand = false; ///< Asks the GPlotDesigner to add a print command to result files

	 std::size_t m_maxIteration = 0; ///< Holds the largest iteration recorded for the algorithm
	 std::size_t m_nIterationsRecorded = 0; ///< Holds the number of iterations that were recorded (not necessarily == m_maxIteration

	 std::vector<std::tuple<double, double>> m_adaptorPropertyStore; ///< Holds all information about the number of adaptions

public:
	 /***************************************************************************/
	 /**
	  * Applies modifications to this object. This is needed for testing purposes
	  *
	  * @return A boolean which indicates whether modifications were made
	  */
	 bool modify_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 bool result = false;

		 // Call the parent classes' functions
		 if(GBasePluggableOM::modify_GUnitTests()) {
			 result = true;
		 }

		 // no local data -- nothing to change

		 return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GAdaptorPropertyLoggerT<num_type>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to succeed. This is needed for testing purposes
	  */
	 void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GAdaptorPropertyLoggerT<num_type>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }

	 /***************************************************************************/
	 /**
	  * Performs self tests that are expected to fail. This is needed for testing purposes
	  */
	 void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		 using boost::unit_test_framework::test_suite;
		 using boost::unit_test_framework::test_case;

		 // Call the parent classes' functions
		 GBasePluggableOM::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		 Gem::Common::condnotset("GAdaptorPropertyLoggerT<num_type>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	 }
	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log the time needed for the processing step of each
 * individual. The output happens in the form of two root files, one holding histograms
 * for the processing times, the other showing the distribution of processing times for
 * each iteration in a 2D histogram
 */
class GProcessingTimesLogger
	: public GBasePluggableOM
{
	 ///////////////////////////////////////////////////////////////////////

	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive & ar, const unsigned int){
		 using boost::serialization::make_nvp;

		 ar
		 & make_nvp("GBasePluggableOM",	boost::serialization::base_object<GBasePluggableOM>(*this))
		 & BOOST_SERIALIZATION_NVP(m_fileName_pth)
		 & BOOST_SERIALIZATION_NVP(m_canvasDimensions_pth)
		 & BOOST_SERIALIZATION_NVP(m_gpd_pth)
		 & BOOST_SERIALIZATION_NVP(m_fileName_pth2)
		 & BOOST_SERIALIZATION_NVP(m_canvasDimensions_pth2)
		 & BOOST_SERIALIZATION_NVP(m_gpd_pth2)
		 & BOOST_SERIALIZATION_NVP(m_fileName_txt)
		 & BOOST_SERIALIZATION_NVP(m_pre_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_post_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_all_processing_times_hist)
		 & BOOST_SERIALIZATION_NVP(m_pre_processing_times_hist2D)
		 & BOOST_SERIALIZATION_NVP(m_processing_times_hist2D)
		 & BOOST_SERIALIZATION_NVP(m_post_processing_times_hist2D)
		 & BOOST_SERIALIZATION_NVP(m_all_processing_times_hist2D)
		 & BOOST_SERIALIZATION_NVP(m_nBinsX)
		 & BOOST_SERIALIZATION_NVP(m_nBinsY);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/

	 /** @brief The default constructor */
	 G_API_GENEVA GProcessingTimesLogger();
	 /** @brief Initialization with a file name */
	 G_API_GENEVA GProcessingTimesLogger(
		 const std::string& fileName_pth
		 , const std::string& fileName_pth2
		 , const std::string& fileName_txt
		 , std::size_t nBinsX
		 , std::size_t nBinsY
	 );
	 /** @brief The copy constructor */
	 G_API_GENEVA GProcessingTimesLogger(const GProcessingTimesLogger& cp);
	 /** @brief  The destructor */
	 virtual G_API_GENEVA  ~GProcessingTimesLogger() = default;

	 /** @brief Searches for compliance with expectations with respect to another object of the same type */
	 virtual G_API_GENEVA  void compare(
		 const GObject& cp
		 , const Gem::Common::expectation& e
		 , const double& limit
	 ) const override;

	 /** @brief Sets the file name for the processing times histogram */
	 G_API_GENEVA void setFileName_pth(const std::string& fileName);
	 /** @brief Retrieves the current file name for the processing times histogram */
	 G_API_GENEVA std::string getFileName_pth() const;
	 /** @brief Sets the file name for the processing times histograms (2D) */
	 G_API_GENEVA void setFileName_pth2(const std::string& fileName);
	 /** @brief Retrieves the current file name for the processing times histograms (2D) */
	 G_API_GENEVA std::string getFileName_pth2() const;

	 /** @brief Sets the file name for the text output */
	 G_API_GENEVA void setFileName_txt(const std::string& fileName);
	 /** @brief Retrieves the current file name for the text output */
	 G_API_GENEVA std::string getFileName_txt() const;

	 /** @brief Allows to set the canvas dimensions for the processing times histograms */
	 G_API_GENEVA void setCanvasDimensions_pth(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions);
	 /** @brief Allows to set the canvas dimensions using separate x and y values for the processing times histograms */
	 G_API_GENEVA void setCanvasDimensions_pth(std::uint32_t x, std::uint32_t y);

	 /** @brief Gives access to the canvas dimensions of the processing times histograms */
	 G_API_GENEVA std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions_pth() const;
	 /** @brief Allows to set the canvas dimensions for the processing times histograms (2D) */
	 G_API_GENEVA void setCanvasDimensions_pth2(std::tuple<std::uint32_t,std::uint32_t> canvasDimensions);

	 /** @brief Allows to set the canvas dimensions using separate x and y values for the processing times histograms (2D) */
	 G_API_GENEVA void setCanvasDimensions_pth2(std::uint32_t x, std::uint32_t y);
	 /** @brief Gives access to the canvas dimensions of the processing times histograms (2D) */
	 G_API_GENEVA std::tuple<std::uint32_t,std::uint32_t> getCanvasDimensions_pth2() const;

	 /** @brief Sets the number of bins for the processing times histograms in y-direction */
	 G_API_GENEVA void setNBinsX(std::size_t nBinsX);
	 /** @brief Retrieves the current number of bins for the processing times histograms in x-direction */
	 G_API_GENEVA std::size_t getNBinsX() const;

	 /** @brief Sets the number of bins for the processing times histograms in y-direction */
	 G_API_GENEVA void setNBinsY(std::size_t nBinsY);
	 /** @brief Retrieves the current number of bins for the processing times histograms in y-direction */
	 G_API_GENEVA std::size_t getNBinsY() const;

	 /** @brief Allows to emit information in different stages of the information cycle */
	 virtual G_API_GENEVA  void informationFunction(
		 const infoMode& im
		 , G_OptimizationAlgorithm_Base *const goa
	 ) override;

protected:
	 /************************************************************************/

	 /** @brief Loads the data of another object */
	 G_API_GENEVA  void load_(const GObject* cp) override;

private:
	 /************************************************************************/
	 /** @brief Emits a name for this class / object */
	 G_API_GENEVA  std::string name_() const override;
	 /** @brief Creates a deep clone of this object */
	 G_API_GENEVA  GObject* clone_() const override;

	 /************************************************************************/

	 std::string m_fileName_pth = "processingTimingsHist.C"; ///< The name of the file to which timings should be written in ROOT format
	 std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions_pth; ///< The dimensions of the canvas
	 Gem::Common::GPlotDesigner m_gpd_pth; ///< A wrapper for the plots

	 std::string m_fileName_pth2 = "processingTimingsVsIteration.C"; ///< The name of the file to which timings should be written in ROOT format
	 std::tuple<std::uint32_t,std::uint32_t> m_canvasDimensions_pth2; ///< The dimensions of the canvas
	 Gem::Common::GPlotDesigner m_gpd_pth2; ///< A wrapper for the plots

	 std::string m_fileName_txt = "processingTimings.txt"; ///< The name of the file to which timings should be written in text format

	 std::shared_ptr<Gem::Common::GHistogram1D> m_pre_processing_times_hist;  ///< The amount of time needed for pre-processing
	 std::shared_ptr<Gem::Common::GHistogram1D> m_processing_times_hist;  ///< The amount of time needed for processing
	 std::shared_ptr<Gem::Common::GHistogram1D> m_post_processing_times_hist;  ///< The amount of time needed for post-processing
	 std::shared_ptr<Gem::Common::GHistogram1D> m_all_processing_times_hist;  ///< The amount of time needed for the entire processing step

	 std::shared_ptr<Gem::Common::GHistogram2D> m_pre_processing_times_hist2D;  ///< The amount of time needed for pre-processing
	 std::shared_ptr<Gem::Common::GHistogram2D> m_processing_times_hist2D;  ///< The amount of time needed for processing
	 std::shared_ptr<Gem::Common::GHistogram2D> m_post_processing_times_hist2D;  ///< The amount of time needed for post-processing
	 std::shared_ptr<Gem::Common::GHistogram2D> m_all_processing_times_hist2D;  ///< The amount of time needed for the entire processing step

	 std::size_t m_nBinsX = Gem::Common::DEFAULTNBINSGPD; ///< The number of bins in the histograms in x-direction
	 std::size_t m_nBinsY = Gem::Common::DEFAULTNBINSGPD; ///< The number of bins in the histograms in y-direction

public:
	 /***************************************************************************/

	 /** @brief Applies modifications to this object */
	 G_API_GENEVA  bool modify_GUnitTests() override;
	 /** @brief Performs self tests that are expected to succeed */
	 G_API_GENEVA  void specificTestsNoFailureExpected_GUnitTests() override;
	 /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	 G_API_GENEVA  void specificTestsFailuresExpected_GUnitTests() override;

	 /***************************************************************************/
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

using GProgressPlotter = GProgressPlotterT<double>;
template <typename num_type> using GAdaptorPropertyLogger = GAdaptorPropertyLoggerT<num_type>;

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// Exports of classes

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GStandardMonitor)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFitnessMonitor)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GCollectiveMonitor)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GProgressPlotter)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAllSolutionFileLogger)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GIterationResultsFileLogger)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GNAdpationsLogger)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<double>)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<std::int32_t>)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<bool>)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GProcessingTimesLogger)

/******************************************************************************/

