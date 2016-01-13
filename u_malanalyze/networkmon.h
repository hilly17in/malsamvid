# pragma once

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);


DWORD
WINAPI
NetworkPortMonitorThread(LPVOID lParam) ;

DWORD
WINAPI
CapturePackets(LPVOID lParam) ;

DWORD
SetupPacketCapture();
