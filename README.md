# Project1

This project simulates the routing process in a computer network with a known topology and known link costs between nodes. It uses Dijkstra’s algorithm to find the optimal, cheapest path from one node to another, generating a routing table, and be able to send packages (messages) from one computer to another target computer, using the data from the routing table, simulating a private chatting program. 

This project is conformed by 5 different c files:

s211.c -------- To be run on the server computer 1 (IP Address: 220.149.244.211)

c212.c -------- To be run on the client computer 2 (IP Address: 220.149.244.212)

c213.c -------- To be run on the client computer 3 (IP Address: 220.149.244.213)

c214.c -------- To be run on the client computer 4 (IP Address: 220.149.244.214)

c215.c -------- To be run on the client computer 5 (IP Address: 220.149.244.215)

If you want to access the 5 computers remotely by PuTTY, please follow the next steps:

1.	Open PuTTY. Write the IP address of the computer you want to access, port 5222 and press open.
2.	After logging in to the computer (you must have a user for this), clone our project folder entering this command line:  
git clone https://github.com/NachoYo/Project1
3.	Enter the Project1 folder with cd Project1/
4.	Compile the source code gcc FileName.c -o Name -pthread
For example, if you want to run the client code on computer 5 (220.149.244.215), the command line would be: gcc c215.c -o 215 -pthread
5.	Run the compiled file with ./Name
Following the previous example, the command line would be ./215
6.	NOTE: For proper functioning, please RUN SERVER PROGRAM FIRST (s211.c)
7.	After running the 5 programs on their corresponding computer, the programs will do the rest and you can start chatting!
