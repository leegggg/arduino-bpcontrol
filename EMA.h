
#ifndef EMA_H_
#define EMA_H_

#include "Arduino.h"

class EMAUint16
{
public:
  EMAUint16(void) {  };
  explicit EMAUint16(const uint16_t length, const uint16_t init);
  ~EMAUint16() { };
  void    clear(const uint16_t init);
  uint16_t  update(const uint16_t value, const uint16_t length);
  uint16_t   get() const { return _sum/_length; };
  uint32_t   getSum() const { return _sum; }
  uint16_t getLength() const { return _length; }


protected:
  uint16_t _length;
  uint32_t   _sum;
};

class EMAFloat
{
public:
  EMAFloat(void) {  };
  explicit EMAFloat(const uint16_t length, const float init);
  ~EMAFloat() { };
  void    clear(const float init);
  float  update(const float value, const uint16_t length);
  float   get() const { return _ema; };
  uint16_t getLength() const { return _length; }


protected:
  uint16_t _length;
  float   _ema;
};

#endif