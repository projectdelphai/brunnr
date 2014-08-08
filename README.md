brunnr
===============
Reuben Castelino - projectdelphai@gmail.com

Description
-------------
A [progether](http://reddit.com/r/progether) project.

A program written in C that is designed to interact with arduinos through the serial port. Brunnr is designed as a way for a computer and 1+ arduinos to communicate both online and offline. It works by brunnr reading a special string from designated serial ports. This string contains three pieces of information: the source (the device sending the message), the target (the device which needs to read the message), and the message itself (a string) in the format "source:target:message". This string is parsed by brunnr and either outputted to stdout, written to a file, or written to a database. The database is a standard sqlite3 database which can be read by any program that can interat with sqlite database. 

*This portion is still under consideration*
This interaction isn't a one-way road either. Programs or other arduinos can write messages to the database that are designated to other arduinos. A message can be left on the database for arduinos that brunnr will write across the necessary serial port. Brunnr will write the corresponding messages to the arduino if the respective arduino requests that info (source and target is the same string and message = "request").

*Example*
 1. Arduino A leaves a message for Arduino B on Host 1: "b:a:message for b"
 1. Arduino B upon starting up requests brunnr for any messages: "b:b:request"
 1. Brunnr writes the string to Arduino B: "b:a:message for b"

Usage
-------------
brunnr is designed as a messaging service so that one can share messages and notifications between a host computer and one or more arduinos. It can run as a simple real time program by reading and/or writing data straight to a serial port. However, it is also designed to run as queue for when machines are offline. It can store data in a database that is then sent to the corresponding machine when it (be it an arduino or not) comes online. So arduinos can send data to other arduinos or to regular computers. Since the data is stored in a regular sqlite database, you can also write programs to interact with the database and retrieve messages.

Examples
-------------
NOTE: Not all of these features have been implemented and/or thoroughly tested so your results may vary.

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
