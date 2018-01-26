#include "rtcmem.h"

extern "C" {
  #include "user_interface.h" // this is for the RTC memory read/write functions
}

const uint32 marker = 1510163588;

void rtc_write(const void *src_addr, uint16 save_size)
{
  system_rtc_mem_write(65, &marker, sizeof marker);
  system_rtc_mem_write(69, src_addr, save_size);
}

bool rtc_read(void *des_addr, uint16 load_size)
{
  uint32 mark;
  system_rtc_mem_read(65, &mark, sizeof mark);
  if (mark == marker)
  {
    system_rtc_mem_read(69, des_addr, load_size);
    return true;
  }
  else
  {
    return false;
  }
}

