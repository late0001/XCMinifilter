@echo off
set curdir=%~dp0
echo 1.��װ      2.ж��
set /p a=������: 
if %a% == 1 (
	echo "Installing ..."
RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection DefaultInstall 132 %curdir%\MyMinifilter.inf
) else (
	echo "Uninstalling ..."
RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection DefaultUninstall 132 %curdir%\MyMinifilter.inf
)
pause