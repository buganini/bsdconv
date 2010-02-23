@echo off
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v BSDCONV_PATH /t REG_SZ /d "%CD%" /f
