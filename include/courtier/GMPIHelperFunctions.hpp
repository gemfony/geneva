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
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// MPI library
#include <mpi.h>

// standard headers
#include <string>
#include <chrono>
#include <thread>
#include <functional>

// TODO: maybe create a new namespace for these utilities

enum MPIStatusCode {
    /**
     * The operation has succeeded without errors
     */
    SUCCESS,
    /**
     * The operation was stopped due to the halt criterion
     */
    STOPPED,
    /**
     * The operation has completed with an error
     */
    ERROR
};

/**
 * Stores the result of an MPI while operation
 */
struct MPICompletionStatus {
    /**
     * Type of completion
     */
    MPIStatusCode statusCode{};
    /**
     * The status which was returned by MPI at that time
     */
    MPI_Status mpiStatus{};
};

/******************************************************************************/
G_API_COMMON
int mpiGetCount(const MPI_Status &, MPI_Datatype = MPI_CHAR);

/******************************************************************************/
G_API_COMMON
std::string mpiErrorString(int);

/******************************************************************************/
G_API_COMMON
std::uint32_t mpiSize(const MPI_Comm &comm);

/******************************************************************************/
G_API_COMMON
[[nodiscard]] MPICompletionStatus mpiScatterWhile(const void *sendBuf,
                                                  const std::uint32_t &sendCount,
                                                  void *recvBuf,
                                                  MPI_Datatype type,
                                                  const std::function<bool()> &runWhile,
                                                  const std::uint32_t &root,
                                                  MPI_Comm comm,
                                                  const std::uint64_t &pollIntervalMSec);

/******************************************************************************/
G_API_COMMON
[[nodiscard]] MPICompletionStatus mpiGatherWhile(const void *sendBuf,
                                                 const std::uint32_t &sendCount,
                                                 void *recvBuf,
                                                 MPI_Datatype type,
                                                 const std::function<bool()> &runWhile,
                                                 const std::uint32_t &root,
                                                 MPI_Comm comm,
                                                 const std::uint64_t &pollIntervalMSec);