/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Boost header files go here

// Geneva headers go here
#include "courtier/GCommandContainerT.hpp"
#include "courtier/GCourtierHelperFunctions.hpp"
#include "geneva/genome/GGenome.hpp"

/******************************************************************************/
/**
 * The command container instantiated for GGenome, the work-item
 * payload type carried over the wire by the networked consumers / clients. The matching
 * BOOST_CLASS_EXPORT_IMPLEMENT lives in GGenomeCommandContainerExport.cpp (one translation unit
 * in the geneva library). Including this header makes the registration visible at every networked
 * (de)serialization site -- it is pulled in via GenevaInitializer.hpp.
 */

/******************************************************************************/
