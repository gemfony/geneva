FROM geneva-prerequisites

WORKDIR /home/app/

# make directory to map the library to, which is stored on the host
RUN mkdir -p geneva

COPY ./build-geneva.sh .
COPY ./build-geneva-interactive.sh .

# This directory should be mounted.
# We must specify the mounting point of the host when starting the container
VOLUME [ "/home/app/geneva" ]

# set the working directory to the build directory.
# in case of an interactive run this makes it easier to run the compiled examples and benchmarks
WORKDIR /home/app/geneva/docker/docker-geneva-build/

CMD ["/home/app/build-geneva.sh"]