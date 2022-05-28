#include "exception.h"

char exceptMSG[512] 	= "EXCEPTION OCCURED";

void setExceptionMessage(const char* msg){}
void setMiniDampName(const char* name){}
void setExceptionHandler(void (*pf)()){}
void setInteractiveExceptionHandler(bool interactiveEH) {}
