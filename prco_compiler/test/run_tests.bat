@ECHO OFF
setlocal enabledelayedexpansion
setlocal

set /A RESULT = 0
set /A ANSWER = 0
set /A NUM_TESTS = 0

dir
dir ../

echo "Doing wbuild..."
cd ..
mkdir wbuild
cd wbuild
cmake .. -G "MinGW Makefiles"
mingw32-make
if %errorlevel% NEQ 0 (
	echo Error during make. Stopping...
	goto :eof
)
cd ..

rem call :build_std

echo "Running Tests..."
cd test
call :run_test tests/binary_ops_1.prco 6
call :run_test tests/foo.prco 6

echo %NUM_TESTS% ran.
echo %RESULT%/%NUM_TESTS% tests failed.
rem del out.s
rem del a.exe
goto :eof

:build_std
cd libccl/std
gcc -c stdio.c -o stdio.o
echo Built stdio.o
cd ../..
EXIT /B %ERRORLEVEL%

:run_test
set /A NUM_TESTS=NUM_TESTS+1
call "../wbuild/cli/cli.exe" -i %~1 -d -D0xFF
if !errorlevel! EQU 0 (
	rem Compiler successfullly created assembly file
	rem Compiler it with gcc as...
	gcc -m32 -O0 out.s
	a.exe
	set ANSWER=!errorlevel!
) else (
	rem Tests can also test failure conditions.
	rem In this case, set the answer to the compiler
	rem return code, not the executables.
	echo Compiler exited with error code !errorlevel!
	set ANSWER=!errorlevel!
)

if %ANSWER% EQU %~2 (
	echo PASS %~1	Got %ANSWER%	Expected %~2
) else (
	echo FAIL %~1	Got %ANSWER%	Expected %~2
	set /A RESULT=!RESULT!+1
)
EXIT /B !ERRORLEVEL!