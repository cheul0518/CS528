### Summary: Programming with Pcap (Tim Carstens)

1. Setting the Device
- Determine which interface you want to sniff on.
- You can either define this device in a strong, or you can ask pcap to provide you with the name of an interface.
```c
// Specify the device by passing its name as the first argument to the program
#include <stdio.h>
#include <pcap.h>

int main(int argc, char *argv[]){
  char *dev = argv[1];
  
  printf("Device: %s\n", dev);
  return(0);
}
```

```c
// Pcap sets the device on its own
#include <stdio.h>
#include <pcap.h>

int main(int argc, char *argv[]){
  char *dev, errbuf[PCAP_ERRBUF_SIZE];
  /*
  pcap_lookupdev() returns a pointer to a string giving the name of a network device.
  If there is an error, NULL is returned and errbuf is filled in with an appropriate error message
  */
  dev = pcap_lookupdev(errbuf);
  if (dev == NULL){
    fprintf(stderr, "Couldn't find default device: $s\n", errbuf);
    return(2);
  }
  printf("Device: %s\n", dev);
  return(0);
}
```

<br />
2. Opening the device for sniffing
- Initialize pcap
- Name your sniffing "session" you can tell it apart from other such sessions

```c
#include <pcap.h>
  ...
  pcap_t *handle;
  /*
  - pcap_t *pcap_open_live(char *device, int snaplen, int promisc, int to_ms, char *ebuf)
  1. device is what you specified in the previous section
  2. snalpen is an integer which defines the maximum number of bytes to be captured by pcap
  3. pomisc, when set to true, brings the interface into promiscuous mode. 
    (In specific cases, promiscuous mode's forcefully on.)
  4. to_ms is the read time out in milliseconds (0 means no time out)
  5. ebuf is a string you can store any error messages within.
  6. Returns a session handler
  */
  handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
  if (handle == NULL) {
    fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuff);
    return(2);
  }
  /*
  In standard, non-promiscuous sniffing, a host is sniffing only traffic that is directly related to it. 
  Only traffic to, from, or routed through the host will be picked up by the sniffer.
  
  Promiscuous mode, on the other hand, sniffs all traffic on the wire. 
  In a non-switched environment, this could be all network traffic. 
  Promiscuous mode provides more packets for sniffing. 
  
  However it is detectable so a host can test with strong reliability determine if another host is doing promiscuous sniffing. 
  Second, it only works in a non-switched environment (such as a hub, or a switch that is being ARP flooded). 
  Third, on high traffic networks, the host can become quite taxed for system resources.
  */
```
- Not all devices provide the same type of link-layer headers in the pacekts. So you need to determine the type of link-layer headers the device provides, and use that type when processing the packet contents. If your program doesn't support the link-layer header type provided by the device, it has to give up
```c
// pcap_datalink() returns a value indicating the type of link-layer headers
if(pcap_datalink(handle) != DLT_EN10MB){
  fprintf(stderr, "Device %s doens't provide Ethernet headers - not supported\n", dev);
  return(2);
}
```

<br />
3. Filterting traffic

- This is a three phase process

- 1)Create a rule set (if you only want to sniff specific traffic. e.g.: only TCP/IP packets, only packets going to port 23, etc), 2)Compile it, and 3)Apply it

- Specifically the rule set is kept in a string, and is converted into a format that pcap can read. The compilation is actually just done by calling a function within your program; it doesn't involve the use of an external application. Then you tell pcap to apply it to whichever session you wish for it to filter
