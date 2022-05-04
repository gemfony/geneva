FROM geneva-prerequisites

WORKDIR /home/app/

# make directory to map the library to, which is stored on the host
RUN mkdir -p geneva

COPY ./docker-entrypoint.sh .

# This directory should be mounted.
# We must specify the mounting point of the host when starting the container
VOLUME [ "/home/app/geneva" ]

CMD ["./docker-entrypoint.sh"]