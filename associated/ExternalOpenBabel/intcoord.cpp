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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "intcoord.hpp"

using namespace OpenBabel;

bool GetVectorOfDihedrals (OpenBabel::OBMol *mol, std::vector<double>& vod) {
  // define a vector of internal coordinates
  if (!mol) return false;
  std::vector<OBInternalCoord*> vic = mol->GetInternalCoord();
  int natoms=mol->NumAtoms();
  if (natoms < 4) {
    std::cerr << "In GetVectorOfDihedrals(): Error: No dihedral angles for less than 4 natoms.\n";
    return false;
  }

  for (unsigned i = 4; i<vic.size(); i++) vod.push_back(vic[i]->_tor);
  return true;
}

bool SetVectorOfDihedrals (OpenBabel::OBMol *mol, const std::vector<double>& vod) {
  // define a vector of internal coordinates
  if (!mol) {
    std::cerr << "In SetVectorOfDihedrals(): Error: Bad molecule given" << std::endl;
    return false;
  }
  int natoms=mol->NumAtoms();
  if(vod.size() != natoms-3) {
    std::cerr << "In SetVectorOfDihedrals(): Error: invalid number of dihedrals: " << vod.size() << " " << natoms << " " << natoms-3 << std::endl;
    return false;
  }

  std::vector<OBInternalCoord*> vic = mol->GetInternalCoord();

  for (unsigned i = 4, k=0; i < vic.size(); i++, k++) {
    vic[i]->_tor=vod[k];
  }
  InternalToCartesian(vic, *mol);
  return true;
}
