### Task2.a Write a spoofing program
- Write your own packet spoofing program. Using this program to perform the following tasks. You will need to submit this program along with your report

```c
int sd;
struct sockaddr_in sin;
char buffer[1024]; // You can change the buffer size

/* Create a raw socket with IP protocol. The IPPROTO_RAW parameter
 * tells the sytem that the IP header is already included;
 * this prevents the OS from adding another IP header.  
 */
 
sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
if(sd < 0) {
    perror("socket() error"); exit(-1);
}

/* This data structure is needed when sending the packets
 * using sockets. Normally, we need to fill out several
 * fields, but for raw sockets, we only need to fill out
 * this one field 
 */
 
sin.sin_family = AF_INET;
// Here you can construct the IP packet using buffer[]
//    - construct the IP header ...
//    - construct the TCP/UDP/ICMP header ...
//    - fill in the data part if needed ...
// Note: you should pay attention to the network/host byte order.
/* Send out the IP packet.
 * ip_len is the actual size of the packet. 
 */

if(sendto(sd, buffer, ip_len, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("sendto() error"); exit(-1);
}
```

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

/* Internet checksum function. This is for calculating checksums for all layers.
 * ICMP protocol mandates checksum, so you have to calculate it
 */
unsigned short in_cksum(unsigned short *addr, int len){
 int nleft = len;
 int sum = 0;
 unsigned short *w = addr;
 unsigned short answer = 0;
 
 while (nleft > 1){
  sum += *w++;
  nleft -= 2;
 }
 if (nleft == 1){
  *(unsigned char *)(&answer) = *(unsigned char *) w;
  sum += answer;
 }
 sum = (sum >> 16) + (sum & 0xFFFF);
 sum += (sum >> 16);
 answer = ~sum;
 return (answer);  
}

int main(int argc, char **arv){
 struct ip ip;
 struct udphdr udp;
 struct icmp icmp;
 int sd;
 const int on = 1;
 struct sockaddr_in sin;
 u_char *packet;
 
 // Grab space for a packet
 packet = (u_char *)malloc(60);
 
 // Fill IP protocol fields. Header length in iunits of 32bits (4bytes).
 // IP header length is 20 bytes, so you need to stuff (20/4 = 5)
 ip.ip_hl = 0x5;
 
 // Protocol Version is 4, meaning IPv4
 ip.ip_v = 0x4;
 
 // Packet precedence
 ip.ip_tos = 0x0;
 
 // All multibyte fields require to be converted to the network byte-order when the fields bigger than 8 bits
 ip.ip_len = htons(60);
 
 // ID field uniquely identifies each datagram sent by this host
 ip.ip_ip = htons(12830);
 
 // Fragment offset for the packet. I set this to 0x0 since i don't desire any fragmentation
 ip.ip_off =0x0;
 
 // Time to live. Maximum number of hops that the packet can pass while travelling through its destination
 ip.ip_ttl = 64;
 
 // Upper layer protocol number
 ip.ip_p = IPPROTO_ICMP;
 
 // I set the checksum value to zero before passing the packet into the checksum function.
 ip.ip_sum = 0x0;
 
 // Source IP address, any IP address that may or may not be one of the assigned address to one of my interfaces
 ip.ip_src.s_addr = inet_addr("192.168.15.16");
 
 // Destiination IP address
 ip.ip_dst.s_addr = inet_addr("192.168.15.17");
 
 // Pass the IP header and itis length into the Internet checksum function
 ip.ip_sum = in_cksum((unsigned short *)&ip, sizeof(ip));
 
 // copy it into the very beginning of the packet
 memcpy(packet, &ip, sizeof(ip));
 
 // As for ICMP, 
 icmp.icmp_type = ICMP_ECHO;
 
 // Code 0. Echo Request
 icmp.icmp_code = 0;
 
 // ID. random number
 icmp.icmp_id = 1000;
 
 // ICMP sequence number
 icmp.icmp_seq = 0;
 
 icmp.icmp_cksum = 0;
 icmp.icmp_cksum = in_cksum((unsigned short *)&icmp, 8);
 
 memcpy(packet + 20, &icmp, 8);
 
 if ((sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0){
  perror("raw socket");
  exit(1);
 }
 if (setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0){
  perror("setsockopt");
  exit(1);
 }
 memset(&sin, 0, sizeof(sin));
 sin.sin_family = AF_INET;
 sin.sin_addr.s_addr = ip.ip_dst.s_addr
 
 if (sendto(sd, packet, 60, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr)) < 0){
  perror("sendto");
  exit(1);
 }
 return 0;
}
```

5. Question 6: Why do you need the root priviledge to run the programs that use raw sockets? Where does the program fail if executed without the root privilege?
 - Raw sockets offer the capability to manipulate the underlying transport, so they can be used for malicious purposes that pose a security threat. Therefore, only members of the Administrators group can create sockets of type SOCK_RAW
