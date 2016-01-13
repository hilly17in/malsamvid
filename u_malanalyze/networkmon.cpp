
# include "stdafx.h"


pcap_if_t *alldevs;
pcap_if_t *d;
int choice_interface;


int findindatapid(int pid)
{
	unsigned int cnt = 0;

	while(cnt < dataPid.size())
	{
		if(pid == dataPid[cnt])
			return 1;

		++cnt;
	}

	return 0;
}

DWORD
GetTcpConnections()
{
	DWORD	status = 0 ;
	DWORD	dwSize  = 0 ;
	PMIB_TCPTABLE_OWNER_PID	pTcpTable = NULL;
	DWORD	dwRet;
	
	dwRet = GetExtendedTcpTable(pTcpTable, 
						&dwSize, 
						FALSE, 
						AF_INET, 
						TCP_TABLE_OWNER_PID_ALL, 
						0);
	switch(dwRet)
	{
	case NO_ERROR:
		//proceed with analysis
		break;

	case ERROR_INSUFFICIENT_BUFFER:

		pTcpTable = (PMIB_TCPTABLE_OWNER_PID)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
		if(pTcpTable == NULL)
		{
			printf("memory allocation for tcp table has failed. \n");		
			return dwRet ;
		}

		dwRet = GetExtendedTcpTable(pTcpTable, 
							&dwSize, 
							FALSE, 
							AF_INET, 
							TCP_TABLE_OWNER_PID_ALL, 
							0);
	
		//printf("ret = %d \n", dwRet);
		if(dwRet != NO_ERROR)
		{
			//printf("An Error has Occured in GetExtendedTable : %d\n", GetLastError());
			return dwRet;
		}

		break;

	default:

		//printf("An Error has Occured in GetExtendedTable : %d\n", GetLastError());
		return dwRet;
	}
	
	DWORD cnt = 0;
	WCHAR tmp[MAX_PROGRAM_LEN];
	char l_str[14], r_str[14];
	in_addr l_addr, r_addr;
	
	for(cnt=0; cnt<pTcpTable->dwNumEntries;cnt++)
	{
		if(findindatapid(pTcpTable->table[cnt].dwOwningPid) )
		{
			l_addr.S_un.S_addr = pTcpTable->table[cnt].dwLocalAddr ;
			//strcpy(l_str, inet_ntoa(l_addr));

			StringCbCopyA(l_str, 14, inet_ntoa(l_addr));
			
			if(pTcpTable->table[cnt].dwState == MIB_TCP_STATE_LISTEN) {
				StringCbPrintf(tmp, MAX_PROGRAM_BYTES,
							   L"process %d found in listen state on TCP: %S:%u\n", 
							   pTcpTable->table[cnt].dwOwningPid, l_str, ntohs(pTcpTable->table[cnt].dwLocalPort));
			}
			else {

				r_addr.S_un.S_addr = pTcpTable->table[cnt].dwRemoteAddr ;
				StringCbCopyA(r_str, 14, inet_ntoa(r_addr));

				StringCbPrintf(tmp, MAX_PROGRAM_BYTES, L"Process %d -- %s:%u <-------> %s:%u found in state %d\n", pTcpTable->table[cnt].dwOwningPid, l_str, ntohs(pTcpTable->table[cnt].dwLocalPort),
					r_str, ntohs(pTcpTable->table[cnt].dwRemotePort), pTcpTable->table[cnt].dwState);
				}

			tc.replace(tmp);
			dataTcpcon.push_back(tc);
		}
	}	
 
	VirtualFree(pTcpTable, 0, MEM_DECOMMIT);
	return status;

}

DWORD 
GetUdpConnections()
{
	DWORD	status = 0 ;
	DWORD	dwSize  = 0 ;
	PMIB_UDPTABLE_OWNER_PID	pUdpTable = NULL;
	DWORD	dwRet;
	
	dwRet = GetExtendedUdpTable(pUdpTable, 
						&dwSize, 
						FALSE, 
						AF_INET, 
						UDP_TABLE_OWNER_PID, 
						0);
	switch(dwRet)
	{
	case NO_ERROR:
		//proceed with analysis
		break;

	case ERROR_INSUFFICIENT_BUFFER:

		pUdpTable = (PMIB_UDPTABLE_OWNER_PID)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
		if(pUdpTable == NULL)
		{
			printf("memory allocation for udp table has failed. \n");		
			return dwRet ;
		}

		dwRet = GetExtendedUdpTable(pUdpTable, 
							&dwSize, 
							FALSE, 
							AF_INET, 
							UDP_TABLE_OWNER_PID, 
							0);
	
		//printf("ret = %d \n", dwRet);
		if(dwRet != NO_ERROR)
		{
			//printf("An Error has Occured in GetExtendedTable : %d\n", GetLastError());
			return dwRet;
		}

		break;

	default:

		//printf("An Error has Occured in GetExtendedTable : %d\n", GetLastError());
		return dwRet;
	}
	
	DWORD cnt = 0;
	WCHAR tmp[100];
	char l_str[14];
	in_addr l_addr;
	
	for(cnt=0; cnt<pUdpTable->dwNumEntries;cnt++)
	{
		if(findindatapid(pUdpTable->table[cnt].dwOwningPid))
		{
			l_addr.S_un.S_addr = pUdpTable->table[cnt].dwLocalAddr ;
			StringCbCopyA(l_str, 14, inet_ntoa(l_addr));
			StringCbPrintf(tmp, MAX_PROGRAM_BYTES, 
				L"process %d found as on UDP: %S:%u\n", 
				pUdpTable->table[cnt].dwOwningPid, l_str, ntohs(pUdpTable->table[cnt].dwLocalPort));

			tc.replace(tmp);
			dataTcpcon.push_back(tc);
		}
	}	
 
	VirtualFree(pUdpTable, 0, MEM_DECOMMIT);
	
	return status;

}

DWORD
WINAPI
NetworkPortMonitorThread(LPVOID lParam)
{
	DWORD	status = SUCCESS ;

	while(1)
	{
		if(ERROR_INSUFFICIENT_BUFFER == GetTcpConnections())
			ExitThread(0);

		if(ERROR_INSUFFICIENT_BUFFER == GetUdpConnections())
			ExitThread(0);

			Sleep(1);
	}
	
	return status;
}

DWORD
WINAPI
CapturePackets(LPVOID lParam)
{
	pcap_t			*adhandle;
	char			errbuf[PCAP_ERRBUF_SIZE];
	pcap_dumper_t	*dumpfile;
	int				i = 0;
	
	
		
	/* Jump to the selected adapter */
    for(d=alldevs, i=0; i< choice_interface-1 ;d=d->next, i++);
    
    
	/* Open the adapter */
	if ((adhandle= pcap_open_live(d->name,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	StringCbPrintfA(filename_pktdump, MAX_PATH-1, "%ws\\dump_%ws.pcap", dirpath, malwarename);
	
	/* Open the dump file */
	dumpfile = pcap_dump_open(adhandle, filename_pktdump);

	if(dumpfile==NULL)
	{
		fprintf(stderr,"\nError opening output file\n");
		return -1;
	}
    
    //printf("\nlistening on %s... Press Ctrl+C to stop...\n", d->description);
	
    /* At this point, we no longer need the device list. Free it */
    pcap_freealldevs(alldevs);
    
    /* start the capture */
    pcap_loop(adhandle, 0, packet_handler, (unsigned char *)dumpfile);

    pcap_close(adhandle);
    return 0;
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *dumpfile, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	/* save the packet on the dump file */
	pcap_dump(dumpfile, header, pkt_data);
}


DWORD
SetupPacketCapture()
{
	DWORD	status = SUCCESS;
	char	errbuf[PCAP_ERRBUF_SIZE];
	int		i=0;
	int		inum=0;
	//DWORD	dwThreadId;

	/* Retrieve the device list on the local machine */
	if (pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		status = ERROR_FAILURE;
		goto complete;
	}
    
    /* Print the list */
    for(d=alldevs; d; d=d->next)
    {
        printf("%d. %s", ++i, d->name);
        if (d->description)
            printf(" (%s)\n", d->description);
        else
            printf(" (No description available)\n");
    }

    if(i==0)
    {
        printf("\nNo interfaces found! Make sure Networking is installed.\n");
		status = ERROR_FAILURE;
		goto complete;
    }
    
    printf("Enter the interface number (1-%d):",i);
	if(i == 1)
		inum = 1;
	else
		scanf_s("%d", &inum);

    if(inum < 1 || inum > i)
    {
        printf("\nInterface number out of range.\n");
        /* Free the device list */
        pcap_freealldevs(alldevs);
        status = ERROR_FAILURE;
		goto complete;
    }

	choice_interface = inum ;

complete:
	if(status == ERROR_FAILURE)
		printf("\n packet capture will not be available \n");

	return status;
}