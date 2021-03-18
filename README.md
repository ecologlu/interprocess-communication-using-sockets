This is not a personal project, but an old schoolwork I particularly liked.

To run it, start the server.cpp and provide an integet port no as an argument, then use the same port no while running client.cpp

The client will read the ip/subnet file from cin, and send it to the server with as many child threads as there are lines. 

The server will wait for the client infinitely unless it receives the ip and subnet addresses. After it succeeds the connection (line 77), it will create a child proceess and calculate network and broadcast addresses for that specific ip/subnet. It will then return the calculated network and broadcast addresses to client.cpp and client.cpp will print them out.