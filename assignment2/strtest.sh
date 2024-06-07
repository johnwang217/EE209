#!/bin/bash
./client StrGetLength > strtestresult
./client StrCopy >> strtestresult
./client StrCompare >> strtestresult
./client StrFindChr >> strtestresult
./client StrFindStr >> strtestresult
./client StrConcat >> strtestresult
./client StrToLong >> strtestresult
./client StrCaseCompare >> strtestresult