# TLS Proxy

## Author(s):

Maxwell Yass

## Date:

3 November 2023

## Description:

This code implements a proxy that communicates between a client and a server. However it acts as the server to the client. When a browser accesses a web page it usually (based on the browser) will attempt an unsecure HTTP connection just to have the server route it to a secure HTTPS connection via TLS. If an attacker uses something like an ARP Spoofing attack to gain a Man in the Middle position then it can act as the server to the browser. This allows it to have a secure TLS connection with the server and an unsecure connection to the browser. 

All of the files required to make the TLS connection can be found in the tls_help and include directories. I am using a service called MBEDTLS to help provide me the TLS connection

** Using this software on a public network for maliscious activities is illegal. **

## How to build the software in this repository

1\. Clone the base repository to your local machine to have access to all the exploits:
	
	git clone https://github.com/maxyass/Projects-Im-Proud-Of

2\. Navigate to the project directory of the exploit you want to test:
	
	cd Projects-Im-Proud-Of/tlsproxy

3\. Compile the program using make:
	
	make

## How to use the software

To set up the TLS proxy, follow these steps:

1\. Compile the project by running:

	make

2\. In one terminal window run the service, that will sit and listen for connections, by running:

	./service-tlsproxy


3\. In a second terminal window, set up the proxy by running:

	./tlsproxy L localhost P

Where the proxy will listen on port L for a connection and will connect to the local service on port P.

4\. In a third terminal window send a GET request to the service (really the proxy) by running:

	echo -e "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n" | nc localhost L


## How the software was tested

The software was tested locally by running the steps as described above. As well as testing poorly formatted inputs to ensure error handling is set up properly. 

## Known bugs and problem areas

Based on the system you are running may get 2 warnings from compilation. Neither will effect how the program operates.
Please report any further bugs or problems areas you encounter during usage.
