@echo off
set "src=..\Build\build-yaz.z64"
set "projectname=HeroOfLawFR"

for %%F in ("%src%") do set "mod=%%~tF"
for /f "tokens=1-2 delims= " %%a in ("%mod%") do set "d=%%a" & set "t=%%b"

:: Strip garbage from filename
set "d=%d:/=-%"
set "t=%t::=-%"
set "t=%t: =%"

set "ts=%d%_%t%"
usb64.exe -cp "%src%" "sd:TESTING\%projectname%-%ts%.z64"