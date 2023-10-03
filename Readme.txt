Hello and Welcome to my Promgramming Assignment 2 - Implementing UDP 

To run and test my submissions simply run these commands: 

1. If the server executable does not exist run: gcc server.c -o server -Wall
2. If the client executable does not exist run: gcc client.c -o client -Wall
3. Next in a new terminal run: ./server 5 0.1
4. In another new terminal run: ./client
5. After running /client you will presented with what file to transmit. Type "test.txt"
6. Check the terminal for the expected outputs 

After testing it seems to work as directed! 

All the client code is of course in client.c 
This file gets the file name and ACK loss ratio from the user, it creates the UDP socket and configures the server address.
Afterwards it send the file name to the server.
Once it receive file data packets from the server, it store the data in the output file (out.txt), and print appropriate messages.

The server.c file contains all of the server code and simulates a server. 
This file create a UDP socket, configures the server address, then bind the sockets.
Once it reads the file and sends the data packets to the client, simulating packet loss during transmission it terminates the program.

This was very fun project and attemping this after programing assignment 1 provided much more clarity on how to implement sockets and switches.
This time I am happy to say it works and I get the desired output.
This was a great learning experience and gave me a much deeper understanding of the material learned in this course.