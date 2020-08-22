@echo off

set opts=-FC -GR- -EHa- -nologo -Zi
set code=%cd%
pushd o
cl %opts% %code%\g -Feb
popd
