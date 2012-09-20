@echo off
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v BSDCONV_PATH /t REG_SZ /d %~dp0 /f
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d %PATH%;%~dp0 /f
pause