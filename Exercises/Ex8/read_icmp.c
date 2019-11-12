#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <errno.h>

#define BUFSIZE 1500

char *get_ip (struct in_addr addr) {
	char *ip = malloc(INET6_ADDRSTRLEN);
	inet_ntop(AF_INET, &addr, ip, INET6_ADDRSTRLEN);
	return ip;
}

int main () {
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == -1) {
		perror("socket() err");
		if (errno == EPERM) {
			printf("Try with sudo\n");
		}
		exit(0);
	}

	char recvbuf [BUFSIZE];
	char controlbuf [BUFSIZE];
	struct iovec iov;
	struct msghdr msg;

	iov.iov_base = recvbuf;
	iov.iov_len = sizeof(recvbuf);

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = controlbuf;
	msg.msg_controllen = sizeof(controlbuf);
	
	printf("Waiting for ICMP packets...\n\n\n");
	while (1) {
		int n = recvmsg(sockfd, &msg, 0);
		if (n == -1) {
			perror("recvmsg err");
			exit(0);
		}

		if (n > 0) {
			int ip_hlen, icmp_hlen;
			struct ip *ip;
			struct icmp *icmp;

			ip = (struct ip *) recvbuf;
			ip_hlen = ip->ip_hl << 2;

			char *srcip = get_ip(ip->ip_src);
			char *dstip = get_ip(ip->ip_dst);

			if (ip->ip_p != IPPROTO_ICMP) return 0;

			icmp = (struct icmp *) (recvbuf + ip_hlen);

			icmp_hlen = n - ip_hlen;

			if (icmp_hlen < 8) return 0;

			printf("ICMP message details:\n");
			printf("Source IP: %s\n", srcip);
			printf("Destination IP: %s\n", dstip);
			printf("Type: %d\n", icmp->icmp_type);
			printf("SubCode: %d\n", icmp->icmp_code);
			printf("Checksum: %d\n", icmp->icmp_cksum);
			printf("Sequence number: %d\n", icmp->icmp_seq);
			printf("Identifier: %d\n", icmp->icmp_id);
			printf("\n\n");
		}
	}
}
