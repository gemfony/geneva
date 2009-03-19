/**
 * @file intcoord.hpp
 */

/*
 * Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * Based on earlier work by Dr. Ivan Kondov
 *
 * This file is part of the eminim2 program. eminim2 is free software:
 * you can redistribute and/or modify this file under the terms of
 * version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * eminim2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */


// OpenBabel includes
#include <openbabel/babelconfig.h>
#include <openbabel/base.h>
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>
#include <openbabel/forcefield.h>
#include <openbabel/internalcoord.h>

#include <vector>

bool GetVectorOfDihedrals (OpenBabel::OBMol*, std::vector<double>&);
bool SetVectorOfDihedrals (OpenBabel::OBMol*, const std::vector<double>&);
