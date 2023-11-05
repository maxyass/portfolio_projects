# UDP Port Scanner

## Author(s):

Maxwell Yass

## Date:

23 October 2023

## Description:

This code implements a simple UDP port scanner. Something like nmap uses TCP by default (the '-sU' flag makes it use UDP). The biggest difference between UDP and TCP is that TCP has this concept of a 'connection' between the client and host. Something that UDP does not provide. A TCP port scanner would cycle through the ports attempting to 'connect' to each. For each port that is not being listen on the scanner would receive back a message from the host saying that it is not listening on that port. Since UDP does not have a 'connection' per say it will not let you know when a port is not active. Because of this, my version of a UDP scanner utilizes a timeout that will break it out of all the ports that are not being listened on.

A service is included that will provide a secret to any connections made

** Using this software on a public network for maliscious activities is illegal. **

## How to build the software in this repository

1\. Clone the base repository to your local machine to have access to all the exploits:
	
	git clone https://github.com/maxyass/Projects-Im-Proud-Of

2\. Navigate to the project directory of the exploit you want to test:
	
	cd Projects-Im-Proud-Of/udp_scan

3\. Compile the program using make:
	
	make

## How to use the software

To set up the port scanner, follow these steps:

1\. Compile the project by running:

	make

2\. In one terminal window run netcat that will sit and listen for connections on Port P and pass those connections to the service by running:

	nc --udp -l -p P -c ./service_udp_scan


3\. In a second terminal window, have the port scanner locally scan from Port P1 to Port P2 by running:

	./udp_scan localhost P1 P2

4\. Assuming P1 <= P <= P2: When port P is hit the service will pass the scanner a big secret!


## How the software was tested

The software was tested locally by running the steps as described above. As well as testing poorly formatted inputs to ensure error handling is set up properly, along with very wide ranges of ports.

## Known bugs and problem areas

There are currently no known bugs with this software, please report any bugs or problems you encounter during usage.
