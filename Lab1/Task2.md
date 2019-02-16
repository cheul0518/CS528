### Task2.a Write a spoofing program
- Write your own packet spoofing program. Using this program to perform the following tasks. You will need to submit this program along with your report

```c
/* I'm going to create my ICMP packet, which is type echo-request,
 * and hand it over to the raw sockets API to deliver it to the network
 */
#include <stdio.h>
#include <stdlib.h>
#include <unitstd.h>
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

// Internet checksum function. This function is used for calculating checksum for ICMP protocol
unsigned short chksum(unsigned short *addr, int len){
 int nleft = len;
 int sum = 0;
 unsigned short *w = addr;
 unsigned short answer = 0;
 
 while (nleft > 1){
  sum += *w++;
  nleft -= 2;
 }
 
 if (nleft == 1){
  *(unsigned char *) (&answer) = *(unsigned char *) w;
  sum += answer;
 }
 
 sum = (sum >> 16) + (sum & 0xFFFF);
 sum += (sum >> 16);
 answer = ~sum;
 return (answer);
}

int main(int argc, char **argv){
 struct ip ip;
 struct udphdr udp;
 struct icmp icmp;
 const int on = 1;
 u_char *packet;
 int sd;
 struct sockaddr_in sin;
 char buffer[1024]; // You can change the buffer size
 
 // Grab some space for my packet
 packet = (u_char *)malloc(60);

 // Fill Network layer fields. Header length in units of 32bits.
 // Assuming any OP options not sending, IP header length is 20 bytes, so 20/4 = 5
 ip.ip_hl = 0x5;

 // IPv4
 ip.ip_v = 0x4;

 // Type of Service. Packet precedence
 ip.ip_tos = 0x0;
 
 // Total length for the packet. It's converted to the network byte-order
 ip.ip_len = htons(60);
 
 // ID field uniquely identfies each datagram sent by this host
 ip.ip_id = htons(12830);
 
 // Fragment offset for the packet. Set it to 0x0 due to no need for any fragmentation
 ip.ip_off = 0x0;
 
 // Time to live. Maximum number of hops that the packet can pass while travelling through its destination
 ip.ip_ttl = 64;
 
 // Transport Layer protocol number
 ip.ip_p = IPPROTO_ICMP;
 
 // Set the checksum value to 0 before passing the packet into the checksum function
 ip.ip_sum = 0x0;

 // Source IP address
 ip.ip_src.s_addr = inet_addr("192.168.15.17");
 
 // Destination IP address
 ip.ip_dst.s_addr = inet_addr("google.com");
 
 // Pass the IP header and its length into the checksum function.
 // The function returns a 16-bit checksum value for the header
 ip.ip_sum = chksum((unsigned short *)&ip, sizeof(ip));
 
 // Copy the IP header into the very beginning of the packet
 memcpy(packet, &ip, sizeof(ip));
 
 // ICMP type
 icmp.icmp_type = ICMP_ECHO;
 
 // Code: Echo Request
 icmp.icmp_code = 0;
 
 // ID: Random number
 icmp.icmp_id = 1000;
 
 // ICMP sequence number
 icmp.icmp_seq = 0;
 
 
 
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
 

1.
2.
3. Question 6: Why do you need the root priviledge to run the programs that use raw sockets? Where does the program fail if executed without the root privilege?

   - Raw sockets offer the capability to manipulate the underlying transport, so they can be used for malicious purposes that pose a security threat. Therefore, only members of the Administrators group can create sockets of type SOCK_RAW
