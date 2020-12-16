#include "EMA.h"

EMAUint16::EMAUint16(const uint16_t length, const uint16_t init){
    _length = length;
    _sum = uint32_t(length) * uint32_t(init);
}

uint16_t EMAUint16::update(const uint16_t value, const uint16_t length){
    if(length > _length){
        _sum = uint32_t(value * _length);
    }else{
        _sum = _sum - _sum * uint32_t(length) / _length  + uint32_t(value) * uint32_t(length);
    }
    return get();
}

EMAFloat::EMAFloat(const uint16_t length, const float init){
    _length = length;
    _ema = init;
}

float EMAFloat::update(const float value, const uint16_t length){
    if(length > _length){
        _ema = value;
    }else{
        _ema = _ema + ( value - _ema ) * length / _length;
    }
    // if(_ema > 1024 || _ema<0){
    //     _ema = value;
    // }
    return _ema;
}