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

# How to launch the MPI cluster

1. Configure your host file
Both worker and manager should have the same /etc/hosts file
```

$ cat /etc/hosts

127.0.0.1       localhost
172.50.88.34    worker
```
2. Create a new user

Both worker and manager should have this user
```
sudo adduser mpiuser
```

3. Setting up SSH

Everything here is applied for both worker & manager

```
sudo apt-get install openssh-server
```

```
su - mpiuser
```

```
ssh-keygen -t rsa
```

```
ssh-copy-id worker
```
```
ssh-copy-id localhost
```

To enable passwordless
```
eval `ssh-agent`
```

```
ssh-add ~/.ssh/id_rsa
```
Checkout it is working
```
ssh worker
```
4. Setting up NFS

# NFS Manager
```
sudo apt-get install nfs-kernel-server
```
Under the home of mpiuser create the directory
```
mkdir cloud
```

Inside /etc/exports copy **/home/mpiuser/cloud *(rw,sync,no_root_squash,no_subtree_check)**
```
cat /etc/exports
```

```
exportfs -a
```

# NFS worker

```
sudo apt-get install nfs-common
```
```
mkdir cloud
```
```
sudo mount -t nfs manager:/home/mpiuser/cloud ~/cloud
```
```
df -h
```
Change /etc/fstab
```
$ cat /etc/fstab
#MPI CLUSTER SETUP
manager:/home/mpiuser/cloud /home/mpiuser/cloud nfs
```

5. Running MPI Programms

Run it
```
mpirun -np 3 -host worker:1,localhost:2 ./znewsim
```

# TODO

- System testing
- Automatize the environment
