cd %~dp0

start /wait "Astyle Clean" cmd /c call Astyle_clean.bat
start /wait "Board Clean" cmd /c call Board_clean.bat
start /wait "ProjectTemplate Clean" cmd /c call ProjectTemplate_clean.bat
start /wait "Eclipse_clean" cmd /c call Eclipse_clean.bat
