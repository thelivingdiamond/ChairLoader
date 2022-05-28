@echo off
setlocal

set DEVENV2005="c:\Program Files (x86)\Microsoft Visual Studio 8\Common7\IDE\devenv.com"
for %%C in ( ^
Release^|Win32 ^
Release^|x64 ^
ReleaseStatic^|Win32 ^
ReleaseStatic^|x64 ^
Debug^|Win32 ^
Debug^|x64 ^
DebugStatic^|Win32 ^
DebugStatic^|x64 ^
 ) do ( %DEVENV2005% %~d0%~p0\yasli.sln /Rebuild %%C || ( exit /b -1 ) )

:vs2013
set DEVENV2013="c:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.com"
for %%C in ( ^
Release^|Win32 ^
Release^|x64 ^
ReleaseStatic^|Win32 ^
ReleaseStatic^|x64 ^
Debug^|Win32 ^
Debug^|x64 ^
DebugStatic^|Win32 ^
DebugStatic^|x64 ^
 ) do ( %DEVENV2013% %~d0%~p0\yasli2013.sln /Rebuild %%C || ( exit /b -1 ) )

endlocal
