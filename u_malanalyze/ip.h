/*

this file is ip.h

*/

#pragma pack(4)

# define ETHERNET_ARP	0x806
# define ETHERNET_IP	0x800

# define ICMP_REQUEST	8
# define ICMP_REPLY	0
# define ETHER_ADDR_LEN	6
# define ETHER_TYPE_LEN	2
# define ETHER_CRC_LEN	4
# define ETHER_HEADER_LEN	(2 * ETHER_ADDR_LEN) + (ETHER_TYPE_LEN)

# define SYN 2

# define IP_INIT_INFO	12
# define IP4_ADDR_LEN	4

# define ICMP	0x01
# define TCP	0x06
# define UDP	0x11
# define ANYP	0x00
typedef struct _ether_header
{
	u_char	src[ETHER_ADDR_LEN] ;
	u_char	dst[ETHER_ADDR_LEN] ;
	u_short	type ;
} ether_header ;


/* 4 bytes IP address */
typedef struct ip_address{
    u_char byte1;
    u_char byte2;
    u_char byte3;
    u_char byte4;
}ip_address;


/* IPv4 header */
typedef struct ip_header{
    u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
    u_char  tos;            // Type of service 
    u_short tlen;           // Total length 
    u_short identification; // Identification
    u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
    u_char  ttl;            // Time to live
    u_char  proto;          // Protocol
    u_short crc;            // Header checksum
    u_long  saddr;          // Source address
    u_long  daddr;          // Destination address
}ip_header;

/* UDP header*/
typedef struct udp_header{
    u_short sport;          // Source port
    u_short dport;          // Destination port
    u_short len;            // Datagram length
    u_short crc;            // Checksum
}udp_header;

typedef struct _icmp_header
{
	u_char	type ;			// 1
	u_char	code ;			// 1
	u_short	checksum ;		// 2
	u_short	identifier ;		// 2
	u_short	seq_num	;		// 2
} icmp_header;

typedef struct _tcp_header
{
	u_short	src_port ;	// 2
	u_short	dst_port ;	// 2
	u_long	seq_num ;	// 4
	u_long	ack_num ;	// 4
	u_char	hlen ;		// 1
	u_char	flags;		// 1
	u_short	win_size ;	// 2
	u_short	chk_sum ;	// 2
	u_short	urg_ptr ;	// 2

} tcp_header;

typedef struct _icmp_
{
	icmp_header ich ;
	char data[32];
} icmp ;// icmp_

typedef struct _dns_header
{
	u_short	trans_id;
	u_short	flag_ra:1,
			flag_z:3,
			flag_rcode:4,
			flag_qr:1,
			flag_opcode:4,
			flag_aa:1,
			flag_tc:1,
			flag_rd:1;	
	u_short qdcount;
	u_short ancount;
	u_short nscount;
	u_short arcount;
}dns_header;


/*		
	u_short	flag_rcode:4,
			flag_z:3,
			flag_ra:1,
			flag_rd:1,	
			flag_tc:1,
			flag_aa:1,
			flag_opcode:4,
			flag_qr:1;*/	
