#ifndef __YOLSPC_CLIENT_UTILS__
#define __YOLSPC_CLIENT_UTILS__

#include <sstream>
#include <stdint.h>
#include <thread>

namespace yolspc {

//获取线程id
uint32_t GetThreadId();

//获取协程id
uint32_t GetFiberId();

//获得机器字节序,若为true则为小端序
bool GetEndian();

//大小端序转换
uint32_t LittleToBig(uint32_t data);
uint32_t BigToLittle(uint32_t data);

}   // namespace yolspc

#endif   //__YOLSHIP_CLIENT_UTILS__