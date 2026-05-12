@rem Copyright (c) 2024 Huawei Device Co., Ltd.
@rem Licensed under the Apache License, Version 2.0 (the "License");
@if "%DEBUG%" == "" @echo off
set DIRNAME=%~dp0
if "%DIRNAME%" == "" set DIRNAME=.
set APP_HOME=%DIRNAME%
for %%i in ("%APP_HOME%") do set APP_HOME=%%~fi
set WRAPPER_MODULE_PATH=%APP_HOME%\hvigor\hvigor-wrapper.js
set NODE_EXE=node.exe
if defined NODE_HOME goto findNodeFromNodeHome
%NODE_EXE% --version >NUL 2>&1
if "%ERRORLEVEL%" == "0" goto execute
echo ERROR: node.exe not found
goto fail
:findNodeFromNodeHome
set NODE_EXE_PATH=%NODE_HOME%\%NODE_EXE%
if exist "%NODE_EXE_PATH%" goto execute
goto fail
:execute
"%NODE_EXE%" "%WRAPPER_MODULE_PATH%" %*
if "%ERRORLEVEL%" == "0" goto end
:fail
exit /b 1
:end
