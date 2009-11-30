/**
 * @file intcoord.hpp
 */

/*
 * Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association); Based on earlier work by Dr. Ivan Kondov.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the eminim2 program, a demo application in the
 * context of Gemfony scientific's optimization library.
 *
 * eminim2 is free software: you can redistribute and/or modify this file
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * eminim2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


// OpenBabel includes
#include <openbabel/babelconfig.h>
#include <openbabel/base.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>
#include <openbabel/internalcoord.h>

#include <vector>

#ifndef INTCOORD_HPP_
#define INTCOORD_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

bool GetVectorOfDihedrals (OpenBabel::OBMol*, std::vector<double>&);
bool SetVectorOfDihedrals (OpenBabel::OBMol*, const std::vector<double>&);

#endif /* INTCOORD_HPP_ */
