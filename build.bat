call "%VS100COMNTOOLS%vsvars32.bat"

cl /Ox /Os /O2 /GS- /MD helloworld.cpp

cl /Ox /Os /O2 /GS- helloworld2.cpp kernel32.lib user32.lib
external\crinkler\crinkler.exe /SUBSYSTEM:WINDOWS /COMPMODE:SLOW kernel32.lib user32.lib helloworld2.obj /OUT:helloworld2_with_crinkler.exe

cl /Ox /Os /O2 /GS- /I external\glew\include opengl.cpp kernel32.lib user32.lib opengl32.lib winmm.lib gdi32.lib
external\crinkler\crinkler.exe /SUBSYSTEM:WINDOWS /COMPMODE:SLOW kernel32.lib user32.lib opengl32.lib winmm.lib gdi32.lib opengl.obj /OUT:opengl_with_crinkler.exe
