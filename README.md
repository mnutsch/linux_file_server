# linux_file_server
Description
This is a file server written in C for my Networking class at OSU. The server is intended to run on a Linux server and be compiled with gcc. 
An example client written in Python is also provided.

INSTRUCTIONS

Set up:
	Unzip this file to a Linux based server, such as OSU's flip
	Compile the server with the commands:
		cd server
		gcc -pthread -o ftserver ftserver.c

Start the file server:
	Start the server with the command:
		ftserver (port)

Request directory contents from the file server with the client:
	cd client
	python ftclient.py localhost 23233 -l 23239

Download a file with the client:
	python ftclient.py localhost 23233 -g alice.txt 23240
