Total: client.exe server.exe
   @echo Make...

socklib.lib: Socklib.obj 
    lib /out:Socklib.lib Socklib.obj

Socklib.obj: Socklib.h Socklib.cpp
    cl /c /std:c++latest /O2 /Ot /Oi /Ox /Oy /Ob2 /EHsc /FC /W4 /nologo /MT socklib.cpp 

Client.exe: Socklib.h socklib.lib client.cpp
    cl /std:c++latest /O2 /Ot /Oi /Ox /Oy /Ob2 /EHsc /FC /W4 /nologo /MT client.cpp socklib.lib

Server.exe: Socklib.h socklib.lib server.cpp
    cl /std:c++latest /O2 /Ot /Oi /Ox /Oy /Ob2 /EHsc /FC /W4 /nologo /MT server.cpp socklib.lib

