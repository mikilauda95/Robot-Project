# Robot-Project: Code for Marvin
This repository contains code intended to be run on Marvin the robot. The repository is set up to be cross-compiled on Linux (Ubuntu/Debian) and for the final executable to be copied over to the robot using [SCP](http://www.hypexr.org/linux_scp_help.php).

## Getting started with this repository
1. In an empty folder on your computer, clone this repository:
```
$ git clone https://github.com/amundas/Robot-Project.git
```

2. From inside the newly created folder `Robot-Project`, clone the ev3dev repository. This is a library that is needed by our program. 
```
$ cd Robot-Project && git clone https://github.com/in4lio/ev3dev-c
```

3. In order to cross-compile, we need to install the arm cross compiler. Download links are below:
	* https://www.acmesystems.it/arm9_toolchain for Ubuntu-based systems (including Windows Subsystem for Linux)
	* https://packages.debian.org/stretch/gcc-arm-linux-gnueabi for Debian based systems

4. If everything went well, running `make` in the Robot-Project folder should compile the project without errors and yield an executable called `main` in the Robot-Project directory.

## Connecting to the robot
We can connect to the robot using SSH. This requires that the robot is either connected to the same wifi as your computer (using a wifi USB dongle) or connected directly to the computer using a USB cable. Either way, take note of the IP address of the robot which is displayed at the top of the ev3 brick's embedded display. Connecting is done by the following command, omitting the angle brackets:
```
$ ssh robot@<robot's IP>
```

## Copying executables to the ev3 brick and running them
Since the repository already contains static libraries, we can run the code directly on the ev3 brick without installing anything fancy on the brick. Using SCP, copy the executable to the robot, again omitting the angle brackets:
```
$ scp main robot@<robot's IP>:~/
```
Connect to the robot and start the executable:
```
$ ./main
```