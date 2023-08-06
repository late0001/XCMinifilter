@echo off
set curdir=%~dp0
echo 1.∞≤◊∞      2.–∂‘ÿ
set /p a=«Î ‰»Î: 
if %a% == 1 (
	echo "Installing ..."
RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection DefaultInstall 132 %curdir%\MyMinifilter.inf
) else (
	echo "Uninstalling ..."
RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection DefaultUninstall 132 %curdir%\MyMinifilter.inf
)
pause