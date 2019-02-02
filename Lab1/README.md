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
  
  // pcap_lookupdev() returns a pointer to a string giving the name of a network device
  // If there is an error, NULL is returned and errbuf is filled in with an appropriate error message
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

handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
if (handle == NULL) {
  fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuff);
  return(2);
}
```
