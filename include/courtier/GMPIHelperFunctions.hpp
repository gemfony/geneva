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

// MPI library
#include <mpi.h>

// standard headers
#include <chrono>
#include <functional>
#include <string>
#include <thread>

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

int mpiGetCount(const MPI_Status &, MPI_Datatype = MPI_CHAR);

/******************************************************************************/

std::string mpiErrorString(int);

/******************************************************************************/

std::uint32_t mpiSize(const MPI_Comm &comm);

/******************************************************************************/

[[nodiscard]] MPICompletionStatus mpiScatterWhile(
    const void *sendBuf,
    const std::uint32_t &sendCount,
    void *recvBuf,
    MPI_Datatype type,
    const std::function<bool()> &runWhile,
    const std::uint32_t &root,
    MPI_Comm comm,
    const std::uint64_t &pollIntervalMSec
);

/******************************************************************************/

[[nodiscard]] MPICompletionStatus mpiGatherWhile(
    const void *sendBuf,
    const std::uint32_t &sendCount,
    void *recvBuf,
    MPI_Datatype type,
    const std::function<bool()> &runWhile,
    const std::uint32_t &root,
    MPI_Comm comm,
    const std::uint64_t &pollIntervalMSec
);
