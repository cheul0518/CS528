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

I dig "www.example.com" on the user VM in order to verify Kaminsky DNS attack is indeed successful. However it turns out that the attack is incomplete. Look at the picture below: 

<img src = "images/11.png" width = "700">

This is because the DNS server finds out that the domain name "dnslabattacker.net" doesn't exist, and marks the NS entry invalid. 

<br />
<br />
<br />

<img src = "images/1.png" width = "700">

<img src = "images/2.png" width = "700">

<img src = "images/3.png" width = "700">
