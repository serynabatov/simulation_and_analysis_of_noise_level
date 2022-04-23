# Before launching the simulation

Download the following libraries:
- https://www.open-mpi.org/software/ompi/v4.1/
- https://github.com/edenhill/librdkafka
- https://github.com/DaveGamble/cJSON

Version that were used during testing:
- Open MPI - 4.0.3v
- cJSON - 1.7.15v
- rdKafka - 1.6.2v

# How to launch the simulation

After you have installed the libraries, run

```
make
```

And then

```
mpirun -np 3 ./znewsim
```

# TODO

- System testing
- Automatize the environment
