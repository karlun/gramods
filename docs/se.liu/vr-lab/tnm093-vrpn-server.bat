:: Script for running VRPN server for TNM093
:: 

@echo off

del /s /q "%USERPROFILE%\AppData\Local\openvr"
robocopy "C:\Apps\steamvr_config\AppData_Local_openvr\openvr" "%USERPROFILE%\AppData\Local\openvr" /E /IS /IT

cd /D %USERPROFILE%\AppData\Local\openvr
for /r %%x in (*) do copy /b "%%x"+,, "%%x"


del /s /q "C:\Program Files (x86)\Steam\config\lighthouse"
robocopy "C:\Apps\steamvr_config\Steam_config_lighthouse\lighthouse" "C:\Program Files (x86)\Steam\config\lighthouse" /E /IS /IT

cd /D "C:\Program Files (x86)\Steam\config\lighthouse"
for /r %%x in (*) do copy /b "%%x"+,, "%%x"


del /s /q "C:\Program Files (x86)\Steam\config\vrappconfig"
robocopy "C:\Apps\steamvr_config\Steam_config_vrappconfig\vrappconfig" "C:\Program Files (x86)\Steam\config\vrappconfig" /E /IS /IT

cd /D "C:\Program Files (x86)\Steam\config\vrappconfig"
for /r %%x in (*) do copy /b "%%x"+,, "%%x"


START /B CMD /C CALL "C:\Program Files (x86)\Steam\steamapps\common\SteamVR\bin\win64\vrstartup.exe"

timeout /t 5 /nobreak

set SCRIPT_PATH=%~dp0
call %SCRIPT_PATH%\setenv.bat

gm-load ^
  --config urn:gramods:config/output-normal-to-console.xml ^
  --config urn:gramods:config/se.liu/vr-lab.tracker-server-openvr.xml
