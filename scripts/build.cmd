:: This is pretty lame, it's just for my local development use.
:: In the github actions, I use a task to properly locate and load vcvars
:: so that it can invoke build.sh directly.
@echo off
SET here=%~dp0
pushd %here%..

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
if %errorlevel% NEQ 0 (
	exit /b %errorlevel%
)

"C:\Program Files\Git\bin\bash.exe" --login -i scripts/build.sh
if %errorlevel% NEQ 0 (
   exit /b %errorlevel%
)