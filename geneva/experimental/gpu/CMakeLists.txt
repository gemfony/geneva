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
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################

# Demo programs for the unified, device-model-agnostic GPU consumer framework. The framework itself
# (CPU/CUDA/OpenCL backends, runtime kernel loading) now lives in the Courtier library and is built as
# the optional gemfony-courtier-gpu add-on (see courtier/src/gpu/). These demos just drive it.
#
# NOTE: this experimental tree is being retired. The Mona-Lisa demo has been promoted to the official
# example 15, and the CUDA optimisation benchmark reuses the same framework. The remaining demos here
# are kept only as additional smoke tests until the migration is complete.

ADD_SUBDIRECTORY ( examples/function_demo )
ADD_SUBDIRECTORY ( examples/image_demo )
ADD_SUBDIRECTORY ( examples/monalisa_demo )
