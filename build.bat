@echo off

if not defined DevEnvDir (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >NUL
)

IF NOT EXIST build mkdir build
pushd build

set files=src/main.c
set compile_flags=/std:c11 /MT /nologo /GR- /EHa- /Od /Oi /WX /W4 /wd4100 /DDEBUG /FC /Z7 /Fm3drenderer.map
set linker_flags=/opt:ref /subsystem:windows user32.lib gdi32.lib

cl %compile_flags% ../src/main.c /link %linker_flags%

popd
