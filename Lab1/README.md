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

- u_char *pcap_next(pcap_t *p, struct pcap_pkthdr *h)
  0. It returns a u_char pointer to the packet that is described this structure
  1. pcap_t *p: your session handler
  2. struct pcap_pkthdr *h: a pointer to a structure that holds general information about the packet
     specifically the time in which it was sniffed, the length of this packet, and the length of its
     specific ortion (in case it is fragmentised)
 
/*
This application sniffs on a device. It finds the first packet to come across a certain port 
and tells the user the size of the packet
*/

#include <pcap.h>
#include <stdio.h>

int main(int argc, char *argv[]){
  pcap_t *handle;                 // Session handle
  char *dev;                      // The device to sniff on
  char errbuf[PCAP_ERRBUF_SIZE];  // Error string
  struct bpf_program fp;          // The compiled filter
  char filter_exp[] = "port 23";  // The filter expression
  bpf_u_int32 mask;               // Your netmask
  bpf_u_int32 net;                // Your IP
  struct pcap_pkthdr header;      // the header that pcap gives us
  const u_char *packet;           // The actual packet
  
  // Define the device
  dev = pcap_lookupdev(errbuf);
  if (dev == NULL){
    fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
    return(2);
  }
  
  // Find the properties for the device
  if (pcap_lookupnet(Dev, &net, &mask, errbuff) == -1){
    fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
    net = 0;
    mask = 0;
  }
  
  // Open the session in promiscous mode
  handle = pcap_open_live(dev, BUFFSIZ, 1, 1000, errbuf);
  if (handle == NULL){
    fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
    return(2);
  }
  
  // Compile the filter
  if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1){
    fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
    return(2);
  }

  // Apply the filter
  if (pcap_setfilter(handle, &fp) == -1){
    fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
    return(2);
  }
  
  // Grab a packet
  packet = pcap_next(handle, &header);
  
  // Print its length
  printf("Jacked a packet with length of [%d]\n", header.len);
  
  // And close the session
  pcap_close(handle);
  return(0);
}
```

<br />

- Using loops to catpchutre multiple packets at a time
- Few sniffers actually use pcap_next(). They use pcap_loop() or pcap_dispatch(). These two funcs ruquire an understanding of the idea of a callback function
- Both pcap_loop() and pcap_dispatch() call a callback function every time a packet is sniffed that meets your filter requirements (if any filter exists, of course. If not, then all packets that are sniffed are sent to the call back)

```c
- int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user)
  0. It returns 0 if cnt is exhausted. -1 on failure, -2 if the loop terminated due to a call to 
     pcap_breakloop() before any packets were processed. No return when live read timeouts occur
  1. pcap_t *p: session handler
  2. int cnt: an integer that tells pcap_loop() how many packets it should sniff for before 
              returning (a negative value means it should sniff until an error occurs)
  3. pcap_handler callback: the name of the callback function (its idenfier, no parenthesis)
  4. u_char *user: NULL
  * Suppose you have arguments of your own that you wish to send to your callback function, in 
    addition to the arguments that pcap_loop() sends. You must typecast to a u_char pointer to 
    ensure the reuslt make it there correctly. pcap passes information in the form of a u_char pointer.
  * pcap_dispatch* is almost identical in usage. The only difference between pcap_dispatch() and 
    pcap_loop() is that pcap_dispatch() will only process the first batch of packets that it receives 
    from the system, while pcap_loop() will continute processing packets or atches of packets until 
    the count of pacekts runs out

// The prototype for a callback function
- void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
  0. No return since it's void (because pcap_loop() wouldn't know how to handle a return value anyways)
  1. u_char *args: it corresponds to the last argument of pcap_loop(). Whatever value is passed as the
                   last argument to pcap_loop() is passed to the first argument of your callback function 
                   every time the function is called
  2. const struct pcap_pkthdr *header: the pcap header, which contains information about when the 
                                       packet was sniffed, how large it is, etc
     // pacp_pkthdr strcuture is defined as
       struct pcap_pkthdr{
        struct timeval ts;    // time stamp
        bpf_u_int32 caplen;   // length of portion present
        bpf_u_int32 len;      // length this apcket (off wire)
       };
  3. const u_char *packet: it points to the first byte of a chunk of data containing entire packet, 
                           as sniffed by pcap_loop()
  * How do you make use of this variable? ("packet")
    A packet contains many attributes, it is not really string, but actually a collection of structures
    (for instance, a TCP/IP packet would have an Ethernet header, an IP header, a TCP header, and lastly,
    the packet's payload). This u_char pointer points to the serialized version of these structures.
    To make any use of it, you must do some interesting typecasting
     
/*
You must havfe the actual structues define before you can typecast to them. The following are the 
structure definitions that Tim Carstens use to describe a TCP/IP packet over Ethernet
*/

// Ethernet address are 6 bytes
#define ETHER_ADDR_LEN 6

// Ethernet header
struct sniff_ethernet {
  u_char ether_dhost[ETHER_ADDR_LEN]; // Destination host address
  u_char ether_shost[ETHER_ADDR_LEN]; // Source host address
  u_short ether_type; // IP? ARP? RARP? etc
};

// IP header
struct sniiff_ip{
  u_char ip_vhl;  // version << 4 | header length >> 2
  u_char ip_tos;  // type of service
  u_short ip_len; // total length
  u_short ip_id;  // identification
  u_short ip_off; // fragment offset field
#define IP_RF 0x8000  // reserved fragment flag
#define IP_DF 0x4000  // dont fragment flag
#define IP_MF 0x2000  // more fragments flag
#define IP_OFFMASK 0x1fff // mask for fragmenting bits
  u_char ip_ttl;  // time to live
  u_char ip_p;    // protocol
  u_short ip_sum; // checksum
  struct in_addr ip_src, ip_dst;  // source and dest address
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
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN | TH_SYN | TH_RST | TH_ACK | TH_URG |TH_ECE | TH_CWR)
        u_short th_win; // window
        u_short th_sum; // checksum
        u_short th_urp; // urgent pointer
};


}
```
