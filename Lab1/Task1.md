### Task1.a: Understanding

1. Problem 1

2. Problem 2: Why do you need the root privilege to run sniffex? Where does the program fail if executed without the root privilege?
 
```c
// The program fails to find the default device on which to capture if executed without no root privilege
dev = pcap_lookupdev(errbuf);
if (dev == NULL) {
	fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
	exit(EXIT_FAILURE);
}
```
- Access models in Linux are implemented in the OS when it comes to accessing devices, files, and other sevices provided by the OS. pcap_lookupdev(), for example, requires low-level access to a network interface. Due to security implications such as capturing network traffic, generating arbitary packets etc, such access is limited to privileged users only. Thus, you cannot run the above code without the root privilege.

3. Problem 3
