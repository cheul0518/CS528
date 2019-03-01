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

<br />
<br />
<br />

<img src = "images/1.png" width = "700">

<img src = "images/2.png" width = "700">

<img src = "images/3.png" width = "700">
