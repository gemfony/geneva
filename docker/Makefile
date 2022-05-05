#
# This file allows you to build and run the geneva docker images
# without having to memorize all docker related commands
#

# builds the geneva image, which is independent of any shared volumes
build-geneva-image: run-geneva-builder
	docker build -f ./runner/Dockerfile -t geneva-runner ../

# builds the geneva library into a shared volume located at ./runner/docker-geneva-build
run-geneva-builder: build-prerequisites
	docker-compose -f ./builder/docker-compose.yml up --build

# builds the geneva library into a shared volume and opens an interactive shell session after the build has been completed
# This assumes you have already built the geneva-prerequisites image.
# We do not want force rebuild here, because this target is mostly for developers, 
# who most likely already have build the geneva-prerequisites
run-geneva-builder-interactive:
	docker-compose -f ./builder/docker-compose-interactive.yml up --build --detach
	docker exec -it geneva_builder /bin/bash
	docker-compose -f ./builder/docker-compose-interactive.yml down

# builds the geneva-prerequisites image, which is the base image of the other images
build-prerequisites:
	docker build -f GenevaPrerequisites.Dockerfile -t geneva-prerequisites .


# runs the independent geneva image in interactive mode
# This assumes that the geneva_runner image has been build with make build-geneva-image before
run-geneva-image:
	docker run -d --name geneva_runner --entrypoint sleep geneva-runner infinity
	docker exec -it geneva_runner /bin/bash
	make stop-geneva-image
	docker stop geneva_runner
	docker rm geneva_runner

# stops and removes a running instance of the geneva_runner container
stop-geneva-image:
	docker stop geneva_runner
	docker rm geneva_runner

# starts up the local slurm cluster and opens an interactive shell session in the control node
# This assumes you have built the geneva-runner container with `make build-geneva-image`
# if sinfo shows that you have no nodes connected, then run ./runner/.register-cluster.sh from
# the host machine to register the nodes with the cluster manually
run-local-slurm-cluster-interactive:
	docker-compose -f ./runner/docker-compose-slurm.yml up --build --detach
	docker exec -it slurmctld /bin/bash
	docker-compose -f ./runner/docker-compose-slurm.yml down