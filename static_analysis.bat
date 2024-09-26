@echo off

REM Define the cppcheck flags
SET CPPCHECK_FLAGS=--quiet --enable=all --error-exitcode=1 ^
    --inline-suppr

REM Running cppcheck
cppcheck %CPPCHECK_FLAGS% --project=./.cppcheck