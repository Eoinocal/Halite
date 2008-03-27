@echo off

call tools\from_utf16.bat ..\res\%1.rc .\%1.in

rem call ruby tools\translate.rb %*
call tools\textextractor.exe %*
del %1.in
SHIFT
:LOOP
  call tools\to_utf16.bat .\%1.out ..\res\%1.rc
  call tools\rc ..\res\%1.rc
  call tools\cvtres /MACHINE:X86 ..\res\%1.res
  call tools\link /NOENTRY /DLL /MACHINE:X86 /OUT:.\bin\x86\%1.dll ..\res\%1.obj
  call tools\cvtres /MACHINE:AMD64 ..\res\%1.res
  call tools\link /NOENTRY /DLL /MACHINE:AMD64 /OUT:.\bin\x64\%1.dll ..\res\%1.obj
rem  del %1.out
rem  del ..\res\%1.rc
  del ..\res\%1.res
  del ..\res\%1.obj
  SHIFT 
IF "%1" == "" GOTO END
GOTO LOOP
:END

rmdir eee