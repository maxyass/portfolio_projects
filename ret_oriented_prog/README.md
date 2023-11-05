# Return Oriented Programming

## Author(s):

Maxwell Yass

## Date:

16 October 2023

## Description:

This code implements the concept of Return Oriented Programming or ROP. The idea of ROP is that sometimes the kernel can implement a countermeasure against buffer overflow attacks called a no execute bit which makes portions of the stack not be able to hold executable code. This thwarts the standard buffer overflow / shellcode attack such as the nop_sled attack. With ROP we can use tools to help us find 'gadgets', or portions of assembly code, within the existing code that when combined in the right order can do as the attacker desires. The way it works is when the buffer is overflown, the attacked injects the addresses of the desired gadgets such that the PC will jump to those addresses (that are in a portion of the stack that is executable) and simply execute them.

In this specific exploit I \(the attacker\) has already found the addresses in the vulnerbable service that contain various ROP Gadgets that cause the service to open a file it's local directory, and print the file's contents to STDOUT. The file is named 'flag' and contains a big secret! The service is vulnerable because it contains the 'gets' method which does not check the bounds of a buffer and can easily cause a buffer overflow. 

** Using this software on a public network for maliscious activities is illegal. **

## How to build the software in this repository

1\. Clone the base repository to your local machine to have access to all the exploits:
	
	git clone https://github.com/maxyass/Projects-Im-Proud-Of

2\. Navigate to the project directory of the exploit you want to test:
	
	cd Projects-Im-Proud-Of/ret_oriented_prog

3\. Compile the program using make:
	
	make

## How to use the software

To exploit the vulnerable service, follow these steps:

1\. Compile the program by running:

	make

2\. Listen for input on Port P and pipe it into the vulnerable service by running:

	nc -l -p P | ./vulnerable-service-rop


3\. In a second terminal window, send the maliscious input to the vulnerable service by running:

	./rop localhost P

4\. Observe the contents of the 'flag' file being leaked


## How the software was tested

The software was tested against the vulnerable service by running it as described above. As well as testing poorly formatted inputs to ensure error handling is set up properly. 

## Known bugs and problem areas

There are currently no known bugs with this software, please report any bugs or problems you encounter during usage.
