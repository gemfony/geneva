/**
 * @file GInfoFunction.hpp
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
#include <fstream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include "courtier/GAsioHelperFunctions.hpp"
#include "courtier/GAsioTCPConsumerT.hpp"
#include "geneva/GSerialSwarm.hpp"
#include "geneva/GOptimizableEntity.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GOptimizationEnums.hpp"

// The individual that should be optimized
#include "geneva-individuals/GFunctionIndividual.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************************************/
/**
 * The default dimension of the canvas in x-direction
 */
const boost::uint16_t DEFAULTXDIMPROGRESS=1024;

/**
 * The default dimension of the canvas in y-direction
 */
const boost::uint16_t DEFAULTYDIMPROGRESS=1024;

/************************************************************************************************/
/**
 * This class will visualize the progress of an evaluation procedure when called for
 * two-dimensional parameter sets. It will in any case produce plots for the achieved
 * fitness as a function of the current iteration.
 */
class progressMonitor
	: public GBaseSwarm::GSwarmOptimizationMonitor
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GBaseSwarm_GSwarmOptimizationMonitor", boost::serialization::base_object<GBaseSwarm::GSwarmOptimizationMonitor>(*this))
	  	 & BOOST_SERIALIZATION_NVP(xDimProgress_)
	  	 & BOOST_SERIALIZATION_NVP(yDimProgress_)
	  	 & BOOST_SERIALIZATION_NVP(df_)
	  	 & BOOST_SERIALIZATION_NVP(followProgress_)
	  	 & BOOST_SERIALIZATION_NVP(snapshotBaseName_)
	  	 & BOOST_SERIALIZATION_NVP(minX_)
	  	 & BOOST_SERIALIZATION_NVP(maxX_)
	  	 & BOOST_SERIALIZATION_NVP(minY_)
	  	 & BOOST_SERIALIZATION_NVP(maxY_)
	  	 & BOOST_SERIALIZATION_NVP(outputPath_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/*********************************************************************************************/
	/**
	 * The standard constructor. All collected data will be written to file
	 *
	 * @param df The id of the evaluation function
	 * @param summary The stream to which information should be written
	 */
	progressMonitor(const Gem::Geneva::solverFunction& df)
		: xDimProgress_(DEFAULTXDIMPROGRESS)
		, yDimProgress_(DEFAULTYDIMPROGRESS)
		, df_(df)
		, followProgress_(false)
		, snapshotBaseName_("GSwarmAlgorithmSnapshot")
		, minX_(-10.)
		, maxX_( 10.)
		, minY_(-10.)
		, maxY_( 10.)
		, outputPath_("./results/")
	{ /* nothing */  }

	/**********************************************************************************/
	/**
	 * A simple copy constructor
	 *
	 * @oaram cp A copy of another progressMonitor object
	 */
	progressMonitor(const progressMonitor& cp)
		: GBaseSwarm::GSwarmOptimizationMonitor(cp)
		, xDimProgress_(cp.xDimProgress_)
		, yDimProgress_(cp.yDimProgress_)
		, df_(cp.df_)
		, followProgress_(cp.followProgress_)
		, snapshotBaseName_(cp.snapshotBaseName_)
		, minX_(cp.minX_)
		, maxX_(cp.maxX_)
		, minY_(cp.minY_)
		, maxY_(cp.maxY_)
		, outputPath_(cp.outputPath_)
	{ /* nothing */ }

   /**********************************************************************************/
   /**
    * A simple destructor
    */
   virtual ~progressMonitor()
   { /* nothing */ }

	/**********************************************************************************/
	/**
	 * A standard assignment operator.
	 *
	 * @param cp A copy of another GEAOptimizationMonitor object
	 * @return A constant reference to this object
	 */
	const progressMonitor& operator=(const progressMonitor& cp){
		load_(&cp);
		return *this;
	}

	/**********************************************************************************/
	/**
	 * Checks for equality with another GParameter Base object
	 *
	 * @param  cp A constant reference to another GEAOptimizationMonitor object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const progressMonitor& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
	}

	/**********************************************************************************/
	/**
	 * Checks for inequality with another GEAOptimizationMonitor object
	 *
	 * @param  cp A constant reference to another GEAOptimizationMonitor object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const progressMonitor& cp) const {
      using namespace Gem::Common;
      try {
         this->compare(cp, CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
         return true;
      } catch(g_expectation_violation&) {
         return false;
      }
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
   ) const OVERRIDE {
      using namespace Gem::Common;

      // Check that we are indeed dealing with a GAdaptorT reference
      const progressMonitor *p_load = GObject::gobject_conversion<progressMonitor >(&cp);

      try {
         BEGIN_COMPARE;

         // Check our parent class'es data ...
         COMPARE_PARENT(GBaseSwarm::GSwarmOptimizationMonitor, cp, e, limit);

         // ... and then our local data
         COMPARE(xDimProgress_, p_load->xDimProgress_, e, limit);
         COMPARE(yDimProgress_, p_load->yDimProgress_, e, limit);
         COMPARE(df_, p_load->df_, e, limit);
         COMPARE(followProgress_, p_load->followProgress_, e, limit);
         COMPARE(snapshotBaseName_, p_load->snapshotBaseName_, e, limit);
         COMPARE(minX_, p_load->minX_, e, limit);
         COMPARE(maxX_, p_load->maxX_, e, limit);
         COMPARE(minY_, p_load->minY_, e, limit);
         COMPARE(maxY_, p_load->maxY_, e, limit);
         COMPARE(xDimProgress_, p_load->xDimProgress_, e, limit);
         COMPARE(yDimProgress_, p_load->yDimProgress_, e, limit);
         COMPARE(outputPath_, p_load->outputPath_, e, limit);

         END_COMPARE;

      } catch(g_expectation_violation& g) { // Create a suitable stack-trace
         throw g("g_expectation_violation caught by GAdaptorT<T>");
      }
   }

	/*********************************************************************************************/
	/**
	 * Allows to set the dimensions of the canvas
	 *
	 * @param xDimProgress The desired dimension of the canvas in x-direction
	 * @param yDimProgress The desired dimension of the canvas in y-direction
	 */
	void setProgressDims(const boost::uint16_t& xDimProgress, const boost::uint16_t& yDimProgress) {
		xDimProgress_ = xDimProgress;
		yDimProgress_ = yDimProgress;
	}

	/*********************************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in x-direction
	 *
	 * @return The dimension of the canvas in x-direction
	 */
	boost::uint16_t getXDimProgress() const {
		return xDimProgress_;
	}

	/*********************************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in y-direction
	 *
	 * @return The dimension of the canvas in y-direction
	 */
	boost::uint16_t getYDimProgress() const {
		return yDimProgress_;
	}

	/*********************************************************************************************/
	/**
	 * A snapshot of the individuals will be taken for every iteration that the progressMonitor
	 * is called for, when the followProgress_ flag is set.
	 *
	 * @param followProgress_ The desired new value of the followProgress_ variable
	 */
	void setFollowProgress(bool followProgress) {
		followProgress_ = followProgress;
	}

	/*********************************************************************************************/
	/**
	 * Retrieves the current value of the followProgress_ flag
	 *
	 * @return The current value of the followProgress_ flag
	 */
	bool getFollowProgress() const {
		return followProgress_;
	}

	/*********************************************************************************************/
	/**
	 * Allows to set the base name used for snapshot files
	 *
	 * @param snapshotBaseName The desired base name for snapshot files
	 */
	void setSnapshotBaseName(const std::string& snapshotBaseName) {
		snapshotBaseName_ = snapshotBaseName;
	}

	/*********************************************************************************************/
	/**
	 * Allows to retrieve the current base name used for snapshot files
	 */
	std::string getSnapshotBaseName() const {
		return snapshotBaseName_;
	}

	/*********************************************************************************************/
	/**
	 * Allows to set the extreme x values for snapshot plots
	 *
	 * @param minX The minimal allowed value in x-direction
	 * @param maxX The minimal allowed value in x-direction
	 */
	void setXExtremes(const double& minX, const double& maxX) {
		if(minX >= maxX) {
			std::ostringstream error;
			error << "In progressMonitor::setXExtremes(): Error!" << std::endl
				  << "Invalid min/max x values provided: " << minX << " / " << maxX << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		minX_ = minX;
		maxX_ = maxX;
	}

	/*********************************************************************************************/
	/**
	 * Allows to set the extreme y values for snapshot plots
	 *
	 * @param minY The minimal allowed value in y-direction
	 * @param maxY The minimal allowed value in y-direction
	 */
	void setYExtremes(const double& minY, const double& maxY) {
		if(minY >= maxY) {
			std::ostringstream error;
			error << "In progressMonitor::setYExtremes(): Error!" << std::endl
				  << "Invalid min/max y values provided: " << minY << " / " << maxY << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		minY_ = minY;
		maxY_ = maxY;
	}

	/*********************************************************************************************/
	/**
	 * Allows to retrieve the minimal allowed value in x-direction for snapshots
	 *
	 * @return The value of the minX_ variable
	 */
	double getMinX() const {
		return minX_;
	}

	/*********************************************************************************************/
	/**
	 * Allows to retrieve the maximal allowed value in x-direction for snapshots
	 *
	 * @return The value of the maxX_ variable
	 */
	double getMaxX() const {
		return maxX_;
	}

	/*********************************************************************************************/
	/**
	 * Allows to retrieve the minimal allowed value in y-direction for snapshots
	 *
	 * @return The value of the minY_ variable
	 */
	double getMinY() const {
		return minY_;
	}

	/*********************************************************************************************/
	/**
	 * Allows to retrieve the maximal allowed value in y-direction for snapshots
	 *
	 * @return The value of the maxY_ variable
	 */
	double getMaxY() const {
		return maxY_;
	}

protected:
	/*********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object
	 */
	GObject* clone_() const {
		return new progressMonitor(*this);
	}

	/*********************************************************************************************/
	/**
	 * Loads the data of another progressMonitor object, camouflaged as a GObject.
	 *
	 * @param cp A pointer to another progressMonitor object, camouflaged as a GObject
	 */
	void load_(const GObject * cp)
	{
		const progressMonitor *p_load = gobject_conversion<progressMonitor>(cp);

		// First load the parent class'es data ...
		GSerialSwarm::GSwarmOptimizationMonitor::load_(cp);

		// ... and then our own data
		xDimProgress_ = p_load->xDimProgress_;
		yDimProgress_ = p_load->yDimProgress_;
		df_ = p_load->df_;
		followProgress_ = p_load->followProgress_;
		snapshotBaseName_ = p_load->snapshotBaseName_;
		minX_ = p_load->minX_;
		maxX_ = p_load->maxX_;
		minY_ = p_load->minY_;
		maxY_ = p_load->maxY_;
		outputPath_ = p_load->outputPath_;
	}

	/**********************************************************************************/
	/**
	 * A function that is called during each optimization cycle, acting on evolutionary
	 * algorithms. It writes out a snapshot of the GBaseSwarm object we've
	 * been given for the current iteration. In the way it is implemented here, this
	 * function only makes sense for two-dimensional optimization problems. It is thus
	 * used for illustration purposes only.
	 */
	virtual void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
      // Convert the base pointer to the target type
      GBaseSwarm * const swarm = static_cast<GBaseSwarm * const>(goa);

		if(followProgress_) {
			boost::uint32_t iteration = swarm->getIteration();
			std::string outputFileName = snapshotBaseName_ + "_" + boost::lexical_cast<std::string>(iteration) + ".C";

			// Check whether the output directory exists, otherwise create it
			boost::filesystem::path outputPath(outputPath_.c_str());
			if(!boost::filesystem::exists(outputPath)) {
				boost::filesystem::create_directory(outputPath);
				std::cout << "Created output directory " << outputPath_ << std::endl;
			}

			// Open a file stream
			boost::filesystem::ofstream ofs(boost::filesystem::path(outputPath_ + outputFileName));
			if(!ofs) {
				std::ostringstream error;
				error << "In progressMonitor::swarmCycleInformation(): Error!" << std::endl
					  << "Could not open output file " << outputFileName << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			// Retrieve the globally best individual for later use
			boost::shared_ptr<GParameterSet> g_best_ptr = swarm->GOptimizableI::getBestIndividual<GParameterSet>();
         // Extract the fitness (Note: this will throw, if the individual is "dirty")
         double global_best_fitness = g_best_ptr->transformedFitness();

			// Output a ROOT header
			ofs << "{" << std::endl
				<< "  gROOT->Reset();" << std::endl
				<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0," << xDimProgress_ << "," << yDimProgress_ << ");" << std::endl
				<< "  gStyle->SetTitle(\"" << (GFunctionIndividual::getStringRepresentation(df_) + " / iteration = " + boost::lexical_cast<std::string>(iteration)) + " / fitness = "<< global_best_fitness << "\");" << std::endl
				<< std::endl
			    << "  TF2 *tf = new TF2(\"tf\", \"" << GFunctionIndividual::get2DROOTFunction(df_) << "\", " << minX_ << ", " << maxX_ << ", " << minY_ << ", " << maxY_ << ");" << std::endl
				<< "  tf->SetLineWidth(0.05);" << std::endl
				<< "  tf->SetLineColor(16);" << std::endl
				<< "  tf->GetXaxis()->SetLabelSize(0.02);" << std::endl
				<< "  tf->GetYaxis()->SetLabelSize(0.02);" << std::endl
				<< "  tf->GetHistogram()->SetTitle(\"" << (GFunctionIndividual::getStringRepresentation(df_) + " / iteration " + boost::lexical_cast<std::string>(iteration)) + " / fitness = "<< global_best_fitness << "\");"
				<< std::endl
				<< "  tf->Draw();" << std::endl
				<< std::endl;

			// Draw lines where the global optima are
			std::vector<double> f_x_mins = GFunctionIndividual::getXMin(df_);
			std::vector<double> f_y_mins = GFunctionIndividual::getYMin(df_);

			ofs << "  //============================================================" << std::endl
			    << "  // Minima and maxima" << std::endl
			    << std::endl;
			for(std::size_t i=0; i<f_x_mins.size(); i++) {
				ofs << "  TLine *tlx" << i << " = new TLine(" << f_x_mins[i] << ", " << minY_ << ", " << f_x_mins[i] << ", " << maxY_ << ");" << std::endl
					<< "  tlx" << i << "->SetLineStyle(5);" << std::endl
					<< "  tlx" << i << "->SetLineColor(45);" << std::endl
					<< "  tlx" << i << "->Draw();" << std::endl;
			}
			for(std::size_t i=0; i<f_y_mins.size(); i++) {
				ofs << "  TLine *tly" << i << " = new TLine(" << minX_ << ", " << f_y_mins[i] << ", " << maxX_ << ", " << f_y_mins[i] << ");" << std::endl
					<< "  tly" << i << "->SetLineStyle(5);" << std::endl
					<< "  tly" << i << "->SetLineColor(45);" << std::endl
					<< "  tly" << i << "->Draw();" << std::endl;
			}
			ofs << std::endl
			    << "  //============================================================" << std::endl
				<< "  // Neighborhood bests" << std::endl
				<< std::endl;

			// Extract the locally best individuals and mark them in the plot
			for(std::size_t neighborhood=0; neighborhood<swarm->getNNeighborhoods(); neighborhood++) {
				boost::shared_ptr<GParameterSet> l_best_ptr = swarm->getBestNeighborhoodIndividual<GParameterSet>(neighborhood);

				// Extract the coordinates
				double x_local_best = l_best_ptr->at<GDoubleCollection>(0)->at(0);
				double y_local_best = l_best_ptr->at<GDoubleCollection>(0)->at(1);

				// Add to the plot, if the marker would still be inside the main drawing area
				if(x_local_best > minX_ && x_local_best < maxX_ && y_local_best > minY_ && y_local_best < maxY_) {
					ofs << "  TMarker *lbest" << neighborhood << " = new TMarker(" << x_local_best << ", " << y_local_best << ", 22);" << std::endl // A circle
						<< "  lbest" << neighborhood << "->SetMarkerColor(4);" << std::endl
						<< "  lbest" << neighborhood << "->SetMarkerSize(1.3);" << std::endl
						<< "  lbest" << neighborhood << "->Draw();" << std::endl
						<< std::endl;
				} else { // Mark entries as being outside of the drawing area
					ofs << "  // TMarker *lbest" << neighborhood << " = new TMarker(" << x_local_best << ", " << y_local_best << ", 22); /* Marker outside of drawing area! */" << std::endl
						<< "  // lbest" << neighborhood << "->SetMarkerColor(4);" << std::endl
						<< "  // lbest" << neighborhood << "->SetMarkerSize(1.3);" << std::endl
						<< "  // lbest" << neighborhood << "->Draw();" << std::endl
						<< std::endl;
				}
			}

			ofs << std::endl
				<< "  //============================================================" << std::endl
				<< "  // Global best" << std::endl
				<< std::endl;

			// Extract the coordinates of the globally best individual and mark them in the plot
			double x_global_best = g_best_ptr->at<GDoubleCollection>(0)->at(0);
			double y_global_best = g_best_ptr->at<GDoubleCollection>(0)->at(1);

			// Add to the plot, if the marker would still be inside the main drawing area
			if(x_global_best > minX_ && x_global_best < maxX_ && y_global_best > minY_ && y_global_best < maxY_) {
				ofs << "  TMarker *gbest = new TMarker(" << x_global_best << ", " << y_global_best << ", 8);" << std::endl // A circle
					<< "  gbest->SetMarkerColor(2);" << std::endl
					<< "  gbest->SetMarkerSize(1.8);" << std::endl
					<< "  gbest->Draw();" << std::endl
					<< std::endl;
			} else {
				ofs << "  // TMarker *gbest = new TMarker(" << x_global_best << ", " << y_global_best << ", 8); /* Marker outside of drawing area! */" << std::endl
					<< "  // gbest->SetMarkerColor(2);" << std::endl
					<< "  // gbest->SetMarkerSize(1.8);" << std::endl
					<< "  // gbest->Draw();" << std::endl
					<< std::endl;
			}


			// Loop over all individuals in this iteration and output their parameters
			ofs << std::endl
				<< "  //============================================================" << std::endl
				<< "  // Individuals" << std::endl
				<< std::endl;

			GBaseSwarm::iterator it;
			std::size_t particle = 0;
			for(it=swarm->begin(); it!=swarm->end(); ++it, ++particle) {
				// Retrieve the data members
				boost::shared_ptr<GDoubleCollection> x = (*it)->at<GDoubleCollection>(0);
				// Store a reference for ease of access
				const GDoubleCollection& x_ref = *x;
#ifdef DEBUG
				// Check that we indeed only have two dimensions
				if(x_ref.size() != 2) {
					std::ostringstream error;
					error << "In progressMonitor::cycleInformation(): Error!" << std::endl
						  << "Found GDoubleCollection with invalid number of entries: " << x_ref.size() << std::endl;
					throw(Gem::Common::gemfony_error_condition(error.str()));
				}
#endif /* DEBUG */

				// Only draw the particle if it is inside of the function plot
				if(x_ref[0] > minX_ && x_ref[0] < maxX_ && x_ref[1] > minY_ && x_ref[1] < maxY_) {
					ofs << "  TText txt_" << particle << "(" << x_ref[0] << ", " << x_ref[1] << ", \"" << (*it)->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood() << "\");" << std::endl
						<< "  txt_" << particle << "->SetTextSize(0.013);" << std::endl
						<< "  txt_" << particle << "->Draw();" << std::endl
						<< std::endl;
				} else {
					ofs << "  // TText txt_" << particle << "(" << x_ref[0] << ", " << x_ref[1] << ", \"" << (*it)->getPersonalityTraits<GSwarmPersonalityTraits>()->getNeighborhood() << "\"); /* Marker outside of drawing area! */" << std::endl
						<< "  // txt_" << particle << "->SetTextSize(0.013);" << std::endl
						<< "  // txt_" << particle << "->Draw();" << std::endl
						<< std::endl;
				}
			}

			ofs << std::endl
				<< "  //============================================================" << std::endl
				<< "  // Plotting" << std::endl
				<< std::endl
				<< "  cc->Print(\"" << (snapshotBaseName_ + "_" + boost::lexical_cast<std::string>(iteration) + ".jpg") << "\");" << std::endl
				<< "}" << std::endl;

			// Close the file stream
			ofs.close();
		}

		//-----------------------------------------------------------------------------------------

		// Make sure the usual iteration work is performed
		GSerialSwarm::GSwarmOptimizationMonitor::cycleInformation(goa);
	}

private:
	/*********************************************************************************************/
	progressMonitor(); ///< Intentionally left undefined

	boost::uint16_t xDimProgress_; ///< The dimension of the canvas in x-direction
	boost::uint16_t yDimProgress_; ///< The dimension of the canvas in y-direction
	solverFunction df_; ///< The id of the evaluation function
	bool followProgress_; ///< Indicates whether a snapshot of the current individuals should be taken whenever the infoFunction is called
	std::string snapshotBaseName_; ///< The base name of the snapshot file
	double minX_, maxX_; ///< Minimal and maximal x values for snapshots
	double minY_, maxY_; ///< Minimal and maximal y values for snapshots
	std::string outputPath_; ///< The output directory for snapshots
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
