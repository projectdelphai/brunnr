brunnr
===============
Reuben Castelino - projectdelphai@gmail.com

Description
-------------
A program written in C that is designed to interact with arduinos through the serial port.

Usage
-------------
brunnr is designed as a messaging service so that one can share messages and notifications between a host computer and one or more arduinos. It can run as a simple real time program by reading and/or writing data straight to a serial port. However, it is also designed to run as queue for when machines are offline. It can store data in a database that is then sent to the corresponding machine when it (be it an arduino or not) comes online. So arduinos can send data to other arduinos or to regular computers. Since the data is stored in a regular sqlite database, you can also write programs to interact with the database and retrieve messages.

Examples
-------------
NOTE: Not all of these features have been implemented, your results may vary.

To run on port /dev/ttyASM0 and print to stdout ("-o stdout" is implied):

     brunnr -p /dev/ttyASM0

To run on multiple ports and only print the next 5 incoming messages to stdout ("-o stdout" is implied):

    brunnr -p /dev/ttyASM0,/dev/ttyASM1 -n 5

To run on port /dev/ttyASM0 and print to a database file:

    brunnr -p /dev/ttyASm0 -o db -f /home/user/arduino_messages.db

To write a message to a database file:

    brunnr -w source:target:message

To view all the messages in a database:

    brunnr -r -f /home/user/arduino_messages.db

To view the 10 latest messages from arduino1 (source) in a database:

    brunnr -r -n 10 -s arduino1 -f /home/user/arduino_messages.db

To view all messages from server1 (target) in a database:

    brunnr -r -t server1 -f /home/user/arduino_messages.db

Installation
-------------
brunnr can be easily compiled with make:

    make

You can then run it with:

    ./brunnr

To create a permanent installation:

    make install

Contribution
------------
Contributions are always welcome. 

 1. Fork this project
 1. Create a branch to work on (optional)
 1. Make your changes.
 1. Edit the readme to reflect your changes (if necessary)
 1. Commit and push your changes to your repo
 1. Make a pull request.

Feel free to contact me if you have any questions.
