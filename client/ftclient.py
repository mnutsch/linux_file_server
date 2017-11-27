#*************************************************************************
# Name: ftclient.py
# Author: Matt Nutsch
# Date: 11-24-2017
# Description: 
# This is a TCP/IP file transfer client, written in Python.
# TCP IP connection code is based in part on the class lecture content from CS372.
# File output code is based on the example at: http://www.pythonforbeginners.com/files/reading-and-writing-files-in-python
#*************************************************************************

from socket import *
import sys

continueProgram = 1

debugging = 0 #change this to 1 to see verbose output

# if there are no arguments, then ask the user to rerun with the correct parameters 
if debugging == 1:
	print 'Number of arguments:', len(sys.argv), 'arguments.'

if len(sys.argv) < 4:
	print 'Insufficient arguments received. Rerun the program with the appropriate commands i.e. python ftclient.py flip 30021 -l 30020'
else: 

	sentence = ""

	try:
		if debugging == 1:
			print 'Using this value as the server host: ',  sys.argv[1]
		serverName = sys.argv[1]
	except: 
		print 'missing argument 1'
		continueProgram = 0
	
	try:
		if debugging == 1:
			print 'Using this value as the server port: ',  sys.argv[2]
		serverPort = int(sys.argv[2])
	except: 
		print 'missing argument 2'
		continueProgram = 0
		
	try:
		if debugging == 1:
			print 'Using this value as the command: ',  sys.argv[3]
		command = sys.argv[3]
	except: 
		print 'missing argument 3'
		continueProgram = 0
	
	if(sys.argv[3] == "-l"):
		if debugging == 1:
			print 'The command is to list directory contents.'
		
		try:
			if debugging == 1:
				print 'Using this value as the dataPort: ',  sys.argv[4]
			dataPort = int(sys.argv[4])
			sentence = str(command) + ' ' + str(dataPort)
		except: 
			print 'missing argument 4'
			continueProgram = 0
	
	if(sys.argv[3] == "-g"):
		if debugging == 1:
			print 'The command is to get a file.'
		
		try:
			if debugging == 1:
				print 'Using this value as the fileName: ',  sys.argv[4]
			fileName = sys.argv[4]
		except: 
			print 'missing argument 4'
			continueProgram = 0
			
		try:
			if debugging == 1:
				print 'Using this value as the dataPort: ',  sys.argv[5]
			dataPort = int(sys.argv[5])
			sentence = str(command) + ' ' + str(fileName) + ' ' + str(dataPort)
		except: 
			print 'missing argument 5'
			continueProgram = 0
	
	if debugging == 1:
		print 'continueProgram == ', continueProgram
		print 'sentence == ', sentence
	
	#call the file server
	
	#establish the connection
	clientSocket = socket(AF_INET, SOCK_STREAM)
	clientSocket.connect((serverName,serverPort))
	
	#send the text
	clientSocket.send(sentence)
		
	#listen for a reply
	textReceived = clientSocket.recv(1024)
		
	#output the text received from the server
	print 'From Server:', textReceived

	#############################################
	
	#set up a server to receive the message from the file server
	
	#confirm which port number we are using
	if debugging == 1:
		print 'Using this value as the port number: ', dataPort
	serverPort = int(dataPort)
	
	#set up the socket to listen for the client
	serverSocket = socket(AF_INET,SOCK_STREAM)
	serverSocket.bind(('',serverPort))
	serverSocket.listen(1)
	if debugging == 1:
		print 'The server is ready to receive.'
	
	connectionSocket, addr = serverSocket.accept()	
	
	if debugging == 1:
		print "waiting for client..."
	
	#receive messages from the client
	sentence = connectionSocket.recv(1024)
		
	#output the message received to the server console
	if(sys.argv[3] == "-l"):
		print 'Receiving directory structure from ' + str(serverName) + ':' + str(serverPort)
		print sentence 
		
	if(sys.argv[3] == "-g"):
		print 'Receiving "' + fileName + '" from ' + str(serverName) + ':' + str(serverPort)
		file = open(fileName,"w") 
		file.write(sentence) 
		file.close() 
		print 'File transfer complete.'
		
	#get the text to send
	if debugging == 1:
		textToSend = "Message received."
		
	#send the text
	if debugging == 1:
		connectionSocket.send(textToSend)
		
	#############################################	
	
	#close the original connection
	clientSocket.close()
	
	#close the second connection
	connectionSocket.close()
	
	
	
