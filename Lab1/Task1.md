### Task1: Writing a packet sniffing program
#### Task1.a: Understanding

1. Problem 1: Please use your own words to describe the sequence of the library calls that are essential for sniffer programs. This is meant to be a summary, not detailed explanation like the one in the tutorial.
	- pcap_lookupdev(): find a device on which to capture
	- pcap_lookupnet(): find the network number and netmask for the deivce
	- pcap_open_live(): open the deive for sniffing/capturing (obtain a packet capture handle to look at packets on the network)
	- pcap_datalink(): get the link-layer header type. For this program, make sure we're capturing an Ethernet device
	- pcap_compile(): compile a filter expression
	- pcap_setfilter(): apply/set the filter
	- pcap_loop(): process packets until the count of packets runs out
	- pcap_freecode(): free up allocated memory pointed by a bpf_program struct
	- pcap_close(): close the capture device

2. Problem 2: Why do you need the root privilege to run sniffex? Where does the program fail if executed without the root privilege?
	- Access models in Linux are implemented in the OS when it comes to accessing devices, files, and other sevices provided by the OS. pcap_lookupdev(), for example, requires low-level access to a network interface. Due to security implications such as capturing network traffic, generating arbitary packets etc, such access is limited to privileged users only. Thus, you cannot run the above code without the root privilege. 
 
```c
// The program fails to find the default device on which to capture if executed with no root privilege
dev = pcap_lookupdev(errbuf);
if (dev == NULL) {
	fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
	exit(EXIT_FAILURE);
}
```


3. Problem 3: Please turn on and turn off the promiscuous mode in the sniffer program. Can you demonstrate the difference when this mode is on and off? Please describe how you demonstrate this.
	- The sniffing program in the promiscuous mode sniffs all traffic on the wire while in the non-promiscuous mode the program sniffs only traffic directly related to it (that is, only traffic to, from, or routed through the host will be observed)
	- Promiscuous mode: pcap_open_live(dev, SNAP_LEN, 1, 1000, errbuf);
	- Non-promiscuous mode: handle = pcap_open_live(dev, SNAP_LEN, 0, 1000, errbuf);
	- If you'd like to run your sniff program in promiscuous mode, then set 1 to the third argument in "pcap_open_live". Otherwise 0 to it for non-promiscuous mode

- Here is an example showing the difference between sniffing in promisc mode and in non-promisc mode. The host IP is 192.168.15.16
- Below image is a result in Promiscuous mode. You can see traffic not related to the host such as a packet from 193.0.14.129 to 192.168.15.17
<img src = "images/part1/Promiscuous.png" width ="550">

- Below image is a result in Non-Promiscuous mode. You cannot see any traffic nothing to do with the host
<img src = "images/part1/Nonpromiscuous.png" width = "550">

<br />

#### Task1.b: Wriiting Filters

1. Capture the ICMP packets between two specific hosts
	- I manually set a filter expression in a sniff program as follows: char filter_exp[] = "icmp and (src host 192.168.15.17 and dst host google.com) or (src host google.com and dst host 192.168.15.17)";
	- I have two virtual machines run at a time. Let one VM ping google.com while another VM run the sniffing program.
	
- Below is the result of sniffing 192.168.15.17, which is communicating with google.com, at 192.168.15.16)
<img src = "images/part1/icmp.png">

2. Capture the TCP packets that have a destination port range from to port 50-100
	- I manually set a filter expression in a sniff program as follows: char filter_exp[] = "tcp dst portrange 50-100";
	- I have two virtual machines run at a time. Let one VM "sudo apt-get install curl" while another VM run the sniffing program.
	- The traffic is over HTTP which is assigned port 80 so that the sniffing program successfuly capture them all	

- Below is the result of capturing TCP packets with destination port 50 - 100
<img src = "images/part1/tcpport50_100.png">

<br />

#### Task1.c: Sniffing Passwords

- I set a filter expression in a sniff program as follows: char filter_exp[] = "tcp port 23";
- I increase num_packets by 70 in order to make sure the whole password's being captured during the sniffing time.
- I have two viirtual machine run at a time. Let one VM "telnet 192.168.15.16" while another VM run the sniffing program.
- Below is the result of caputring the password (Left is a VM sniffing, Right is a VM telnetting)
<img src = "images/part1/telnet.png">
<img src = "images/part1/password.png", width = 700>
