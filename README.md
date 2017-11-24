# Robot-Project

Getting started:
To get started, clone the repository to a folder on your machine. Within the repository, clone https://github.com/in4lio/ev3dev-c.git This is a library that is needed by our program. Now, move into ev3dev-c/source/ev3 and run "make" and "sudo make install".
To cross compile, get the tool described here https://www.acmesystems.it/arm9_toolchain (the one under "Arietta, Aria or FOX G20 board"). Now the setup should be complete, and you should be able to compile the project by running "make" in the repository directory, the build output will be located in the build folder.

The makefile is here to make it easier for us to compile the program. Instead of running every gcc command each time, we simply run "make" and the project will be built.

We should always work on branches (maybe except for the first few commits), and create pull-request that should be reviewed before being merged with the master branch.
