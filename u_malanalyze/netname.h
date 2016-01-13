#pragma once

#define N	61

extern char netbiosmap[N][3] ;

int 
PCAP_Analysis() ;

void 
processstring
			(char *str, 
			int length, 
			char nbns);
