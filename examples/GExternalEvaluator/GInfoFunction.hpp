/**
 * @file GInfoFunction.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <cmath>
#include <sstream>
#include <limits>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

// Geneva header files go here
#include <courtier/GAsioHelperFunctions.hpp>
#include <courtier/GAsioTCPClientT.hpp>
#include <courtier/GAsioTCPConsumerT.hpp>
#include <geneva/GBrokerEA.hpp>
#include <geneva/GEvolutionaryAlgorithm.hpp>
#include <geneva/GIndividual.hpp>
#include <geneva/GMultiThreadedEA.hpp>

// The individual that should be optimized
#include "GExternalEvaluatorIndividual.hpp"

namespace Gem {
namespace Geneva {
/************************************************************************************************/
/**
 * An information object that will also emit result information in every n-th generation,
 * if requested.
 */
class optimizationMonitor{
public:
	/*********************************************************************************************/
	/**
	 * The standard constructor. All collected data will to be written to file
	 *
	 * @param nInfoIndividuals The amount of individuals for which information should be emitted
	 * @param summary The stream to which information should be written
	 */
	optimizationMonitor(const std::size_t nInfoIndividuals, std::ostream& summary):
		nInfoIndividuals_(nInfoIndividuals),
		summary_(summary)
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
	 * @param gbp A pointer to a GEvolutionaryAlgorithm object for which information should be collected
	 */
	void informationFunction(const infoMode& im, GEvolutionaryAlgorithm * const gbp){
		switch(im) {
		//---------------------------------------------------------------------------
		case Gem::Geneva::INFOINIT:
		{
			// Output the header to the summary stream
			summary_ << "{" << std::endl
					<< "  gROOT->Reset();" << std::endl
					<< "  gStyle->SetOptTitle(0);" << std::endl
					<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);" << std::endl
					<< "  cc->Divide(2,2);" << std::endl
					<< std::endl
					<< "  std::vector<long> generation;" << std::endl;

			for(std::size_t p=0; p<nInfoIndividuals_; p++) {
				summary_ << "  std::vector<double> evaluation" << p << ";" << std::endl
						<< "  std::vector<double> sigma" << p << ";" << std::endl
						<< "  std::vector<double> minSigma" << p << ";" << std::endl
						<< "  std::vector<double> maxSigma" << p << ";" << std::endl
						<< std::endl;
			}
		}
		break;

		//---------------------------------------------------------------------------
		case Gem::Geneva::INFOPROCESSING:
		{
			bool isDirty = false;
			double currentEvaluation = 0.;

			// Retrieve the current generation
			boost::uint32_t generation = gbp->getIteration();

			summary_ << "  generation.push_back(" << generation << ");" << std::endl;

			for(std::size_t p=0; p<nInfoIndividuals_; p++) {
				// Get access to the inidividual
				boost::shared_ptr<GExternalEvaluatorIndividual> gdii_ptr = gbp->individual_cast<GExternalEvaluatorIndividual>(p);

				// Extract the GConstrainedDoubleObjectCollection object, so we can get information about sigma
				std::vector<boost::shared_ptr<GConstrainedDoubleObjectCollection> > v;
				gdii_ptr->attachViewTo(v);

				std::size_t nSigmas = 0;
				double sigmaSum=0.;
				double minSigma=std::numeric_limits<double>::max();
				double maxSigma=-std::numeric_limits<double>::max();

				std::vector<boost::shared_ptr<GConstrainedDoubleObjectCollection> >::iterator it;
				for(it=v.begin(); it!=v.end(); ++it) {
					// We need to loop over all GConstrainedDoubleObject objects to extract the desired information
					GConstrainedDoubleObjectCollection::iterator gbdc_it;
					for(gbdc_it=(*it)->begin(); gbdc_it!=(*it)->end(); ++gbdc_it) {
#ifdef DEBUG
						if(!(*gbdc_it)->hasAdaptor()) { // This should not happen
							std::ostringstream error;
							error << "In optimizationMonitor::informationFunction(INFOPROCESSING): Error!" << std::endl
									<< "Expected an adaptor in GConstrainedDoubleObject object but didn't find it." << std::endl;
							throw(Gem::Common::gemfony_error_condition(error.str()));
						}
#endif /* DEBUG */

						// Extract the adaptor
						boost::shared_ptr<GDoubleGaussAdaptor> ad_ptr = (*gbdc_it)->adaptor_cast<GDoubleGaussAdaptor>();

						// Get the sigma and sum it up
						double sigma = ad_ptr->getSigma();
						sigmaSum += sigma;
						if(sigma < minSigma) minSigma = sigma;
						if(sigma > maxSigma) maxSigma = sigma;
						nSigmas++;
					}
				}

				// Scale the overall sigmaSum according to the number of variables
				sigmaSum /= nSigmas;

				// Retrieve the fitness of this individual
				currentEvaluation = gdii_ptr->getCurrentFitness(isDirty);

				// Write the evaluation to the output stream
				summary_ << "  evaluation" << p << ".push_back(" <<  currentEvaluation << ");" << (isDirty?" // dirty flag is set":"") << std::endl;

				// Write the sigma values to the output stream
				summary_ << "  sigma" << p << ".push_back(" << sigmaSum << ");" << std::endl
						<< "  minSigma" << p << ".push_back(" << minSigma << ");" << std::endl
						<< "  maxSigma" << p << ".push_back(" << maxSigma << ");" << std::endl;

				// Let the audience know about the best result
				if(p==0) {
					std::cout << generation << ": " << currentEvaluation << std::endl;
				}
			}
			summary_ << std::endl; // Improves readability when following the output with "tail -f"
		}
		break;

		//---------------------------------------------------------------------------
		case Gem::Geneva::INFOEND:
		{
			// Output final print logic to the stream
			summary_ << "  // Transfer the vectors into arrays" << std::endl
					<< "  double generation_arr[generation.size()];" << std::endl;
			for(std::size_t p=0; p<nInfoIndividuals_; p++) {
				summary_ << "  double evaluation" << p << "_arr[evaluation" << p << ".size()];" << std::endl
						<< "  double sigma" << p << "_arr[evaluation" << p << ".size()];" << std::endl
						<< "  double minSigma" << p << "_arr[evaluation" << p << ".size()];" << std::endl
						<< "  double maxSigma" << p << "_arr[evaluation" << p << ".size()];" << std::endl
						<< std::endl
						<< "  for(std::size_t i=0; i<generation.size(); i++) {" << std::endl;

				if(p==0) summary_ << "     generation_arr[i] = (double)generation[i];" << std::endl;

				summary_ << "     evaluation" << p << "_arr[i] = evaluation" << p << "[i];" << std::endl
						<< "     sigma" << p << "_arr[i] = sigma" << p << "[i];" << std::endl
						<< "     minSigma" << p << "_arr[i] = minSigma" << p << "[i];" << std::endl
						<< "     maxSigma" << p << "_arr[i] = maxSigma" << p << "[i];" << std::endl
						<< "  }" << std::endl
						<< std::endl
						<< "  // Create a TGraph object" << std::endl
						<< "  TGraph *evGraph" << p << " = new TGraph(evaluation" << p << ".size(), generation_arr, evaluation" << p << "_arr);" << std::endl
						<< "  TGraph *sigmaGraph" << p << " = new TGraph(sigma" << p << ".size(), generation_arr, sigma" << p << "_arr);" << std::endl
						<< "  TGraph *minSigmaGraph" << p << " = new TGraph(minSigma" << p << ".size(), generation_arr, minSigma" << p << "_arr);" << std::endl
						<< "  TGraph *maxSigmaGraph" << p << " = new TGraph(maxSigma" << p << ".size(), generation_arr, maxSigma" << p << "_arr);" << std::endl
						<< std::endl;
			}

			summary_ << "  // Do the actual drawing" << std::endl;

			for(std::size_t p=0; p<nInfoIndividuals_; p++) {
				summary_ << "  cc->cd(1);" << std::endl
						<< "  evGraph" << p << "->Draw(\"AP\");" << std::endl
						<< "  cc->cd(2);" << std::endl
						<< "  sigmaGraph" << p << "->Draw(\"AP\");" << std::endl
						<< "  cc->cd(3);" << std::endl
						<< "  minSigmaGraph" << p << "->Draw(\"AP\");" << std::endl
						<< "  cc->cd(4);" << std::endl
						<< "  maxSigmaGraph" << p << "->Draw(\"AP\");" << std::endl
						<< "  cc->cd();" << std::endl
						<< std::endl
						<< "  // Saving the result to file" << std::endl
						<< "  cc->Print(\"individual" << p << ".pdf\");" << std::endl;
			}

			summary_ << "}" << std::endl;
		}
		break;

		//---------------------------------------------------------------------------
		default:
			break;
		};
	}

	/*********************************************************************************************/

private:
	optimizationMonitor(); ///< Intentionally left undefined

	std::size_t nInfoIndividuals_; ///< The number of individuals for which information should be gathered
	std::ostream& summary_; ///< The stream to which information is written
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
