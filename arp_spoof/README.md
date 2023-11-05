# Arp-Spoof

## Author(s):

Maxwell Yass

## Date:

30 October 2023

## Description:

This code implements a simple arp-spoof exploit. An ARP spoof exploit is a cyberattack where an attacker manipulates the Address Resolution Protocol (ARP) to associate their own MAC address with a legitimate IP address. This allows an attacker to intercept network traffic and set up for various attacks such as eavesdropping or man-in-the-middle attacks.

** Within this directory are a client and server image that you should exlusively test this exploit on. Using this software on a public network to gain an offensively maliscious position is illegal. **

## How to build the software in this repository

1\. Clone the base repository to your local machine to have access to all the exploits:
	
	git clone https://github.com/maxyass/Projects-Im-Proud-Of

2\. Navigate to the project directory of the exploit you want to test:
	
	cd arp_spoof

3\. Compile the program using make:
	
	make

## How to use the software

To use the program, follow these steps:

1\. Load both the server and client within QEMU by running:

	qemu-system-x86_64 -m 2048 -nographic -net nic -net user,net=10.0.3.0/24 -device e1000,netdev=n1,mac=52:54:00:12:34:56 -netdev socket,id=n1,listen=:1024 -hda arpspoof-server-sanitized-openwrt-x86-64-generic-ext4-combined.img
and:

	qemu-system-x86_64 -m 2048 -nographic -device e1000,netdev=n1,mac=52:54:00:12:34:57 -netdev socket,id=n1,connect=:1024 -hda arpspoof-client-sanitized-openwrt-x86-64-generic-ext4-combined.img

2\. In a third terminal window compile the software by running:

	make

3\. Copy the executable over to the client:

Locally run:

	nc -l 2048 -c \"/bin/cat arpspoof\"

On the client run:

	nc I 2048 >arpspoof

Where <I> is your local IP Address

4\. The server has a UDP Beacon that you can point to IP Address H by running:

	udpbeacon H

5\. Run the arp_spoof executable by running:

	./arpspoof MY_INTERFACE MY_MAC HIJACKED_IP VICTIM_MAC VICTIM_IP

Where: 

	MY_INTERFACE = The name of the client’s network interface.

	MY_MAC = The client’s MAC address.

	HIJACKED_IP = The IP address of the UDP beacon is pointing at.

	VICTIM_MAC = The server’s MAC address.

	VICTIM_IP = The IP address 

6\. Observe the \[Sanatized\] secret that the udpbeacon was attempting to send to H being printed on your cleint's STDOUT


## How the software was tested

The software was tested using a variety of manual testing mostly consisting of input 'corner cases'. 

## Known bugs and problem areas

There are currently no known bugs with this software, please report any bugs or problems you encounter during usage.
