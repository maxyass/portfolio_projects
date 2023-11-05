# NOP Sled

## Author(s):

Maxwell Yass

## Date:

15 October 2023

## Description:

This code implements the concept of a No-Operation sled or a NOP sled for short. A NOP sled is implemented by an attacker when the precise location of a buffer is not known in memory. In order to solve this the attacker will fill the buffer with No Ops and then inject their shellcode at the end of the buffer. After the vulnerable buffer is overflown the attacker can jump the program counter back into the buffer. But remember, in this scenario the location of the buffer is unknown. However, the attacker just needs to make a good enough guess such that execution will land on the NOP sled and safely ride its way to the injected shellcode.

In this specific exploit the 'shellcode' injected into the vulnerable service forces it to open a file that is in the service's local directory, and print it's contents to STDOUT. The file is named 'flag' and contains a big secret!

The contents of the assembly code injected into the buffer have been included, for reference, in a file named 'assembly.S'

** Using this software on a public network for maliscious activities is illegal. **

## How to build the software in this repository

1\. Clone the base repository to your local machine to have access to all the exploits:
	
	git clone https://github.com/maxyass/Projects-Im-Proud-Of

2\. Navigate to the project directory of the exploit you want to test:
	
	cd Projects-Im-Proud-Of/nop_sled

3\. Compile the program using make:
	
	make

## How to use the software

To exploit the vulnerable service, follow these steps:

1\. Compile the program by running:

	make

2\. Listen for input on Port P and pipe it into the vulnerable service by running:

	nc -l -p P | ./vulnerable-service-nop


3\. In a second terminal window, send the maliscious input to the vulnerable service by running:

	./nop localhost P

4\. Observe the contents of the 'flag' file being leaked


## How the software was tested

The software was tested against the vulnerable service by running it as described above. As well as testing bad inputs to ensure error handling is set up properly. 

## Known bugs and problem areas

There are currently no known bugs with this software, please report any bugs or problems you encounter during usage.
