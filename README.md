# linux_file_server
Description
This is a file server written in C for my Networking class at OSU. The server is intended to run on a Linux server and be compiled with gcc. 
An example client written in Python is also provided.

INSTRUCTIONS<br/>

Set up:<br/>
	Unzip this file to a Linux based server, such as OSU's flip.<br/>
	Compile the server with the commands:<br/>
		cd server<br/>
		gcc -pthread -o ftserver ftserver.c<br/>

Start the file server:<br/>
	Start the server with the command:<br/>
		ftserver (port)

Example - Request directory contents from the file server with the client:<br/>
	cd client<br/>
	python ftclient.py localhost 23233 -l 23239<br/>

Example - Download a file with the client:<br/>
	python ftclient.py localhost 23233 -g alice.txt 23240<br/>
