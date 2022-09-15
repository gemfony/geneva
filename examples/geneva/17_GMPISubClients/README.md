# MPI Sub-Clients

This example shows how to use the GMPIConsumerT with MPI sub-clients. The MPI-Consumer distributes the individuals via MPI to clients/workers.
Additionally, in this example each of those clients talks to a number of clients while calling the calculateFitness function
of the individual. So we have two layers of MPI.

To make this work we have to initialize MPI ourselves and create inter-communicators. This way the Geneva-clients can talk
to the Geneva server using one communicator and the clients can talk to the sub-clients using another communicator.

To run this example with 1 server, 4 clients and 3 sub-clients for each of the clients use:

``````
    mpirun --oversubscribe -np 17 ./GMPISubClients --consumer mpi 
``````

NOTE: if you want to change the amount of sub-clients you have to change the constant DEFAULT_N_SUB_CLIENTS
in the GMPISubClients.cpp file.
