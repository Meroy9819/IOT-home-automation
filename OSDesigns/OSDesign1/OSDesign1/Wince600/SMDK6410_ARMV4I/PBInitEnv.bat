@echo off

REM Initial environment configuration

set _PB_INSTALL_ROOT=C:\PROGRA~1\MIFAB2~1\6.00
set USING_PB_WORKSPACE_ENVIRONMENT=1
set _WINCEROOT=C:\WINCE600
set _FLATRELEASEDIR=D:\Projects\OSDesigns\OSDesign1\OSDesign1\RelDir\SMDK6410_ARMV4I_Debug
set LOCALE=0409
set _PROJECTROOT=D:\Projects\OSDesigns\OSDesign1\OSDesign1\Wince600\SMDK6410_ARMV4I

REM Workspace and configuration variables

set PBWORKSPACE=D:\Projects\OSDesigns\OSDesign1\OSDesign1\OSDesign1.pbxml
set PBWORKSPACEROOT=D:\Projects\OSDesigns\OSDesign1\OSDesign1
set PBCONFIG=SMDK6410 ARMV4I Debug

REM Call wince.bat

call C:\WINCE600\public\COMMON\OAK\MISC\wince.bat ARMV4I OSDesign1 SMDK6410

REM Make sure all build options are turned off

set IMGNODEBUGGER=
set IMGNOKITL=
set IMGEBOOT=
set IMGFLASH=
set IMGPROFILER=
set IMGCELOGENABLE=
set IMGAUTOFLUSH=
set IMGOSCAPTURE=
set BUILDREL_USE_COPY=
set WINCESHIP=
set IMGRAM64=
set IMGHDSTUB=

REM Anchored features

set SYSGEN_AUDIO=1
set SYSGEN_AUTH=1
set SYSGEN_AUTH_NTLM=1
set SYSGEN_AUTH_SCHANNEL=1
set SYSGEN_AUTORAS=1
set SYSGEN_AYGSHELL=1
set SYSGEN_BATTERY=1
set SYSGEN_CERTS=1
set SYSGEN_CONNMC=1
set SYSGEN_CRYPTO=1
set SYSGEN_CURSOR=1
set SYSGEN_EAP=1
set SYSGEN_FSREPLBIT=1
set SYSGEN_HELP=1
set SYSGEN_IABASE=1
set SYSGEN_IE_JSCRIPT=1
set SYSGEN_MENU_OVERLAP=1
set SYSGEN_MSIM=1
set SYSGEN_NOTIFY=1
set SYSGEN_PM=1
set SYSGEN_REDIR=1
set SYSGEN_SHELL=1
set SYSGEN_STANDARDSHELL=1
set SYSGEN_TOUCH=1
set SYSGEN_DOTNETV2=1
set SYSGEN_DOTNETV2_SUPPORT=1
set SYSGEN_CACHEFILT=1
set SYSGEN_AS_BASE=1
set SYSGEN_AS_FILE=1
set SYSGEN_IESAMPLE=1
set SYSGEN_QVGAP=1
set SYSGEN_LOCALAUDIO=1
set SYSGEN_STREAMAUDIO=1
set SYSGEN_PWORD=1
set SYSGEN_TCPIP6=1
set SYSGEN_MODEM=1
set SYSGEN_PPP=1
set SYSGEN_PPTP=1
set SYSGEN_ETHERNET=1
set SYSGEN_ETH_80211=1
set SYSGEN_BTH=1
set SYSGEN_BTH_AG=1
set SYSGEN_OBEX_SERVER=1
set SYSGEN_IRDA=1
set SYSGEN_OBEX_CLIENT=1

REM BSP features


REM Misc settings

set WINCEDEBUG=debug
set PATH=%PATH%;C:\Windows\system32;C:\Windows;C:\Program Files\Microsoft Platform Builder\6.00\cepb\IdeVS

REM Configuration environment variables


REM Build options

set IMGEBOOT=1
set IMGNODEBUGGER=
set IMGNOTALLKMODE=1
set ri_suppress_info=all
set BUILD_MULTIPROCESSOR=1

REM Project settings

set _USER_SYSGEN_BAT_FILES=D:\Projects\OSDesigns\OSDesign1\OSDesign1\Wince600\SMDK6410_ARMV4I\OAK\MISC\OSDesign1.bat

REM Locale options

set IMGNOLOC=0
set IMGSTRICTLOC=0
