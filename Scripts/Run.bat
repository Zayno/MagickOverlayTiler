@echo off

rem The `/d` switch changes both the drive and directory.
cd /d "%~dp0"


start /affinity  20	 /high MagickTiler.exe 7
start /affinity  40	 /high MagickTiler.exe 8
start /affinity  80	 /high MagickTiler.exe 9
start /affinity  100	 /high MagickTiler.exe 10
start /affinity  200	 /high MagickTiler.exe 11
start /affinity  400	 /high MagickTiler.exe 12
start /affinity  800	 /high MagickTiler.exe 13
start /affinity  1000 /high MagickTiler.exe 14
start /affinity  2000 /high MagickTiler.exe 15
start /affinity  4000 /high MagickTiler.exe 16
start /affinity  8000 /high MagickTiler.exe 17
start /affinity  10000 /high MagickTiler.exe 18
start /affinity  20000 /high MagickTiler.exe 19
start /affinity  40000 /high MagickTiler.exe 20