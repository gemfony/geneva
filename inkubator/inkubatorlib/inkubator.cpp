/**
 * @file oainkubator.cpp
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

#include "inkubator.hpp"

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The solver-constructor
 */
solver::solver(const solverFunction& f)
	:f_(f)
{ /* nothing */ }

/******************************************************************************/
/**
 * The solver copy constructor
 */
solver::solver(const solver& cp)
	:f_(cp.f_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The solver destructor
 */
solver::~solver()
{ /* nothing */ }


/******************************************************************************/
/**
 * This is just syntactic sugar ...
 */
double solver::operator()(const std::vector<double>& vec) const {
	return fitnessCalculation(vec);
}

/******************************************************************************/
/**
 * Trigger the actual fitness calculation
 */
double solver::fitnessCalculation(const std::vector<double> vec) const {
	double result = 0.;

	switch(f_) {
		case PARABOLA:
			result = fitnessParabola(vec);
			break;

		case NOISYPARABOLA:
			result = fitnessNoisyParabola(vec);
			break;

		case ROSENBROCK:
			result = fitnessRosenbrock(vec);
			break;

		case ACKLEY:
			result = fitnessAckley(vec);
			break;

		case RASTRIGIN:
			result = fitnessRastrigin(vec);
			break;

		case SCHWEFEL:
			result = fitnessSchwefel(vec);
			break;

		case SALOMON:
			result = fitnessSalomon(vec);
			break;
	}

	return result;
}

/******************************************************************************/
/**
 * A simple parabola
 */
double solver::fitnessParabola(const std::vector<double>& vec) const {
	double result = 0;

	for(auto d: vec) {
		result += pow(d,2.);
	}

	return result;
}

/******************************************************************************/
/**
 * A parabola with many overlaid local optima
 */
double solver::fitnessNoisyParabola(const std::vector<double>& vec) const {
	double result = 0;

	// Calculate result --> You can get all solver function definitions from here:
	// http://bazaar.launchpad.net/~gemfony/geneva/trunk/view/head:/src/geneva-individuals/GFunctionIndividual.cpp
	// For the purpose of this demo, you can copy them verbatim (look at line 871 ff.). The macro "GSQUARED(x)" can
	// be replaced with pow(x,2.)

	double xsquared = 0.;
	for(auto d: vec) {
		xsquared += pow(d,2.);
	}
	result = (cos(xsquared) + 2.) * xsquared;
	return result;
}

/******************************************************************************/
/**
 * The Rosenbrock function
 */
double solver::fitnessRosenbrock(const std::vector<double>& vec) const {
	double result = 0;

	for(std::size_t i=0; i<(vec.size()-1); i++) {
		result += 100.*pow((pow(vec.at(i),2.) - vec.at(i+1)),2.) + pow(1.-vec.at(i),2.);
	}
	return result;
}

/******************************************************************************/
/**
 * The Ackley function
 */
double solver::fitnessAckley(const std::vector<double>& vec) const {
	double result = 0;
	for(std::size_t i=0; i<(vec.size()-1); i++) {
		result += (exp(-0.2)*sqrt(pow(vec.at(i),2.) + pow(vec.at(i+1),2.)) + 3.*(cos(2.*vec.at(i)) + sin(2.*vec.at(i+1))));
	}
	return result;
}

/******************************************************************************/
/**
 * The Rastrigin function
 */
double solver::fitnessRastrigin(const std::vector<double>& vec) const {
	double result = 10*double(vec.size());

	for(auto d: vec) {
		result += (pow(d,2.) - 10.*cos(2*M_PI*d));
	}

	return result;
}

/******************************************************************************/
/**
 * The Schwefel function
 */
double solver::fitnessSchwefel(const std::vector<double>& vec) const {
	double result = 0;

	for(auto d: vec) {
		result += -d*sin(sqrt(fabs(d)));
	}

	result /= vec.size();

	return result;
}

/******************************************************************************/
/**
 * The Salomon function
 */
double solver::fitnessSalomon(const std::vector<double>& vec) const {
	double result = 0;

	double sum_root = 0.;
	for(auto d: vec) {
		sum_root += pow(d,2.);
	}

	sum_root=sqrt(sum_root);
	result = -cos(2*M_PI*sum_root) + 0.1*sum_root + 1;

	return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor. Note: Some parameters are initialized in the
 * class body.
 */
optimizerBase::optimizerBase(
	const std::vector<double>& startValues
	, const solver& s
	, const std::size_t& maxIterations
)
	: bestParameters_(startValues)
	, solver_(s)
	, maxIterations_(maxIterations)
{ /* nothing */ }


/******************************************************************************/
/**
 * The copy constructor
 */
optimizerBase::optimizerBase(const optimizerBase& cp)
	: bestEvaluation_(cp.bestEvaluation_)
	, bestParameters_(cp.bestParameters_)
	, solver_(cp.solver_)
	, maxIterations_(cp.maxIterations_)
	, currentIteration_(0)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
optimizerBase::~optimizerBase()
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieve the current iteration
 */
std::size_t optimizerBase::getCurrentIteration() const {
	return currentIteration_;
}

/******************************************************************************/
/**
 * Retrieve the best result found so far
 */
double optimizerBase::getBestResult() const {
	return bestEvaluation_;
}

/******************************************************************************/
/**
 * Retrieve the best parameters  found so far
 */
std::vector<double> optimizerBase::getBestParameters() const {
	return bestParameters_;
}

/******************************************************************************/
/**
 * The external optimizer interface.
 * Will reset the currentIteration_ variable when called
 * Returns a vector of best solutions.
 */
std::vector<double> optimizerBase::optimize() {
	currentIteration_ = 0;
	double currentEvaluation = std::numeric_limits<double>::max();
	std::vector<double> currentBestParameters(bestParameters_.size());

	// Initialization code
	init();

	// Optimize, until a halt criterion is reached
	do {
		// This is where the actual work is done
		currentEvaluation = cycleLogic(currentBestParameters);

		// Update the best results
		if(currentEvaluation <= bestEvaluation_) {
			bestEvaluation_ = currentEvaluation;
			bestParameters_ = currentBestParameters;
		}

		// Emit progress information
		std::cout << this->getCurrentIteration() << ": " << this->getBestResult() << std::endl;
	} while(!this->halt());

	// Finalization code
	finalize();

	// Let the audience know
	return this->getBestParameters();
}

/******************************************************************************/
/**
 * Overload this function in derived classes, if initialization work is required
 */
void optimizerBase::init()
{ /* nothing */ };

/******************************************************************************/
/**
 * Overload this function in derived classes, if initialization work is required
 */
void optimizerBase::finalize()
{ /* nothing */ };

/******************************************************************************/
/**
 * Return "true", if currentIteration_ becomes larger than maxIterations_, otherwise "false"
 */
bool optimizerBase::halt()
{ return (currentIteration_++ >= maxIterations_); }

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The standard constructor
 */
optimizerPlaceHolder::optimizerPlaceHolder(
	const std::vector<double>& startValues
	, const solver& s
	, const std::size_t& maxIterations
)
	: optimizerBase(startValues, s, maxIterations) // Initialize our parent class
{ /* nothing */ }

/******************************************************************************/
/**
 * The optimization logic. We do nothing in this dummy optimizer. You may implement
 * your own optimization code here.
 */
double optimizerPlaceHolder::cycleLogic(std::vector<double>& bestParameters) {
	bestParameters = bestParameters_;
	return bestEvaluation_;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Helper function to output results and keep "main()" clean
 */
void print(
	const std::vector<double>& vec
	, const std::string& envelope
) {
	std::cout << envelope << std::endl;
	std::vector<double>::const_iterator cit;
	for(cit=vec.begin(); cit!=vec.end(); ++cit) {
		std::cout << *cit << " ";
	}
	std::cout << std::endl;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
