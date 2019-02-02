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
