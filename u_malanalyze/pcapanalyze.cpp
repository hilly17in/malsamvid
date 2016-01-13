# include "stdafx.h"

void
PushQueryToVector(char *str)
{
	int len;
    int slength = strlen(str) + 1;
    
	len = MultiByteToWideChar(CP_ACP, 0, str, slength, 0, 0); 
    
	wchar_t* wstr = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str, slength, wstr, len);
    std::wstring wq(wstr);
    
	gQuery.push_back(wq);

	delete[] wstr;
}


void 
processstring
			(char *str, 
			int length, 
			char nbns)
{
	int i=0, j=0, k=0;
	char *temp;

	// replace all 0x02 with a .
	if(nbns == 0) {

		for(; i<length; i++) {
			if( (str[i]==0x02) || (str[i]==0x04) || (str[i]==0x07) )
				str[i] = 46;
		} // for 
	} //if
	else if(nbns == 1){
		
		temp = (char *)malloc(length+1);		

		StringCbCopyNA(temp, length, str,  length);
		memset(str, 0, length);
		
		while( j < length-2 ) {
		
			i=0;
			while(i < N)
			{				
				if( (netbiosmap[i][0] == temp[j]) && (netbiosmap[i][1] == temp[j+1]))
					str[k++] = netbiosmap[i][2];
				
				i++;	// increment the index in the map
			} // while

			j += 2;
		} // while

		free(temp);
	} // else
} // function

int 
PCAP_Analysis()
{
	pcap_t *fp;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
 
	ether_header *eh ;
	ip_header *ih;
	udp_header *uh;
	u_char *data ;
	dns_header *dns;

	u_int pkt_num=0;
	int lenofquery, res;
	int datalen;
	int i=0, index=0, offset=0;

	char nbns=0;
	char *query;

	
	/* Open the capture file */
	if ((fp = pcap_open_offline(filename_pktdump,		// name of the device
						 errbuf							// error buffer
						 )) == NULL)
	{
		fprintf(stderr,"\nUnable to open the file %s.\n", filename_pktdump);
		return -1;
	}
	
	/* Retrieve the packets from the file */
	while((res = pcap_next_ex(fp, &header, &pkt_data)) >= 0)
	{
		 pkt_num++;

		 eh = (ether_header *) (pkt_data) ;
		 

         if (ETHERNET_IP == ntohs(eh->type))
		 {
			ih = (ip_header *)(pkt_data + 14);
			if(UDP == ih->proto)
			{
				uh = (udp_header *)(pkt_data + sizeof(ether_header) + sizeof(ip_header));
				datalen = ntohs(uh->len) - sizeof(udp_header);

				switch(ntohs(uh->dport))
				{
				case 137:
					
					nbns = 1;

				case 53:

					dns = (dns_header *)(pkt_data + sizeof(ether_header) + sizeof(ip_header) + sizeof(udp_header));
					data = (u_char *)(pkt_data + sizeof(ether_header) + sizeof(ip_header) + sizeof(udp_header) + sizeof(dns_header));
					dns->qdcount = ntohs(dns->qdcount);		
					
					if(dns->qdcount != 1) {
						printf("multiple query packet \n");
						break;
					}


					// now parse the dns query, assuming one per packet !
					lenofquery = ntohs(uh->len) - (sizeof(udp_header) + sizeof(dns_header) + 4 );
					offset = 0;
					query = (char *)malloc(lenofquery+1);
					
					StringCbCopyNA(query, lenofquery, (const char *)&data[offset+1], lenofquery);
					processstring(query, lenofquery, nbns);
					
					// save the string in vector
					PushQueryToVector(query);

					free(query);

				break;
											
				default:
					break;
		
				} // switch

			} // an udp packet 

		 } // an ip packet

	} // while
	

	if (res == -1)
	{
		printf("Error reading the packets: %s\n", pcap_geterr(fp));
	}
	
	pcap_close(fp);
	return 0;


}