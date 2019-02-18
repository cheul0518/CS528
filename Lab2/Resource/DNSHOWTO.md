## **The following content belongs to Nicolai Langfeldt (dns-howto(at)langfeldt.net), Jamie Norrish and others (http://www.tldp.org/HOWTO/DNS-HOWTO.html)**


## 1. Preamble

Keywords: DNS, BIND, BIND 4, BIND 8, BIND 9, named, dialup, PPP, slip, ISDN, Internet, domain, name, resolution, hosts, caching.

This document is part of the Linux Documentation Project.

### 1.1 Legal stuff

(C)opyright 1995-2001 Nicolai Langfeldt, Jamie Norrish & Co. Do not modify without amending copyright, distribute freely but retain copyright message.

### 1.2 Credits and request for help.

I want to thank all the people that I have bothered with reading this HOWTO (you know who you are) and all the readers that have e-mailed suggestions and notes.

This will never be a finished document; please send me mail about your problems and successes. You can help make this a better HOWTO. So please send comments and/or questions or money to janl(at)langfeldt.net. Or buy my DNS book (it's titled "The Concise Guide to DNS and BIND, the bibliography has ISBNs). If you send e-mail and want an answer please show the simple courtesy of making sure that the return address is correct and working. Also, please read the qanda section before mailing me. Another thing, I can only understand Norwegian and English.

This is a HOWTO. I have maintained it as part of the LDP since 1995. I have, during 2000, written a book on the same subject. I want to say that, though this HOWTO is in many ways much like the book it is not a watered down version concocted to market the book. The readers of this HOWTO have helped me understand what is difficult to understand about DNS. This has helped the book, but the book has also helped me to think more about what this HOWTO needs. The HOWTO begot the book. The book begot version 3 of this HOWTO. My thanks to the book publisher, Que, that took a chance on me :-)

### 1.3 Dedication

This HOWTO is dedicated to Anne Line Norheim Langfeldt. Though she will probably never read it since she's not that kind of girl.

### 1.4 Updated versions

You should be able to find updated versions of this HOWTO both at http://langfeldt.net/DNS-HOWTO/ and on http://www.linuxdoc.org/. Go there if this document is dated more than 9 months ago.

<br />

## 2. Introduction.


**What this is and isn't.

DNS is the Domain Name System. DNS converts machine names to the IP addresses that all machines on the net have. It translates (or "maps" as the jargon would have it) from name to address and from address to name, and some other things. This HOWTO documents how to define such mappings using Unix system, with a few things specific to Linux.

A mapping is simply an association between two things, in this case a machine name, like ftp.linux.org, and the machine's IP number (or address) 199.249.150.4. DNS also contains mappings the other way, from the IP number to the machine name; this is called a "reverse mapping".

DNS is, to the uninitiated (you ;-), one of the more opaque areas of network administration. Fortunately DNS isn't really that hard. This HOWTO will try to make a few things clearer. It describes how to set up a simple DNS name server, starting with a caching only server and going on to setting up a primary DNS server for a domain. For more complex setups you can check the qanda section of this document. If it's not described there you will need to read the Real Documentation. I'll get back to what this Real Documentation consists of in the last chapter.

Before you start on this you should configure your machine so that you can telnet in and out of it, and successfully make all kinds of connections to the net, and you should especially be able to do telnet 127.0.0.1 and get your own machine (test it now!). You also need good /etc/nsswitch.conf, /etc/resolv.conf and /etc/hosts files as a starting point, since I will not explain their function here. If you don't already have all this set up and working the Networking-HOWTO and/or the Networking-Overview-HOWTO explains how to set it up. Read them.

When I say `your machine' I mean the machine you are trying to set up DNS on, not any other machine you might have that's involved in your networking effort.

I assume you're not behind any kind of firewall that blocks name queries. If you are you will need a special configuration --- see the section on qanda.

Name serving on Unix is done by a program called named. This is a part of the ``BIND'' package which is coordinated by The Internet Software Consortium. Named is included in most Linux distributions and is usually installed as /usr/sbin/named, usually from a package called BIND, in upper or lower case depending on the whim of the packager.

If you have a named you can probably use it; if you don't have one you can get a binary off a Linux ftp site, or get the latest and greatest source from ftp://ftp.isc.org/isc/bind9/. This HOWTO is about BIND version 9. The old versions of the HOWTO, about BIND 4 and 8, is still available at http://langfeldt.net/DNS-HOWTO/ in case you use BIND 4 or 8 (incidentally, you will find this HOWTO there too). If the named man page talks about (at the very end, in the FILES section) named.conf you have BIND 8; if it talks about named.boot you have BIND 4. If you have 4 and are security conscious you really ought to upgrade to the latest version of BIND 8. Now.

DNS is a net-wide database. Take care about what you put into it. If you put junk into it, you, and others, will get junk out of it. Keep your DNS tidy and consistent and you will get good service from it. Learn to use it, admin it, debug it and you will be another good admin keeping the net from falling to its knees by mismanagement.

Tip: Make backup copies of all the files I instruct you to change if you already have them, so that if after going through this nothing works you can get it back to your old, working state.

### 2.1 Other nameserver implementations.

This section was written by Joost van Baal.

Various packages exist for getting a DNS server on your box. There is the BIND package ( http://www.isc.org/products/BIND/); the implementation this HOWTO is about. It's the most popular nameserver around and it's used on the vast majority of name serving machines on the Internet, around and being deployed since the 1980's. It's available under a BSD license. Since it's the most popular package, loads of documentation and knowledge about BIND is around. However, there have been security problems with BIND.

Then there is djbdns ( http://djbdns.org/), a relatively new DNS package written by Daniel J. Bernstein, who also wrote qmail. It's a very modular suite: various small programs take care of the different jobs a nameserver is supposed to handle. It's designed with security in mind. It uses a simpler zone-file format, and is generally easier to configure. However, since it's less well known, your local guru might not be able to help you with this. Unfortunately, this software is not Open Source. The author's advertisement is on http://cr.yp.to/djbdns/ad.html.

Whether DJBs software is really an improvement over the older alternatives is a subject of much debate. A discussion (or is it a flame-war?) of BIND vs djbdns, joined by ISC people, is on http://www.isc.org/ml-archives/bind-users/2000/08/msg01075.html

<br />

## 3. A resolving, caching name server.

A first stab at DNS config, very useful for dialup, cable-modem, ADSL and similar users.

On Red Hat and Red Hat related distributions you can achieve the same practical result as this HOWTO's first section by installing the packages bind, bind-utils and caching-nameserver. If you use Debian simply install bind (or bind9, as of this writing, BIND 9 is not supported by Debian Stable (potato)) and bind-doc. Of course just installing those packages won't teach you as much as reading this HOWTO. So install the packages, and then read along verifying the files they installed.

A caching only name server will find the answer to name queries and remember the answer the next time you need it. This will shorten the waiting time the next time significantly, especially if you're on a slow connection.

First you need a file called /etc/named.conf (Debian: /etc/bind/named.conf). This is read when named starts. For now it should simply contain:

```c
// Config file for caching only name server
//
// The version of the HOWTO you read may contain leading spaces
// (spaces in front of the characters on these lines ) in this and
// other files.  You must remove them for things to work.
//
// Note that the filenames and directory names may differ, the
// ultimate contents of should be quite similar though.

options {
        directory "/var/named";

        // Uncommenting this might help if you have to go through a
        // firewall and things are not working out.  But you probably
        // need to talk to your firewall admin.

        // query-source port 53;
};

controls {
        inet 127.0.0.1 allow { localhost; } keys { rndc_key; };
};

key "rndc_key" {
        algorithm hmac-md5;
        secret "c3Ryb25nIGVub3VnaCBmb3IgYSBtYW4gYnV0IG1hZGUgZm9yIGEgd29tYW4K";
};

zone "." {
        type hint;
        file "root.hints";
};

zone "0.0.127.in-addr.arpa" {
        type master;
        file "pz/127.0.0";
};
```


The Linux distribution packages may use different file names for each kind of file mentioned here; they will still contain about the same things.

The `directory' line tells named where to look for files. All files named subsequently will be relative to this. Thus pz is a directory under /var/named, i.e., /var/named/pz. /var/named is the right directory according to the Linux File system Standard.

The file named /var/named/root.hints is named in this. /var/named/root.hints should contain this:

```c
;
; There might be opening comments here if you already have this file.
; If not don't worry.
;
; About any leading spaces in front of the lines here: remove them!
; Lines should start in a ;, . or character, not blanks.
;
.                       6D  IN      NS      A.ROOT-SERVERS.NET.
.                       6D  IN      NS      B.ROOT-SERVERS.NET.
.                       6D  IN      NS      C.ROOT-SERVERS.NET.
.                       6D  IN      NS      D.ROOT-SERVERS.NET.
.                       6D  IN      NS      E.ROOT-SERVERS.NET.
.                       6D  IN      NS      F.ROOT-SERVERS.NET.
.                       6D  IN      NS      G.ROOT-SERVERS.NET.
.                       6D  IN      NS      H.ROOT-SERVERS.NET.
.                       6D  IN      NS      I.ROOT-SERVERS.NET.
.                       6D  IN      NS      J.ROOT-SERVERS.NET.
.                       6D  IN      NS      K.ROOT-SERVERS.NET.
.                       6D  IN      NS      L.ROOT-SERVERS.NET.
.                       6D  IN      NS      M.ROOT-SERVERS.NET.
A.ROOT-SERVERS.NET.     6D  IN      A       198.41.0.4
B.ROOT-SERVERS.NET.     6D  IN      A       128.9.0.107
C.ROOT-SERVERS.NET.     6D  IN      A       192.33.4.12
D.ROOT-SERVERS.NET.     6D  IN      A       128.8.10.90
E.ROOT-SERVERS.NET.     6D  IN      A       192.203.230.10
F.ROOT-SERVERS.NET.     6D  IN      A       192.5.5.241
G.ROOT-SERVERS.NET.     6D  IN      A       192.112.36.4
H.ROOT-SERVERS.NET.     6D  IN      A       128.63.2.53
I.ROOT-SERVERS.NET.     6D  IN      A       192.36.148.17
J.ROOT-SERVERS.NET.     6D  IN      A       198.41.0.10
K.ROOT-SERVERS.NET.     6D  IN      A       193.0.14.129
L.ROOT-SERVERS.NET.     6D  IN      A       198.32.64.12
M.ROOT-SERVERS.NET.     6D  IN      A       202.12.27.33
```

The file describes the root name servers in the world. The servers change over time and must be maintained now and then. See the maintenance section for how to keep it up to date.

The next section in named.conf is the last zone. I will explain its use in a later chapter; for now just make this a file named 127.0.0 in the subdirectory pz: (Again, please remove leading spaces if you cut and paste this)

```c
$TTL 3D
@               IN      SOA     ns.linux.bogus. hostmaster.linux.bogus. (
                                1       ; Serial
                                8H      ; Refresh
                                2H      ; Retry
                                4W      ; Expire
                                1D)     ; Minimum TTL
                        NS      ns.linux.bogus.
1                       PTR     localhost.
```

The sections called key and controls together specify that your named can be remotely controlled by a program called rndc if it connects from the local host, and identifis itself with the encoded secret key. This key is like a password. For rndc to work you need /etc/rndc.conf to match this:

```c
key rndc_key {
    algorithm "hmac-md5";
    secret "c3Ryb25nIGVub3VnaCBmb3IgYSBtYW4gYnV0IG1hZGUgZm9yIGEgd29tYW4K";
};

options {
    default-server localhost;
    default-key    rndc_key;
};
```

As you see the secret is identical. If you want to use rndc from other machines their times need to be within 5 minutes of eachother. I recommend using the ntp (xntpd and ntpdate) software to do this.

Next, you need a /etc/resolv.conf looking something like this: (Again: Remove spaces!)

```c
search subdomain.your-domain.edu your-domain.edu
nameserver 127.0.0.1
```

The `search' line specifies what domains should be searched for any host names you want to connect to. The `nameserver' line specifies the address of your nameserver, in this case your own machine since that is where your named runs (127.0.0.1 is right, no matter if your machine has another address too). If you want to list several name servers put in one `nameserver' line for each. (Note: Named never reads this file, the resolver that uses named does. Note 2: In some resolv.conf files you find a line saying "domain". That's fine, but don't use both "search" and "domain", only one of them will work).

To illustrate what this file does: If a client tries to look up foo, then foo.subdomain.your-domain.edu is tried first, then foo.your-domain.edu, and finally foo. You may not want to put in too many domains in the search line, as it takes time to search them all.

The example assumes you belong in the domain subdomain.your-domain.edu; your machine, then, is probably called your-machine.subdomain.your-domain.edu. The search line should not contain your TLD (Top Level Domain, `edu' in this case). If you frequently need to connect to hosts in another domain you can add that domain to the search line like this: (Remember to remove the leading spaces, if any)

```c
search subdomain.your-domain.edu your-domain.edu other-domain.com
```

and so on. Obviously you need to put real domain names in instead. Please note the lack of periods at the end of the domain names. This is important; please note the lack of periods at the end of the domain names.


### 3.1 Starting named

After all this it's time to start named. If you're using a dialup connection connect first. Now run named, either by running the boot script: /etc/init.d/named start or named directly: /usr/sbin/named. If you have tried previous versions of BIND you're probably used to ndc. I BIND 9 it has been replaced with rndc, which can controll your named remotely, but it can't start named anymore. If you view your syslog message file (usually called /var/log/messages, Debian calls it /var/log/daemon, another directory to look is the other files /var/log) while starting named (do tail -f /var/log/messages) you should see something like:


(the lines ending in \ continues on the next line)

```c
Dec 23 02:21:12 lookfar named[11031]: starting BIND 9.1.3
Dec 23 02:21:12 lookfar named[11031]: using 1 CPU
Dec 23 02:21:12 lookfar named[11034]: loading configuration from \
    '/etc/named.conf'
Dec 23 02:21:12 lookfar named[11034]: the default for the \
    'auth-nxdomain' option is now 'no'
Dec 23 02:21:12 lookfar named[11034]: no IPv6 interfaces found
Dec 23 02:21:12 lookfar named[11034]: listening on IPv4 interface lo, \
    127.0.0.1#53
Dec 23 02:21:12 lookfar named[11034]: listening on IPv4 interface eth0, \
    10.0.0.129#53
Dec 23 02:21:12 lookfar named[11034]: command channel listening on \
    127.0.0.1#953
Dec 23 02:21:13 lookfar named[11034]: running
```

If there are any messages about errors then there is a mistake. Named will name the file it is reading. Go back and check the file. Start named over when it is fixed.

Now you can test your setup. Traditionally a program called nslookup is used for this. These days dig is recommended:

```c
 $ dig -x 127.0.0.1
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 26669
;; flags: qr aa rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 1, ADDITIONAL: 0

;; QUESTION SECTION:
;1.0.0.127.in-addr.arpa.                IN      PTR

;; ANSWER SECTION:
1.0.0.127.in-addr.arpa. 259200  IN      PTR     localhost.

;; AUTHORITY SECTION:
0.0.127.in-addr.arpa.   259200  IN      NS      ns.linux.bogus.

;; Query time: 3 msec
;; SERVER: 127.0.0.1#53(127.0.0.1)
;; WHEN: Sun Dec 23 02:26:17 2001
;; MSG SIZE  rcvd: 91
```

If that's what you get it's working. We hope. Anything very different, go back and check everything. Each time you change a file you need to run rndc reload.

Now you can enter a query. Try looking up some machine close to you. pat.uio.no is close to me, at the University of Oslo:

```c
 $ dig pat.uio.no
; <<>> DiG 9.1.3 <<>> pat.uio.no
;; global options:  printcmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 15574
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 3, ADDITIONAL: 0

;; QUESTION SECTION:
;pat.uio.no.                    IN      A

;; ANSWER SECTION:
pat.uio.no.             86400   IN      A       129.240.130.16

;; AUTHORITY SECTION:
uio.no.                 86400   IN      NS      nissen.uio.no.
uio.no.                 86400   IN      NS      nn.uninett.no.
uio.no.                 86400   IN      NS      ifi.uio.no.

;; Query time: 651 msec
;; SERVER: 127.0.0.1#53(127.0.0.1)
;; WHEN: Sun Dec 23 02:28:35 2001
;; MSG SIZE  rcvd: 108
```

This time dig asked your named to look for the machine pat.uio.no. It then contacted one of the name server machines named in your root.hints file, and asked its way from there. It might take tiny while before you get the result as it may need to search all the domains you named in /etc/resolv.conf.

If you ask the same again you get this:

```c
 $ dig pat.uio.no

; <<>> DiG 8.2 <<>> pat.uio.no 
;; res options: init recurs defnam dnsrch
;; got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 4
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 3, ADDITIONAL: 3
;; QUERY SECTION:
;;      pat.uio.no, type = A, class = IN

;; ANSWER SECTION:
pat.uio.no.             23h59m58s IN A  129.240.130.16

;; AUTHORITY SECTION:
UIO.NO.                 23h59m58s IN NS  nissen.UIO.NO.
UIO.NO.                 23h59m58s IN NS  ifi.UIO.NO.
UIO.NO.                 23h59m58s IN NS  nn.uninett.NO.

;; ADDITIONAL SECTION:
nissen.UIO.NO.          23h59m58s IN A  129.240.2.3
ifi.UIO.NO.             1d23h59m58s IN A  129.240.64.2
nn.uninett.NO.          1d23h59m58s IN A  158.38.0.181

;; Total query time: 4 msec
;; FROM: lookfar to SERVER: default -- 127.0.0.1
;; WHEN: Sat Dec 16 00:23:09 2000
;; MSG SIZE  sent: 28  rcvd: 162
```

As you can plainly see this time it was much faster, 4ms versus more than half a second earlier. The answer was cached. With cached answers there is the possibility that the answer is out of date, but the origin servers can control the time cached answers should be considered valid, so there is a high probability that the answer you get is valid.

### 3.2 Resolvers

All OSes implementing the standard C API has the calls gethostbyname and gethostbyaddr. These can get information from several different sources. Which sources it gets it from is configured in /etc/nsswitch.conf on Linux (and some other Unixes). This is a long file specifying from which file or database to get different kinds of data types. It usually contains helpful comments at the top, which you should consider reading. After that find the line starting with `hosts:'; it should read:

```c
hosts:      files dns
```

(You remembered about the leading spaces, right? I won't mention them again.)

If there is no line starting with `hosts:' then put in the one above. It says that programs should first look in the /etc/hosts file, then check DNS according to resolv.conf.

### 3.3 Congratulations

Now you know how to set up a caching named. Take a beer, milk, or whatever you prefer to celebrate it.

<br />

## 4. Forwarding

In large, well organized, academic or ISP (Internet Service Provider) networks you will sometimes find that the network people have set up a forwarder hierarchy of DNS servers which helps lighten the internal network load and the load on the outside servers as well. It's not easy to know if you're inside such a network or not. But by using the DNS server of your network provider as a "forwarder" you can make the responses to queries faster and less of a load on your network. This works by your nameserver forwarding queries to your ISP nameserver. Each time this happens you will dip into the big cache of your ISPs nameserver, thus speeding your queries up, your nameserver does not have to do all the work itself. If you use a modem this can be quite a win. For the sake of this example we assume that your network provider has two name servers they want you to use, with IP numbers 10.0.0.1 and 10.1.0.1. Then, in your named.conf file, inside the opening section called "options", insert these lines:

```c
           forward first;
           forwarders {
                10.0.0.1;
                10.1.0.1;
            };
```
There is also a nice trick for dialup machines using forwarders, it is described in the qanda section.

Restart your nameserver and test it with dig. Should still work fine.
