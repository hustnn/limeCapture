#ifndef __DECPCAP_H
#define __DECPCAP_H

#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>

#include "lime_capture.h"

void dl_ethernet(u_char *user, const struct pcap_pkthdr *h, const u_char *p);

pcap_handler find_handler(int datalink_type, char *device);

void process_ip(const u_char *data, u_int32_t caplen,const struct pcap_pkthdr *h);

void process_tcp(const u_char *data, u_int32_t length, in_addr src,in_addr dst,const struct pcap_pkthdr *h);

#endif