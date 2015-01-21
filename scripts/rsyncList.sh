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
# This script will sync a local directory with another directory 
# on a list of remote hosts specified in a file. Note that it may be 
# useful to use ssh-agent or to avoid specifying a passphrase during 
# key generation in order not having to enter the passphrase for each 
# ssh call. You can call this script like this:
# ./rsyncList.sh -av -e ssh ./ myHostFile.hcfg:/home/user/somedir --delete-after
# Or in other words: Instead of specifying a remote host name, you
# provide the script with the name of a file holding host names.
# The host file should list one name of a remote host per line. Its
# extension MUST be ".hcfg". The script will stop execution when one of 
# the rsync calls fails.
####################################################################

# Check that the number of command line options is at least two 
if [ $# -lt 2 ]; then
	echo "Usage: ./rsyncList.sh <options> </local/dir> <hostFile.hcfg:/remote/dir> <options>"
	exit
fi

# Locate the host file in the command line arguments
parmcnt=1
hostFilePos=0
hostFileNameWithTargetDir=""
hostFileFound=0
for copt in $@; do
    if [ `echo ${copt} | grep ".hcfg"` ]; then
		hostFilePos=${parmcnt}
		hostFileNameWithTargetDir=${copt}
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

# Extract the name of the host file
hostFileName=`echo ${hostFileNameWithTargetDir} | awk -F: '{print $1}'`

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

# Extract the target directory
targetDir=`echo ${hostFileNameWithTargetDir} | awk -F: '{print $2}'`

# Assemble a "raw" command string with a place holder for the host
optcnt=1
RAWCOMMANDSTRING=""
for copt in $@; do
    if [ ${optcnt} -ne ${hostFilePos} ]; then
    	RAWCOMMANDSTRING="${RAWCOMMANDSTRING} ${copt}"
    else
    	RAWCOMMANDSTRING="${RAWCOMMANDSTRING} HNPLACEHOLDER:${targetDir}"
    fi
    
	let optcnt=${optcnt}+1
done

# Execute the command for each host on the list
for host in "${HOSTS[@]}"; do
	COMMANDSTRING=`echo ${RAWCOMMANDSTRING} | sed s/HNPLACEHOLDER/${host}/g`
	echo "> rsync ${COMMANDSTRING}"
	rsync ${COMMANDSTRING}
	RETCODE=$?
	if [ ${RETCODE} -ne 0 ]; then
		echo "Error: Got bad return code ${RETCODE} from ssh call. Leaving ..."
		exit
	fi
done
