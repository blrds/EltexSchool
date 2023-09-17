#include<stdio.h>	
#include<string.h>
#include<sys/socket.h>
#include<stdlib.h> 
#include<errno.h>
#include<netinet/udp.h>
#include<netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t udp_length;
};

int s;
char my_ip[32]="127.0.0.1";
struct sockaddr_in dest;
int dest_size;

unsigned short csum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}

void PrintData (unsigned char* data , int Size)
{
	int i,j;
	for(i=0 ; i < Size ; i++)
	{
		if( i!=0 && i%16==0)
		{
			for(j=i-16 ; j<i ; j++)
			{
				if(data[j]>=32 && data[j]<=128)
					printf("%c",(unsigned char)data[j]);
				
				else printf(".");
			}
		} 
		
		
				
		if( i==Size-1)
		{		
			for(j=i-i%16 ; j<=i ; j++)
			{
				if(data[j]>=32 && data[j]<=128) printf("%c",(unsigned char)data[j]);
				else printf(".");
			}
		}
	}
}

void recieve_udp(unsigned char *buffer , int Size)
{
	struct iphdr *iph = (struct iphdr*)buffer;
	if(iph->protocol!=17)return;

	unsigned short iphdrlen;
	
	iphdrlen = iph->ihl*4;
	
	struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen);
	//dest.sin_addr.s_addr=iph->daddr;
	printf("\n--client ip=%s--\n",inet_ntoa(dest.sin_addr));
	PrintData(buffer + iphdrlen + sizeof udph ,( Size - sizeof udph - iph->ihl * 4 ));

}

void response_udp()
{

//Datagram to represent the packet
	char datagram[4096], *data, *pseudogram;
	
	//zero out the packet buffer
	memset (datagram, 0, 4096);
	
	//IP header
	struct iphdr *iph = (struct iphdr *) datagram;
	
	//UDP header
	struct udphdr *udph = (struct udphdr *)(datagram + sizeof(struct ip));
	
	struct sockaddr_in sin;
	struct pseudo_header psh;
	
	//Data part
	data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
	scanf("%s", data);
	
	//some address resolution
	
	sin.sin_family = AF_INET;
	sin.sin_port = dest.sin_port;
	sin.sin_addr.s_addr = dest.sin_addr.s_addr;
	
	//Fill in the IP Header
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(data);
	iph->id = htonl(54321);	//Id of this packet
	iph->frag_off = 0;
	iph->ttl = 255;
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;		//Set to 0 before calculating checksum
	iph->saddr = inet_addr(my_ip);	//Spoof the source ip address
	iph->daddr = dest.sin_addr.s_addr;	
	//Ip checksum
	iph->check = csum ((unsigned short *) datagram, iph->tot_len);
	
	//UDP header
	udph->source = htons (5555);
	udph->dest = htons (dest.sin_port);
	udph->len = htons(8 + strlen(data));
	udph->check = 0;	//leave checksum 0 now, filled later by pseudo header
	
	//Now the UDP checksum using the pseudo header
	psh.source_address = inet_addr( my_ip );
	psh.dest_address = dest.sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_UDP;
	psh.udp_length = htons(sizeof(struct udphdr) + strlen(data) );
	
	int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + strlen(data);
	pseudogram = malloc(psize);
	
	memcpy(pseudogram, (char*) &psh , sizeof (struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + strlen(data));
	
	udph->check = csum( (unsigned short*) pseudogram , psize);
	
	{
		if (sendto(s, datagram, iph->tot_len , 0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
		{
			perror("sendto failed");
		}
		else
		{
			printf ("Packet Send. Length : %d \n" , iph->tot_len);
		}
	}
}

int main (void)
{
	int data_size;
	struct in_addr in;
	
	unsigned char *buffer = (unsigned char *)malloc(65536); 
	
	s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	
	if(s == -1)
	{
		//socket creation failed, may be because of non-root privileges
		perror("Failed to create raw socket");
		exit(1);
	}
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5555);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        perror(NULL);
        close(s);
        exit(1);
    }
	while(1){
		
		dest_size = sizeof dest;
		//Receive a packet
		data_size = recvfrom(s , buffer , 65536 , 0 , &dest , &dest_size);
		if(data_size <0 )
		{
			printf("Recvfrom error , failed to get packets\n");
			return 1;
		}
		recieve_udp(buffer, data_size);
		response_udp();
	}	
	return 0;
}
