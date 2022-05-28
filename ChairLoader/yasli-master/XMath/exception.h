#pragma once

void setExceptionMessage(const char* msg);
void setMiniDampName(const char* name);
void setExceptionHandler(void (*pf)());
void setInteractiveExceptionHandler(bool interactiveEH);

