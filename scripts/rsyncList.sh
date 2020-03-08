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
