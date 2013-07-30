EventCoordinator
================

A Distributed Event Coordination System

Compiling with makefile.
  - Move to the folder of the server in terminal.
	- Type "make" and press return.

Compiling without makefile.
	To compile server
	- gcc dec_server.c -o dec_server
	To compile client
	- gcc dec_client.c -o dec_client

To start server
	- Type ./dec_server <options> and press return.
	- To see options type ./dec_server -h and press return.

To start client
	- Type ./dec_client <options> and press return.
	- To see options type ./dec_client -h and press return.

Once the server and client are started, you can give requests in any form as mentioned in the project document.
Event name can be either small capital letters. Event names should be single character.
