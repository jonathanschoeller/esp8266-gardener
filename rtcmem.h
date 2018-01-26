#ifndef __RTCMEM_H__
#define __RTCMEM_H__

#include "os_type.h"

void rtc_write(const void *src_addr, uint16 save_size);
bool rtc_read(void *des_addr, uint16 load_size);

#endif
