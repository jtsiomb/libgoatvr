@echo off

echo // generated file, do not edit! >reg_modules.cc
echo namespace goatvr { >>reg_modules.cc

for %%i in (src\mod_*.cc) do (
	echo void register_%%~ni(^); >>reg_modules.cc
)

echo void register_modules() { >>reg_modules.cc

for %%i in (src\mod_*.cc) do (
	echo register_%%~ni(^); >>reg_modules.cc
)

echo } >>reg_modules.cc
echo } >>reg_modules.cc
