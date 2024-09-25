@echo off
REM Define the src folder
SET src_folder=.\src

REM Define the include directories for cppcheck
SET CPPCHECK_INCLUDES= -I %src_folder%\config\default 

REM Define the cppcheck flags
SET CPPCHECK_FLAGS=--quiet --enable=all --error-exitcode=1 ^
    --inline-suppr ^
    --suppress=missingIncludeSystem ^
    --suppress=unmatchedSuppression ^
    --suppress=unusedFunction ^
	--suppress=*:%src_folder%\config\*

REM Running cppcheck
cppcheck %CPPCHECK_FLAGS% %CPPCHECK_INCLUDES% %CPPCHECK_IGNORE% %src_folder%