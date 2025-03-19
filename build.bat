@echo off

IF NOT EXIST ..\build mkdir ..\build
REM -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers -Wno-deprecated-declarations -Wno-missing-braces -Wno-unused-but-set-variable

Set Opts=-DINTERNAL_DEBUG=1

set CompileFlags=-W4 -std:c++20 -permissive -fp:fast -Fm -GR- -EHa- -Od -Oi -Zi -wd4996 -wd4100 -wd4505 -wd4189
set LinkerFlags=-IGNORE:4099 -incremental:no kernel32.lib shell32.lib gdi32.lib user32.lib opengl32.lib "../deps/OpenGL/glad/src/Glad.lib" "../deps/Freetype/freetype.lib"
Set IncludePaths=-I"../code/" -I"../deps/" -I"../deps/OpenGL" -I"../deps/OpenGL/glad/include" -I"../deps/Freetype/include"

pushd ..\build
del *.obj
del *.pdb
cl %Opts% ..\code\venture_win32_entry.cpp %IncludePaths% %CompileFlags% -MT -link %LinkerFlags% -OUT:"venture_debug.exe"
popd

@echo ====================
@echo Compilation Complete
@echo ====================
