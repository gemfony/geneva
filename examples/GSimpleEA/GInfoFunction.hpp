/**
 * @file GInfoFunction.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/cstdint.hpp>

// Geneva header files go here
#include <courtier/GAsioHelperFunctions.hpp>
#include <courtier/GAsioTCPClientT.hpp>
#include <courtier/GAsioTCPConsumerT.hpp>
#include <geneva/GBrokerEA.hpp>
#include <geneva/GEvolutionaryAlgorithm.hpp>
#include <geneva/GIndividual.hpp>
#include <geneva/GDoubleCollection.hpp>

// The individual that should be optimized
#include "GFunctionIndividualDefines.hpp"
#include "GFunctionIndividual.hpp"

namespace Gem {
namespace Geneva {

/************************************************************************************************/
/**
 * The default dimension of the canvas in x-direction
 */
const boost::uint16_t DEFAULTXDIM=1024;

/**
 * The default dimension of the canvas in y-direction
 */
const boost::uint16_t DEFAULTYDIM=1024;

/************************************************************************************************/
/**
 * An information object that will also emit result information in every n-th iteration,
 * if requested.
 */
class optimizationMonitor {
public:
	/*********************************************************************************************/
	/**
	 * The standard constructor. All collected data will to be written to file
	 *
	 * @param df The id of the evaluation function
	 * @param summary The stream to which information should be written
	 */
	optimizationMonitor(const Gem::Geneva::demoFunction& df, std::ostream& summary)
		: df_(df)
		, summary_(summary)
		, xDim_(DEFAULTXDIM)
		, yDim_(DEFAULTYDIM)
		, followProgress_(false)
		, snapshotBaseName_("GEvolutionaryAlgorithmSnapshot")
		, minX_(-10.)
		, maxX_( 10.)
		, minY_(-10.)
		, maxY_( 10.)
		, outputPath_("./results/")
	{ /* nothing */  }

	/*********************************************************************************************/
	/**
	 * The function that does the actual collection of data. It can be called in
	 * three modes:
	 *
	 * INFOINIT: is called once before the optimization run.
	 * INFOPROCESSING: is called in regular intervals during the optimization, as determined by the user
	 * INFOEND: is called once after the optimization run
	 *
	 * @param im The current mode in which the function is called
	 * @param ea A pointer to a GEvolutionaryAlgorithm object for which information should be collected
	 */
	void informationFunction(const infoMode& im, GEvolutionaryAlgorithm * const ea){
		switch(im) {
		//---------------------------------------------------------------------------
		case Gem::Geneva::INFOINIT:
		{
			// Output the header to the summary stream
			summary_ << "{" << std::endl
					 << "  gROOT->Reset();" << std::endl
					 << "  gStyle->SetOptTitle(0);" << std::endl
					 << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0," << xDim_ << "," << yDim_ << ");" << std::endl
					 << std::endl
					 << "  std::vector<long> iteration;" << std::endl
					 << "  std::vector<double> evaluation;" << std::endl
					 << std::endl;
		}
		break;

		//---------------------------------------------------------------------------
		case Gem::Geneva::INFOPROCESSING:
		{
			bool isDirty = false;
			double currentEvaluation = 0.;

			// Retrieve the current iteration
			boost::uint32_t iteration = ea->getIteration();

			summary_ << "  iteration.push_back(" << iteration << ");" << std::endl;

			// Get access to the best inidividual
			boost::shared_ptr<GIndividual> gdii_ptr = ea->getBestIndividual<GIndividual>();

			// Retrieve the fitness of this individual
			currentEvaluation = gdii_ptr->getCurrentFitness(isDirty);

			// Let the audience know about the best result
			std::cout << iteration << ": " << currentEvaluation << std::endl;

			// Write information to the output stream
			summary_ << "  evaluation.push_back(" <<  currentEvaluation << ");" << (isDirty?" // dirty flag is set":"") << std::endl
			         << std::endl; // Improves readability when following the output with "tail -f"

			if(followProgress_) {
				takeSnapshot(ea);
			}
		}
		break;

		//---------------------------------------------------------------------------
		case Gem::Geneva::INFOEND:
		{
			// Output final print logic to the stream
			summary_ << "  // Transfer the vectors into arrays" << std::endl
					 << "  double iteration_arr[iteration.size()];" << std::endl
					 << "  double evaluation_arr[evaluation.size()];" << std::endl
					 << std::endl
					 << "  for(std::size_t i=0; i<iteration.size(); i++) {" << std::endl
					 << "     iteration_arr[i] = (double)iteration[i];" << std::endl
					 << "     evaluation_arr[i] = evaluation[i];" << std::endl
					 << "  }" << std::endl
					 << std::endl
					 << "  // Create a TGraph object" << std::endl
					 << "  TGraph *evGraph = new TGraph(evaluation.size(), iteration_arr, evaluation_arr);" << std::endl
					 << std::endl
					 << "  // Set the axis titles" << std::endl
					 << "  evGraph->GetXaxis()->SetTitle(\"Iteration\");" << std::endl
					 << "  evGraph->GetYaxis()->SetTitleOffset(1.1);" << std::endl
					 << "  evGraph->GetYaxis()->SetTitle(\"Fitness\");" << std::endl
					 << std::endl
					 << "  // Specify that plots should be done on a logarithmic scale" << std::endl
					 << "  cc->SetLogx();" << std::endl
					 << "  cc->SetLogy();" << std::endl
				     << "  // Do the actual drawing" << std::endl
				     << "  evGraph->Draw(\"APL\");" << std::endl
					 << "}" << std::endl;
		}
		break;

		//---------------------------------------------------------------------------
		default:
			break;
		};
	}

	/*********************************************************************************************/
	/**
	 * Allows to set the dimensions of the canvas
	 *
	 * @param xDim The desired dimension of the canvas in x-direction
	 * @param yDim The desired dimension of the canvas in y-direction
	 */
	void setDims(const boost::uint16_t& xDim, const boost::uint16_t& yDim) {
		xDim_ = xDim;
		yDim_ = yDim;
	}

	/*********************************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in x-direction
	 *
	 * @return The dimension of the canvas in x-direction
	 */
	boost::uint16_t getXDim() const {
		return xDim_;
	}

	/*********************************************************************************************/
	/**
	 * Retrieves the dimension of the canvas in y-direction
	 *
	 * @return The dimension of the canvas in y-direction
	 */
	boost::uint16_t getYDim() const {
		return yDim_;
	}

	/*********************************************************************************************/
	/**
	 * A snapshot of the individuals will be taken for every iteration that the optimizationMonitor
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
			error << "In optimizationMonitor::setXExtremes(): Error!" << std::endl
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
			error << "In optimizationMonitor::setXExtremes(): Error!" << std::endl
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

private:
	/*********************************************************************************************/
	/**
	 * Writes out a snapshot of the GEvolutionaryAlgorithm object we've been given for the current iteration. In
	 * the way it is implemented here, this function only makes sense for two-dimensional optimization
	 * problems. It is thus used for illustration purposes only.
	 *
	 * @param ea A pointer to a GEvolutionaryAlgorithm object
	 */
	void takeSnapshot(GEvolutionaryAlgorithm *ea) {
		boost::uint32_t iteration = ea->getIteration();
		std::string outputFileName = snapshotBaseName_ + "_" + boost::lexical_cast<std::string>(iteration) + ".C";

		// Check whether the output directory exists, otherwise create it
		boost::filesystem::path outputPath(outputPath_.c_str());
		if(!boost::filesystem::exists(outputPath)) {
			boost::filesystem::create_directory(outputPath);
			std::cout << "Created output directory " << outputPath_ << std::endl;
		}

		// Open a file stream
		std::ofstream ofs((outputPath_ + outputFileName).c_str());
		if(!ofs) {
			std::ostringstream error;
			error << "In optimizationMonitor::takeSnapshot(): Error!" << std::endl
				  << "Could not open output file " << outputFileName << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Retrieve the globally best individual for later use
		boost::shared_ptr<GParameterSet> g_best_ptr = ea->getBestIndividual<GParameterSet>();
		// Extract the fitness
		bool isDirty;
		double global_best_fitness = g_best_ptr->getCurrentFitness(isDirty);
#ifdef DEBUG
		// Check that the dirty flag isn't set
		if(isDirty) {
			std::ostringstream error;
			error << "In optimizationMonitor::takeSnapshot(): Error!" << std::endl
				  << "Globally best individual has dirty flag set when it shouldn't" << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#endif

		// Output a ROOT header
		ofs << "{" << std::endl
			<< "  gROOT->Reset();" << std::endl
			<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0," << xDim_ << "," << yDim_ << ");" << std::endl
			<< "  gStyle->SetTitle(\"" << (GFunctionIndividual<>::getStringRepresentation(df_) + " / iteration = " + boost::lexical_cast<std::string>(iteration)) + " / fitness = "<< global_best_fitness << "\");" << std::endl
			<< std::endl
		    << "  TF2 *tf = new TF2(\"tf\", \"" << GFunctionIndividual<>::get2DROOTFunction(df_) << "\", " << minX_ << ", " << maxX_ << ", " << minY_ << ", " << maxY_ << ");" << std::endl
			<< "  tf->SetLineWidth(0.05);" << std::endl
			<< "  tf->SetLineColor(16);" << std::endl
			<< "  tf->GetXaxis()->SetLabelSize(0.02);" << std::endl
			<< "  tf->GetYaxis()->SetLabelSize(0.02);" << std::endl
			<< "  tf->GetHistogram()->SetTitle(\"" << (GFunctionIndividual<>::getStringRepresentation(df_) + " / iteration " + boost::lexical_cast<std::string>(iteration)) + " / fitness = "<< global_best_fitness << "\");"
			<< std::endl
			<< "  tf->Draw();" << std::endl
			<< std::endl;

		// Draw lines where the global optima are
		std::vector<double> f_x_mins = GFunctionIndividual<>::getXMin(df_);
		std::vector<double> f_y_mins = GFunctionIndividual<>::getYMin(df_);
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
		ofs << std::endl;

		// Extract the parents and mark them in the plot
		for(std::size_t parentId=0; parentId < ea->getNParents(); parentId++) {
			boost::shared_ptr<GParameterSet> p_ptr = ea->getParentIndividual<GParameterSet>(parentId);

			// std::cout << "================ Iteration " << ea->getIteration() << " / Parent = " << parentId << " =================" << std::endl
			//	         << p_ptr->toString(Gem::Common::SERIALIZATIONMODE_XML) << std::endl;

			// Extract the coordinates
			double x_parent = p_ptr->pc_at<GDoubleCollection>(0)->at(0);
			double y_parent = p_ptr->pc_at<GDoubleCollection>(0)->at(1);

			// Add to the plot, if the marker would still be inside the main drawing area
			if(x_parent > minX_ && x_parent < maxX_ && y_parent > minY_ && y_parent < maxY_) {
				ofs << "  TMarker *parent_marker" << parentId << " = new TMarker(" << x_parent << ", " << y_parent << ", 22);" << std::endl // A circle
					<< "  parent_marker" << parentId << "->SetMarkerColor(4);" << std::endl
					<< "  parent_marker" << parentId << "->SetMarkerSize(1.3);" << std::endl
					<< "  parent_marker" << parentId << "->Draw();" << std::endl
					<< std::endl;
			}
		}

		// Extract the coordinates of the globally best individual
		double x_global_best = g_best_ptr->pc_at<GDoubleCollection>(0)->at(0);
		double y_global_best = g_best_ptr->pc_at<GDoubleCollection>(0)->at(1);

		// Add to the plot, if the marker would still be inside the main drawing area
		if(x_global_best > minX_ && x_global_best < maxX_ && y_global_best > minY_ && y_global_best < maxY_) {
			ofs << "  TMarker *gbest = new TMarker(" << x_global_best << ", " << y_global_best << ", 8);" << std::endl // A circle
				<< "  gbest->SetMarkerColor(2);" << std::endl
				<< "  gbest->SetMarkerSize(1.8);" << std::endl
				<< "  gbest->Draw();" << std::endl
				<< std::endl;
		}

		// Loop over all individuals in this iteration and output their parameters
		GEvolutionaryAlgorithm::iterator it;
		std::size_t cind = 0;
		for(it=ea->begin() + ea->getNParents(); it!=ea->end(); ++it) {
			// Retrieve the data members
			boost::shared_ptr<GDoubleCollection> x = boost::dynamic_pointer_cast<GParameterSet>(*it)->pc_at<GDoubleCollection>(0);
			// Store a reference for ease of access
			const GDoubleCollection& x_ref = *x;
#ifdef DEBUG
			// Check that we indeed only have two dimensions
			if(x_ref.size() != 2) {
				std::ostringstream error;
				error << "In optimizationMonitor::takeSnapshot(): Error!" << std::endl
					  << "Found GDoubleCollection with invalid number of entries: " << x_ref.size() << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}
#endif

			// Only draw the child if it is inside of the function plot
			if(x_ref[0] > minX_ && x_ref[0] < maxX_ && x_ref[1] > minY_ && x_ref[1] < maxY_) {
				ofs << "  TMarker *child_marker_" << cind << " = new TMarker(" << x_ref[0] << ", " << x_ref[1] << ", 8);" << std::endl // A circle
					<< "  child_marker_" << cind << "->SetMarkerColor(1);" << std::endl
					<< "  child_marker_" << cind << "->SetMarkerSize(1.1);" << std::endl
					<< "  child_marker_" << cind << "->Draw();" << std::endl
					<< std::endl;
			}

			cind++;
		}

		ofs << std::endl
			<< "  cc->Print(\"" << (snapshotBaseName_ + "_" + boost::lexical_cast<std::string>(iteration) + ".jpg") << "\");" << std::endl
			<< "}" << std::endl;

		// Close the file stream
		ofs.close();
	}

	/*********************************************************************************************/
	optimizationMonitor(); ///< Intentionally left undefined

	std::ostream& summary_; ///< The stream to which information is written
	boost::uint16_t xDim_; ///< The dimension of the canvas in x-direction
	boost::uint16_t yDim_; ///< The dimension of the canvas in y-direction
	demoFunction df_; ///< The id of the evaluation function
	bool followProgress_; ///< Indicates whether a snapshot of the current individuals should be taken whenever the infoFunction is called
	std::string snapshotBaseName_; ///< The base name of the snapshot file
	double minX_, maxX_; ///< Minimal and maximal x values for snapshots
	double minY_, maxY_; ///< Minimal and maximal y values for snapshots
	std::string outputPath_; ///< The output directory for snapshots
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
