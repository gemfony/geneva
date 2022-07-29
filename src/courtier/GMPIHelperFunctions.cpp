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

#include "courtier/GMPIHelperFunctions.hpp"

/******************************************************************************/
/**
 * Returns the message size of a completed request.
 * The return value is undefined for the status on not yet completed requests.
 * Therefore you should call MPI_Test first.
 *
 * @param status a reference to an MPI status object
 * @param dataType the datatype of the MPI message. Defaults to MPI_CHAR
 * @return the amount of objects of type dataType transferred by the operation
 */
int mpiGetCount(const MPI_Status &status, MPI_Datatype dataType) {
    int count{};
    MPI_Get_count(&status, dataType, &count);

    return count;
}

/**
 * returns the number of processes in a given MPI Communicator
 * @param comm the communicator
 * @return the number of processes in the communicator
 */
std::uint32_t mpiSize(const MPI_Comm &comm) {
    int size{};
    MPI_Comm_size(comm, &size);

    return size;
}

/**
 * Converts an MPI error code to the corresponding string which explains the error
 *
 * @param mpiError integer error code used by MPI and typically stored in an MPI_Status.status field
 * @return a string corresponding to the error code
 */
std::string mpiErrorString(int mpiError) {
    char errorMessage[MPI_MAX_ERROR_STRING];
    int messageLength;
    MPI_Error_string(mpiError, errorMessage, &messageLength);

    // the string in errorMessage is null terminated, which allows us to pass it to the std::string constructor
    return std::string{errorMessage};
}

/**
 * Waits for the completion of an async MPI request by checking in a cyclic manner as long as a predicate returns true
 * @param request the request to wait on
 * @param pollIntervalMSec the time between checks
 * @param runWhile predicate, when this is false the waiting will be aborted
 * @return status of the operation when terminated
 */
MPICompletionStatus waitForRequestCompletionWhile(MPI_Request &request,
                                                  const std::uint64_t &pollIntervalMSec,
                                                  const std::function<bool()> &runWhile) {
    int isCompleted{0};
    MPI_Status status{};

    while (runWhile()) {
        MPI_Test(&request,
                 &isCompleted,
                 &status);


        // return appropriate result in case of completion
        if (isCompleted) {
            if (status.MPI_ERROR == MPI_SUCCESS) {
                return MPICompletionStatus{MPIStatusCode::SUCCESS, status};
            } else {
                return MPICompletionStatus{MPIStatusCode::ERROR, status};
            }
        }

        // sleep some time before polling again for completion status
        std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMSec));
    }

    // the execution shall be stopped due to the stop criterion returning false

    // return appropriate result
    return MPICompletionStatus{MPIStatusCode::STOPPED, status};
}
/**
 * Performs an async scatter and bocks until the request has completed or a predicate returns false
 */
MPICompletionStatus mpiScatterWhile(const void *sendBuf,
                                    const std::uint32_t &sendCount,
                                    void *recvBuf,
                                    MPI_Datatype type,
                                    const std::function<bool()> &runWhile,
                                    const std::uint32_t &root = 0,
                                    MPI_Comm comm = MPI_COMM_WORLD,
                                    const std::uint64_t &pollIntervalMSec = 1) {
    MPI_Request requestHandle{};

    MPI_Iscatter(
            sendBuf, // send substrings of the test message
            static_cast<int>(sendCount), // send one char to each other process
            type,
            recvBuf, // receive one character as the root process
            static_cast<int>(sendCount), // send one character to every other process
            type,
            static_cast<int>(root), // rank 0 (this process) is the root.
            comm,
            &requestHandle);

    return waitForRequestCompletionWhile(requestHandle, pollIntervalMSec, runWhile);
}

/**
 * Performs an async gather and bocks until the request has completed or a predicate returns false
 */
MPICompletionStatus mpiGatherWhile(const void *sendBuf,
                                   const std::uint32_t &sendCount,
                                   void *recvBuf,
                                   MPI_Datatype type,
                                   const std::function<bool()> &runWhile,
                                   const std::uint32_t &root = 0,
                                   MPI_Comm comm = MPI_COMM_WORLD,
                                   const std::uint64_t &pollIntervalMSec = 1) {
    MPI_Request requestHandle{};

    MPI_Igather(
            sendBuf, // send substrings of the test message
            static_cast<int>(sendCount), // send one char to each other process
            type,
            recvBuf, // receive one character as the root process
            static_cast<int>(sendCount), // send one character to every other process
            type,
            static_cast<int>(root), // rank 0 (this process) is the root.
            comm,
            &requestHandle);

    return waitForRequestCompletionWhile(requestHandle, pollIntervalMSec, runWhile);
}

