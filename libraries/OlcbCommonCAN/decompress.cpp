//
// "Memory-efficient decompression for embedded computers"
//
// See http://excamera.com/sphinx/article-compression.html 
// for more info
//

#include "decompress.h"

class Flashbits {
public:
  void begin(prog_uchar *s) {
    src = s;
    mask =0x01;
  }
  byte get1(void) {
    byte r = (pgm_read_byte_near(src) & mask) !=0;
    mask <<=1;
    if (!mask) {
      mask =1;
      src++;
    }
    return r;
  }
  unsigned short getn(byte n) {
    unsigned short r =0;
    while (n--) {
      r <<=1;
      r |= get1();
    }
    return r;
  }
private:
  prog_uchar *src;
  byte mask;
};

static Flashbits BS;

static void decompress(byte *dst, byte *src)
{
  BS.begin(src);
  byte O = GDFB.getn(4);
  byte L = GDFB.getn(4);
  byte M = GDFB.getn(2);
  byte *end = dst + BS.getn(16);
  while (dst != end) {
    if (BS.get1() ==0) {
      *dst++= BS.getn(8);
    } else {
      int offset =-BS.getn(O) -1;
      int length = BS.getn(L) + minlen;
      while (length--) {
        *dst =*(dst + offset);
        dst++;
      }
    }
  }
}

