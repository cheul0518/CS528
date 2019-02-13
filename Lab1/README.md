### Summary: Programming with Pcap (Tim Carstens)

**1. Setting the Device**

- Determine which interface you want to sniff on.
- You can either define this device in a strong, or you can ask pcap to provide you with the name of an interface.

```c
// Define the device in a string
#include <stdio.h>
#incoude <pcap.h>

int main(int argc, char *argv[]){
  char *dev = argv[1];
  
  print("Device: %s\n", dev);
  return(0);
}
```

```c
// PCAP sets the device on its own
#include <stdio.h>
#include <pcap.h>

int main(int argc, char *argv[]){
  char *dev, errbuf[PCAP_ERRBUF_SIZE];
  
  dev = pcap_lookupdev(errbuf);  // if pcap_lookupdev() fails, it will store an error msg in errbuf
  if (dev == NULL){
    fprintf(stderr, "Couldn't find default device; %s\n", errbuf);
    return(2);
  }
  printf("Device: %s\n", dev);
  return(0);
}
```

<br />

**2. Opening the device for sniffing**

- Initialize pcap
- Sniffing on multiple devices, name your sniffing "session" so you can tell it apart from other such sessions

```c
- pcap_t *pcap_open_live(char *device, int snaplen, int promisc, int to_ms, char *ebuf)
  0. Returns pcap_t * on success, NULL on failure. In other words, it returns your session handler
  1. char *device: the device you speficied
  2. int snaplen: an interger that defines the maximum number of bytes to be captured by pcap
  3. int promisc: promiscuous mode (set to ture). Sometimes, under specific cases, promiscuous mode's forced to set true
  4. to_ms: the read time out in milliseconds (0: no time out)
  5. ebuf: a string you can store any error messages within
  
#include <pcap.h>
...
pcap_t *handle;

handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
if(handle == NULL){
  fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
  return(2);
}

```
<br />

- Non-promiscuous sniffing: a host is sniffing only traffic that is directly related to it so only traffic to, from, or routed through the host will be picked up by the sniffer
- Promiscuous sniffing: it sniffs all traffic on the wire. However it has regressions. First of all, promiscuous mode sniffing is dtectable; a host can test with strong reliability to determine if another host is doing promiscuous sniffing. Secondly it only works in a non-switched environment such as hub, or a switch that is being ARP flooded. Third, on high traffix networks, the host can become quite taxed for system resources

<br />

- Not all devices provide the same type of link-layer headers in the pacekts you read. Etherenet devices, and some non-Ethernet devices, might provide Ethernet headers, but other devices such as loopback devices in BSD and OS X,PPP interfaces, and Wi-Fi interfaces when capturing in monitor mode, don't
- You need to determine the type of link-layer headers the device provides, and use that type when processing the packet contents. If your program doesn't support the link-layer header type provided by the device, it has to give up; this would be done with code such as

```c
// pcap_datalink() returns a value indicating the type of link-layer headers
if (pcap_datalink(handle) != DLT_EN10MB){
  fprintf(stderr, "Device %s doesn't provide Ethernet headers - not supported\n", dev);
  return(2);
}  // which fails if the device doesn't supply Ethernet headers
```

<br />

**3. Filterting traffic**

- This is a three phase process

- Create a rule set (if you only want to sniff specific traffic. e.g.: only TCP/IP packets, only packets going to port 23, etc)
- Compile it

- Apply it

- Specifically the rule set is kept in a string, and is converted into a format that pcap can read. The compilation is actually just done by calling a function within your program; it doesn't involve the use of an external application. Then you tell pcap to apply it to whichever session you wish for it to filter


```c
// Before applying your filter, you must "compile" it
- int pcap_compile(pcap_t *p, struct bpf_program *fp, char *str, int optimize, bpf_u_int32 netmask)
0. It returns -1 on failure; all other values imply success
1. pcap_t *p: your session handle
2. struct bpf_program *fp: a reference to the place you will store the compiled version of your filter
3. char *str: the expression itself
4. int optimize: an integer thar decides if the expression should be "optimized" or not (0: false, 1: true)
5. bpf_u_int32 netmask: the network mask of the network the filter applies to

// Apply it
- int pcap_setfilter(pcap_t *p, struct bpf_program *fp)
0. It returns -1 on failure, o on success
1. pcap_t *p: your session handle
2. struct bpf_program *fp: a reference to the compiled version of the expression (presumably the same variable as the second argument to pcap_compile())
```

```c
// This program preps the sniffer to sniff all traffic coming from or going to port 23, in promiscuous mode, on the device r10
#include <pcap.h>
...
pcap_h *handle                   // Session handle
char dev[] = "r10";             // Device to sniff on
char errbuf[PCAP_ERRBUF_SIZE]   // Error string
struct bpf_program fp;          // The compiled filter expression
char filter_exp[] = "port 23";  // The filter expression
bpf_u_int32 mask;               // The netmask of our sniffing device
bpf_u_int32 net;                // The IP of your sniffing device

// This function is for finding the IPv4 network and netmask for a device. It returns -1 on failture 0 on success
// The network number is the IPv4 address ANDed with the network mask, so it contains only the network part of the address
// This process is essential because you need to know the network mask in order to apply the filter
if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1){
  fprintf(stderr, "Can't get netmask for device %s\n", dev);
  net = 0;
  mask = 0;
}

handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
if (handle == NULL) {
  fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
  return(2);
}
if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1){
  fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
  return(2);
}
if (pcap_setfilter(handle, &fp) == -1){
  fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
  return(2);
}
```

<br />

**4. The actual sniffing**

- There are two main techniques for capturing packets: 1)caputre a single packet at a time or 2)enter a loop that waits for n number of packets to be sniffed before being done.
- Sniff a packet
```c
#include <pcap.h>
#include <stdio.h>

int main(int argc, char *argv[]){
  pcap_t *handle; // Session handle
  char *dev;  // The device to sniff on
  char errbuf[PCAP_ERRBUF_SIZE];  // Error string
  struct bpf_program fp;  // The compiled filter
  char filter_exp[] = "port 23" // The filter expression
  bpf_u_int32 mask; // Our netmask
  bpf_u_int32 net;  // Out IP
  struct pcap_phthdr header;  // The header that pcap gives us
  const u_char *packet; // The actual packet
  
  // Define the device
  dev = pcap_lookupdev(errbuf);
  if (dev == NULL){
    fprintf(stderr, "Couldn't get default device: %s\n", errbuf);
    return(2);
  }
  
  // Find the properties for the device
  if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
    fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
    net = 0;
    mask = 0;
  }
  
  // Open the session in promiscuous mode
  handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
  if (handle == NULL){
    fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
    return(2);
  }
  
  // Compile and apply the filter
  if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1){
    fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
    return(2);
  }
  
  if (pcap_setfilter(handle, &fp) == -1) {
    fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
    return(2);
  }
  
  // Grab a packet
  /*
  - u_char *pcap_next(pcap_t *p, struct pcap_pkthdr *h)
  1. *p is your session handler
  2. *h is a pointter to a structure that holds general information about the packet
  3. It returns a u_char pointer to the packet
  */
  packet = pcap_next(handle, &header);

  // Print its length
  printf("Jacked a packet with length of [%d]\n", header.len);
  
  // Close the session
  pcap_close(handle);
  return(0);
}
```

<br />

- Using loops to catpchutre multiple packets at a time

```c
/* 
Few sniffers actually use pcap_next(). They use pcap_loop() and pcap_dispatch().
Both functions call a callback function every time a pcket is sniffed that meets your filter requriements 
(if any filter exists, of course. If not, then all packets that are sniffed are sent to the callback)

- int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
1. pcap_t *p: session handle
2. int cnt: an integer that tells pcap_loop() how many packets it should sniff for before returning
  (a negativ value means it should sniff until an error occurs)
3. pcap_handler callback: the number of the callback function (no parentheses needed)
4. u_char *user -> useful in some applications but many times is simply set as NULL.

- Difference between pcap_dispatch() and pcap_loop()
  1) pcap_dispatch() will only process the filter batch of packets that it receives from the system
  2) pcap_loop() will continute processing pacekts or batches of packets until the count of packets runs out

- The format of your callback function must be examined before using pcap_loop().
  This is because you cannot arbitarily define your callback's prototype   
  
- void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
1. u_char *args: it corresponds to the last argument of pcap_loop(). Whatever value is passed as the last
                argument to pcap_loop() is passed to the first argument of your callback function everytime
                the function is called
2. const struct pcap_pkthdr *header: it contains information about when the packet was sniffed.
  - The pcap_pkthd structure is as follow:
  struct pcap_pkthdr {
    struct timeval ts;  // time stamp
    bpf_u_int32 caplen; // length of portion present
    bpf_u_int32 len;    // length this packet (off wire)
  };
3. const u_char *packet: another pointer to a u_char, pointing to the first byte of a chunk of data 
                        containing the entire packet, as sniffed by pcap_loop()

Next question will be "How do you make use of this variable?" A packet contains many attributes. 
It is not really a string but a collection of structures. This u_char pointer points to the serialized version 
of these strcutures. To make any use of it, you must do typecasting.

*/

// Etherenet addresss are 6 bytes
# define ETHER_ADDR_LEN 6

  // Ethernet header
  struct sniff_ethernet{
    u_char ether_dhost[ETHER_ADDR_LEN]; // Destination host address
    u_char ether_shost[ETHER_ADDR_LEN]; // Source host address
    u_short ether_type; // IP? ARP? RARP? etc
  };
  
  // IP header
  struct sniff_ip{
    u_char ip_vhl;  // version <<4 | header length >> 2
    u_char ip_tos;  // type of service
    u_short ip_len; // total length
    u_short ip_id;  // identification
    u_short ip_off; // fragment offset field
  
  #define IP_RF 0x8000  // reserved fragment frag
  #define IP_DF 0x4000  // dont fragment flag
  #define IP_MF 0x2000  // more gragments flag
  
    u_char ip_ttl;  // time to live
    u_char ip_p;    // protocol
    u_short ip_sum; // checksum
    struct in_addr ip_src,ip_dst; // source and dest address
  };
  #define IP_HL(ip) (((ip)->ip_vhl) & 0x0f)
  #define IP_V(ip)  (((ip)->ip_vhl) >> 4)
  
  // TCP header
  typedef u_int tcp_seq;
  
  struct sniff_tcp{
    u_short th_sport; // source port
    u_short th_dport; // destination port
    tcp_seq th_seq;   // sequence number
    tcp_seq th_ack;   // acknowledgement number
    u_char th_offx2;  // data offset, rsvd
    
  #define TH_OFF(th)  (((th)->th_offx2 & 0xf0) >> 4)
    u_char th_flags;
  #define TH_FIN 0x01
  #define TH_SYN 0x02
  #define TH_RST 0x04
  #define TH_PUSH 0x08
  #define TH_ACK 0x10
  #define TH_URG 0X20
  #define TH_ECE 0x40
  #define TH_CWR 0x80
  #define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win; // window
    u_short th_sum; // checksum
    u_short th_urp; // urgent pointer
  };
  
// ethernet headers are always exactly 14 bytes  
#define SIZE_ETHERNET 14
  const struct sniff_ethernet *ethernet;  // The ethernet header
  const struct sniff_ip *ip;  // The IP header
  const struct sniff_tcp  *tcp; // The TCP header
  const char *payload;  // Packet payload
  
  u_int size_ip;
  u_intt size_tcp;
  
  ethernet = (struct sniff_ethernet*)(packet);
  ip = (struct sniff_ip*)(packet+SIZE_ETHERNET);
  size_ip = IP_HL(ip)*4
  if (size_ip < 20){
    printf(" *Invalid IP header length: %u bytes\n", size_ip);
    return;
  }
  tcp = (struct sniff_tcp*)(packet +SIZE_ETHERNET +size_ip);
  size_tcp = TH_OFF(tcp)*4;
  if (size_tcp < 20){
    printf("" *Invalid TCP header length: %u bytes\n", size_tcp);
    return;
  }
  payload = (u_char *)(packet + SIZE_ETHERNET +size_ip +size_tcp);
  

```
