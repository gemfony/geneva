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

# builds the geneva run-time image by copying the prebuild library into the image
build-geneva-run-time:
	docker build -f ./run-time-image/Dockerfile -t geneva-run-time ../

# runs the geneva runtime image waiting in an idle state
run-geneva:
	docker run -d --name geneva_run_time --entrypoint sleep geneva-run-time infinity
	
# runs the geneva run-time image and opens an interactive shell session inside of the container
run-geneva-interactive: run-geneva
	docker exec -it geneva_run_time /bin/bash
	make stop-geneva

# stops and removes a running instance of the geneva_run_time container
stop-geneva:
	docker stop geneva_run_time
	docker rm geneva_run_time