#!/bin/bash

################################################################################
#
# This file is part of the Geneva library collection. The following license
# applies to this file:
#
# ------------------------------------------------------------------------------
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------
#
# Note that other files in the Geneva library collection may use a different
# license. Please see the licensing information in each file.
#
################################################################################
#
# Geneva was started by Dr. Rüdiger Berlich and was later maintained together
# with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
# information on Gemfony scientific, see http://www.gemfomy.eu .
#
# The majority of files in Geneva was released under the Apache license v2.0
# in February 2020.
#
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################
# This script will start a given Geneva program a predefined
# number of times in client mode on the same host. It will also
# start the server, using Geneva's standard syntax for networked mode.
# This can be used to test networked execution of a Geneva program
# using local (i.e. reliable, low latency) networking.
# Note that if your program opens many connections to the server,
# e.g. because it uses a large number of individuals and cycles,
# you might quickly run out of local ports (max 64k). See the
# Geneva FAQ for an advice in this situation.
####################################################################

# Check the number of command line arguments (should be exactly 3)
if [ ! $# -eq 3 ]; then
    echo "Usage: ./startLocalJobs.sh <program name> <number of clients> <port>"
    exit 1
fi

# Read in the command line arguments
PROGNAME=$1
NCLIENTS=$2
PORT=$3

# Check that the program exists
if [ ! -e ${PROGNAME} ]; then
    echo "Error: Program file ${PROGNAME} does not exist."
    exit
fi

# Check that the desired number of clients is integral and >= 0
if [ ! $(echo "${NCLIENTS}" | grep -E "^[0-9]+$") ]; then
    echo "Error: Number of clients Number of clients \"${NCLIENTS}\" is not a valid integer. Leaving."
    exit
fi
if [ ! ${NCLIENTS} -gt 0 ];     then
    echo "Error: \"${NCLIENTS}\" should at least be 1. Leaving"
    exit
fi

# Check that the port number is integral and >= 1000
if [ ! $(echo "${PORT}" | grep -E "^[0-9]+$") ]; then
    echo "Error: Port \"${PORT}\" is not a valid integer. Leaving."
    exit
fi
if [ ${PORT} -le 1000 ];     then
    echo "Error: Port \"${PORT}\" should at least be 1001. Leaving"
    exit
fi

# Create an output directory
if [ ! -d ./output ]; then
    mkdir ./output
fi

# Start the server
(./$1 -c asio --asio_port=${PORT} >& ./output/output_server) &

# Start the workers
for i in `seq 1 $2`; do
    (./${PROGNAME} -c asio --client --asio_ip=localhost --asio_port=${PORT} >& ./output/output_client_$i) &
done

tail -f ./output/output_server
