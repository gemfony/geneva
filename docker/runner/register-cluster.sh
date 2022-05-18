#!/bin/bash

#
# This file is taken from https://github.com/giovtorres/slurm-docker-cluster.
# It might have been adjusted to suit the geneva project
#

set -e

docker exec slurmctld bash -c "/usr/bin/sacctmgr --immediate add cluster name=linux" && \
docker-compose restart slurmdbd slurmctld
