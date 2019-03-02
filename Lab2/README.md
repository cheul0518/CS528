**1. Compile command:**

    - gcc -lpcap udp.c -o udp

**2. The program must be run as root:**

    - sudo ./udp [Attacker IP address] [Victim DNS server IP address]
    - e.g) sudo ./udp 192.168.15.20 192.168.15.18
