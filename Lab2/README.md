## Lab2: Remote DNS Cache Poisoning Attack Lab (Due: Feb 28th)

### Introduction

This project provides first-hand experience on the remote DNS cache poisoning attack, or Kaminsky DNS attack. In the project, we're allowed to simultaneously run three virtual machines on a network. The three virtual machines play a role of DNS server, user, and attacker respectively. Kamingsky DNS attack is implemented as follows:

<img src = "images/fig3.png">
For my machine, DNS server is 198.165.15.18, user is 198.165.15.19, and attacker is 198.165.15.20.
 
<br />
<br />

### Task1: Remote Cache Poisoning


<img src = "images/dumpDotDB2.png" width ="700">

<img src = "images/dumpDotdb.png" width = "700">




<br />

### Task2: Result Verification

I dig "www.example.com" on the user VM in order to verify Kaminsky DNS attack is indeed successful. However it turns out the attack is incomplete. Look at the picture below: 

<img src = "images/11.png" width = "700">

When the DNS server receives the DNS query, it searches for example.com's NS record in its cache, and finds "ns.dnslabattacker.net". It will therefore send a DNS query to "ns.dnslabattacker.net". But, before sending the query, the server needs to know the IP address of "ns.dnslabattacker.net". A seperate DNS query is issued, and then the DNS server finds out the domain name "dnslabattacker.net" doesn't exist. The server shortly marks the NS entry invalid. 

You may say "Hey, include an additional record for the IP address for "ns.dnslabattacker.net" when forging the DNS response". Unfortunately, an IP address for the name server "ns.dnslabattacker.net" cannot be provided even if you actually attach it as an additional record in your spoofed response. Bailiwick checking plays a role of ignorning any records that are not within the same domain of the question. Hence "ns.dnslabattacker.net" cannot be answered in the additional section.


In this project, we can luckily handle this issue by simply adding some codes on both the Server and Attacker. Specifically, we store this IP address within resolver's name zone.

**1. Apollo (DNS server)**
- sudo su
- cd /etc/bind/
- nano -c named.conf.default-zones
- Add the following code:
```c
 zone "ns.dnslabattacker.net" {
                type master;
                file "/etc/bind/db.attacker";
};
```
- nano -c db.attacker
- Add the following code:
```c
$TTL 604800
@ IN SOA localhost. root.localhost. (
		2; Serial
		604800 ; Refresh
		86400 ; Retry
		2419200 ; Expire
		604800 ) ; Negative Cache TTL;
@ IN NS ns.dnslabattacker.net.
@ IN A 192.168.15.20
@ IN AAAA ::1
```

**2.DNS_attacker (attacker)**
- sudo su
- cd /etc/bind/
- nano -c named.conf.local
- Add the following entry:
```c
zone "example.com" {
	type master;
	file "/etc/bind/example.com.db";
};
```

<img src = "images/1.png" width = "700">

<img src = "images/2.png" width = "700">

<img src = "images/3.png" width = "700">
