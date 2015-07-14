/**
 * @file incubator.hpp
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

#include <iostream>
#include <vector>
#include <cmath>
#include <limits>

#ifndef OAINKUBATOR_HPP_
#define OAINKUBATOR_HPP_

/******************************************************************************/
/**
 * This enum denotes the possible demo function types
 */
enum solverFunction {
	PARABOLA=0
	, NOISYPARABOLA=1
	, ROSENBROCK=2
	, ACKLEY=3
	, RASTRIGIN=4
	, SCHWEFEL=5
	, SALOMON=6
};

/******************************************************************************/
/**
 * Calculation of the fitness of a given solution
 */
class solver {
public:
	// The standard constructor
	solver(const solverFunction&);
	// The copy constructor
	solver(const solver&);

	// virtual destructor, so we can later derive classes from this one
	virtual ~solver();

	// This is just syntactic sugar ...
	double operator()(const std::vector<double>&) const;

	// Trigger the actual fitness calculation
	double fitnessCalculation(const std::vector<double>) const;

protected:
	double fitnessParabola(const std::vector<double>& vec) const;
	double fitnessNoisyParabola(const std::vector<double>& vec) const;
	double fitnessRosenbrock(const std::vector<double>& vec) const;
	double fitnessAckley(const std::vector<double>& vec) const;
	double fitnessRastrigin(const std::vector<double>& vec) const;
	double fitnessSchwefel(const std::vector<double>& vec) const;
	double fitnessSalomon(const std::vector<double>& vec) const;

	solverFunction f_; ///< The chosen solver

private:
	// The default constructor: intentionally private and undefined, as we want to enforce usage of solver(const solverFunction& f)
	solver() = delete;
};

/******************************************************************************/
/**
 * The optimizer base class. Note that this function assumes that your
 * optimization algorithm MINIMIZES only.
 */
class optimizerBase {
public:
	// The default constructor
	optimizerBase(
		const std::vector<double>&
		, const solver&
		, const std::size_t&
	);
	// The copy constructor
	optimizerBase(const optimizerBase&);
	// virtual destructor
	virtual ~optimizerBase();

	// Retrieve the current iteration
	std::size_t getCurrentIteration() const;
	// Retrieve the best result found so far
	double getBestResult() const;
	// Retrieve the best parameters  found so far
	std::vector<double> getBestParameters() const;

	// The external optimizer interface.
	std::vector<double> optimize();

protected:
	// Overload this function in derived classes, if initialization work is required
	virtual void init();
	// Overload this function in derived classes, if initialization work is required
	virtual void finalize();
	// Overload this function in derived classes -- it holds the logic to be executed in each iteration
	virtual double cycleLogic(std::vector<double>&) = 0;

	// Return "true", if currentIteration_ becomes larger than maxIterations_, otherwise "false"
	bool halt();

	double bestEvaluation_ = std::numeric_limits<double>::max(); ///< Holds the best value found so far
	std::vector<double> bestParameters_; ///< Holds the currently best parameter set

	solver solver_; ///< Holds the solver object

private:
	std::size_t maxIterations_; ///< Holds the maximum number of optimization cycles -- this will serve as our stop criterion for the optimization
	std::size_t currentIteration_ = 0; ///< The current iteration being processed

	// The default constructor: intentionally private and undefined, as we want to enforce usage of optimizerBase(const std::size_t&)
	optimizerBase() = delete;
};

/******************************************************************************/
/**
 * A place holder for optimization algorithms to be tried out
 */
class optimizerPlaceHolder
	: public optimizerBase
{
public:
	optimizerPlaceHolder(
		const std::vector<double>&
		, const solver&
		, const std::size_t&
	);

protected:
	// The optimization logic. We do nothing in this dummy optimizer. You may implement
	// your own optimization code here.
	virtual double cycleLogic(std::vector<double>&);

private:
	// The default constructor: intentionally private and undefined, as we want to enforce usage of optimizerBase(const std::size_t&)
	optimizerPlaceHolder() = delete;
};

/******************************************************************************/
/** @brief Helper function to output results and keep "main()" clean */
void print(const std::vector<double>&, const std::string&);

/******************************************************************************/

#endif /* OAINKUBATOR_HPP_ */
