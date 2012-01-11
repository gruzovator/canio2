@echo off
SET QMAKE=qmake.exe
SET MAKE=nmake.exe
SET QMAKE_OPTIONS="CONFIG+=release"
if "%1"=="debug" SET QMAKE_OPTIONS=""

SET DECOR=---------------------(
echo. & echo %DECOR% MAKE START & echo.
%QMAKE% %QMAKE_OPTIONS% -r -spec win32-msvc2008 
(%MAKE% && echo.) && echo %DECOR% MAKE OK || echo %DECOR% MAKE ERROR
echo.
echo %DECOR% CLEAN BEGIN
%MAKE% clean > NUL 2>NUL
echo %DECOR% CLEAN END 
