REM get stuff!
if not exist download mkdir download
if not exist install mkdir install
if not exist install\windows mkdir install\windows

set eth_server=https://build.ethereum.org/builds/windows-precompiled

call :download boost 1.55.0
call :download cryptopp 5.6.2
call :download curl 7.4.2
call :download jsoncpp 1.6.2
call :download json-rpc-cpp 0.5.0
call :download leveldb 1.18
call :download llvm 3.7.0
call :download microhttpd 0.9.2
call :download OpenCL_ICD 1
call :download qt 5.4.1
call :download miniupnpc 1.9

goto :EOF

:download

set eth_name=%1
set eth_version=%2

cd download

if not exist %eth_name%-%eth_version%-x64.tar.gz (
	for /f "tokens=2 delims={}" %%g in ('bitsadmin /create %eth_name%-%eth_version%-x64.tar.gz') do (
		bitsadmin /transfer {%%g} /download /priority normal %eth_server%/%eth_name%-%eth_version%-x64.tar.gz %cd%\%eth_name%-%eth_version%-x64.tar.gz
		bitsadmin /cancel {%%g}
	)
)
if not exist %eth_name%-%eth_version% cmake -E tar -zxvf %eth_name%-%eth_version%-x64.tar.gz
cmake -E copy_directory %eth_name%-%eth_version% ..\install\windows

cd ..

goto :EOF
