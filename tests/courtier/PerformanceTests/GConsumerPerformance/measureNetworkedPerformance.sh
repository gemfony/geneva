#!/bin/bash

####################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) eu
#
# This file is part of the Geneva library collection.
#
# Geneva was developed with kind support from Karlsruhe Institute of
# Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
# information about KIT and SCC can be found at http://www.kit.edu/english
# and http://scc.kit.edu .
#
# Geneva is free software: you can redistribute and/or modify it under
# the terms of version 3 of the GNU Affero General Public License
# as published by the Free Software Foundation.
#
# Geneva is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
#
# For further information on Gemfony scientific and Geneva, visit
# http://www.gemfony.eu .
#
####################################################################
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
if [ ! $# -eq 2 ]; then
    echo "Usage: ./measureNetworkedPerformance.sh <number of clients> <port>"
    exit 1
fi

# Read in the command line arguments
PROGNAME=./GConsumerPerformance
NCLIENTS=$1
PORT=$2

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
(${PROGNAME} -s -e 7 --port=${PORT} >& ./output/output_server) &

# Start the workers
for i in `seq 1 ${NCLIENTS}`; do
    (${PROGNAME} -e 7 --ip=localhost --port=${PORT} >& ./output/output_client_$i) &
done

tail -f ./output/output_server
