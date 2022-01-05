# How to launch the simulation

# Mosquitto installing and configuring
At first you should download and install mosquitto server from this link

```
https://www.arubacloud.com/tutorial/how-to-install-and-secure-mosquitto-on-ubuntu-20-04.aspx
```

You're doing all the steps and test it **untill Configuring MQTT Password section**

When you've checked that the MQTT server is running. 
You should type 

```
sudo nano /etc/mosquitto/conf.d/default.conf
```

And inside this file write the following line

```
allow_anonymous true
listener 1883 localhost
```

Then you should restart the server and make sure that it is running again

```
sudo systemctl restart mosquitto
```

```
sudo systemctl status mosquitto
```

Yes it is not secure, and the code right now works only for insecure solutions.

# Installing the environment

# MPI installation

Install the packages for OpenMPI

```
sudo apt-get install libopenmpi-dev openmpi-bin
```

# Paho MQTT installation

Clone the GitHub repository and install it

```
git clone https://github.com/eclipse/paho.mqtt.c.git
cd org.eclipse.paho.mqtt.c.git
make
sudo make install
```

To check the correct work the publication program:

```
paho_c_pub -t my_topic --connection mqtt.eclipseprojects.io:1883
```

The subscriber (in another terminal)
```
paho_c_sub -t my_topic --connection mqtt.eclipseprojects.io:1883
```

You should see the messages that your publisher published in the second terminal

# UUID installation

This is for generating unique ids

```
sudo apt-get install libuuid1 uuid-dev
```

# To run the program

At first subscirbe at the topic:

```
paho_c_sub -t my_topic_test --connection localhost:1883
```

Then change in makefile this line to your MQTT Paho path

```
LDFLAGS += -L/{...}/MQTT_Paho/paho.mqtt.c/build/output -lpaho-mqtt3a
```

Run the python script

```
python3 ./runner.py
```

Right now it **intentionally** loses packets to simulate the real world situation. To have an incomplete data at the upstream. 

# TODO

- [ ] secure the server and rewrite the connection with TSL (in a more secure way)
- [ ] think about -lpaho-mqtt3a in makefile
- [ ] simulate the formula with the parameters
- [ ] test the environment
