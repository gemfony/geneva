#!/bin/bash

#
# Builds the geneva library into a shared volume and keeps the container alive afterwards.
# This allows to open an interactive shell session and e.g. run examples
#

/home/app/build-geneva.sh

echo "Build finished. Container will now sleep forever ready for you to open an interactive shell session and do what you like."

sleep infinity