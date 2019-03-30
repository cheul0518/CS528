### Project Proposal


We have done Kaminsky attack in lab2. We make a target server such as www.example.com. Then we forge bogus DNS packets that
consist of random names within the target domain: 1234.example.com, something unlikely to be in cache in the victim nameserver.
We send a stream of bogus DNS packets to the victim,  but instead of A records as part of an answer, it instead delegates
to another nameserver via packet authority records. Once a query ID match, then the name server's cache is poisoned. 

As an extension of lab2, I would like to dig in DNS Cache poisoning/Kaminsky attack in real world. 
Specifically I would like to find out how secure current DNS name server system is against Cache poisoning attack. 
As far as I know, their source port allocation has been also randomised to prevent such an attack. Are they really secure? 
Are there any other security defense mechanism against DNS cache poisoning attack? If yes, I would like to discover what 
they are and their actual/potential vulnerability

Seconly, I would like to implement real-world level of DNS cache poisoning attack. In lab2, we were allowed to set up some 
parts as fixed such as port number or victim nameserver's configuration (using a fake domain)
