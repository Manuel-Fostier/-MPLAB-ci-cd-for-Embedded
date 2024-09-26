@echo off

REM Define the cppcheck flags
SET CPPCHECK_FLAGS=--quiet

REM Running cppcheck
cppcheck %CPPCHECK_FLAGS% --project=.cppcheck