####################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) com
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
# http://www.gemfony.com .
#
####################################################################
# This script will use the ssh program to execute a given command 
# on all hosts specified in a file. Note that it may be useful to use 
# ssh-agent or to avoid specifying a passphrase during key generation 
# in order not having to enter the passphrase for each ssh call. You 
# can call this script like this:
# ./sshList.sh -p 24 -l myUserName ./myHostFile.hcfg myCommand
# The host file should list one name of a remote host per line. Its
# extension MUST be ".hcfg". "myCommand" may itself be a command with 
# arguments. The script will stop execution when one of the ssh calls 
# fails.
####################################################################
# !/bin/bash
#

# Locate the host file in the command line arguments
parmcnt=1
hostFilePos=0
hostFileName=""
hostFileFound=0
for copt in $@; do
    if [ "x${copt#*.}" = "xhcfg" ]; then
		hostFilePos=${parmcnt}
		hostFileName=${copt}
		hostFileFound=1
		break
    fi

    let parmcnt=${parmcnt}+1
done

if [ ${hostFileFound} -eq 0 ]; then
    echo "Error: You need to speficy the name of a file with host names"
    echo "ending in .hcfg . Leaving now ..."
    exit
fi

# Check that the number of command line options is at least two 
# (the name of the hosts file and the command to be executed remotely).
if [ $# -lt 2 ]; then
	echo "Usage: ./listRunCommand.sh <optional ssh options> <hostFile.hcfg> <command>"
	exit
fi

# Check that the hosts file exists
if [ ! -e ${hostFileName} ]; then
	echo "Error: File ${hostFileName} does not exist. Leaving now"
	exit
fi

# Read the host names into a list
hostcnt=0
while read -r hostname; do
  	HOSTS[${hostcnt}]=${hostname}
	let hostcnt=${hostcnt}+1
done < ${hostFileName}

# Assemble a "raw" command string with a place holder for the host
optcnt=1
RAWCOMMANDSTRING=""
for copt in $@; do
    if [ ${optcnt} -ne ${hostFilePos} ]; then
    	RAWCOMMANDSTRING="${RAWCOMMANDSTRING} ${copt}"
    else
    	RAWCOMMANDSTRING="${RAWCOMMANDSTRING} HNPLACEHOLDER"
    fi
    
	let optcnt=${optcnt}+1
done

# Execute the command for each host on the list
for host in "${HOSTS[@]}"; do
	COMMANDSTRING=`echo ${RAWCOMMANDSTRING} | sed s/HNPLACEHOLDER/${host}/g`
	echo "> ssh ${COMMANDSTRING}"
	ssh ${COMMANDSTRING}
	if [ $? -ne 0 ]; then
		echo "Error: Got bad return code from ssh call. Leaving ..."
		exit
	fi
done
