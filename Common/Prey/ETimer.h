#pragma once
enum ITimer::ETimer : __int8
{
  ETIMER_REAL = 0x0,
  ETIMER_GAME = 0x1,
  ETIMER_UI = 0x2,
  ETIMER_PLAYER = 0x3,
  ETIMER_LAST = 0x4,
};