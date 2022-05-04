#
# This file allows you to build and run the geneva docker images
# without having to memorize all docker related commands
#

# runs geneva rebuilding all images
run-geneva-fresh-build: rebuild-geneva
	run-geneva

# rebuilds all images
rebuild-geneva: build-prerequisites run-geneva-builder build-geneva-run-time

# builds the geneva-prerequisites image, which is the base image of the other images
build-prerequisites:
	docker build -f GenevaPrerequisites.Dockerfile -t geneva-prerequisites .

# builds geneva into a shared volume
run-geneva-builder: 
	docker-compose -f ./build-image/docker-compose.yml up --build

# builds the geneva runtime image by copying the prebuild library into the image
build-geneva-run-time:
	docker build -f ./run-time-image/Dockerfile -t geneva-run-time .

# runs the geneva runtime image waiting for you to get an interactive shell session and do what you like
run-geneva:
	docker run geneva-run-time --name geneva_run_time