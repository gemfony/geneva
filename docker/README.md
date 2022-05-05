# Geneva docker and docker-compose setup

This is a docker and docker-compose setup that allows you to build an run geneva. That means that you just need to have `docker` and `docker-compose` installed. All other prerequisites of Geneva e.g. being on a Linux system, having boost, gcc, doxygen and mpi installed are not required when using the docker setup.

## What this setup is useful for

* Compiling and running geneva if your local machine is not running linux
* Compiling and running geneva if you do not have the prerequisites like boost, gcc, mpi or doxygen installed
* Building an independent geneva image that can be shipped and run by other people without having to install anything but docker
* Testing your slurm jobs using geneva in a local slurm cluster started with docker-compose

## What this setup might not be suitable for

For the following use cases you might rather install geneva natively:

* Debugging
* Running in production, because the containerization of course comes with a performance overhead

## Using the docker setup

Refer to the Makefile in this directory for some useful commands to build the images and run the containers.  

The most important command for users might be `make build-geneva-image` which builds the `geneva_runner` image, which can be shipped and run by anyone who has docker installed.  

NOTE: The first time building the images may take over an hour due to building the prerequisites-image. However all of your subsequent builds should be just as fast as if you build geneva natively on your machine.

## Structure of the docker setup

This setup contains multiple components:

* The `geneva-prerequisites` image. This will is the required base image for the other images. Building this image can take over an hour due to compiling boost and gcc, but you only have to compile this the first time you work with the setup, because this image never changes.
* The `geneva-builder` image builds the geneva library into a shared volume located at directory `docker-geneva-build/`. If you are a developer you can also run this container in interactive mode with `make run-geneva-builder-interactive` and e.g. execute the examples in this container. The advantage of this is that cmake can reuse its cache from previous builds, because this is stored in the volume persistently on the host. This drastically speeds up builds.
* The `geneva-runner` image copies the pre-compiled-library (compiled after running the geneva-builder image) which the image and installs the library. This image can be shipped and used even if you have no access to the source code or anything else on the target machine. However if you just want to do quick test runs inside of a container you can do this in the geneva-builder container directly and omit this extra building step.
* With `make run-local-slurm-cluster-interactive` you can start a local slurm cluster and start an interactive shell session in the control node. Each node has access to the compiled and installed geneva library.
