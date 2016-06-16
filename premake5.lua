
solution 'bit7z'
    location 'build'
    configurations { 'Release', 'Debug' }
    platforms { 'Win32', 'x64' }
    warnings "Extra"
    defines { '_UNICODE' }

project 'bit7z'
    kind 'Staticlib'
    files { 'src/*.cpp', 'include/*.hpp' }
    includedirs { 'lib/7zSDK/CPP/'}
    postbuildcommands
    {
        [[{COPY} ..\include\bit*.hpp ..\out\include\]],
        [[{COPY} %{cfg.buildtarget.abspath} ..\out\lib\]],
    }

    filter { 'configurations:Release', 'platforms:Win32' }
        optimize 'On'
        defines { 'NDEBUG' }
        architecture 'x86'
        targetsuffix '32'

    filter { 'configurations:Release', 'platforms:x64' }
        optimize 'On'
        defines { 'NDEBUG' }
        architecture 'x86_64'
        targetsuffix '64'

    filter { 'configurations:Debug', 'platforms:Win32' }
        flags { 'Symbols' }
        defines { 'DEBUG' }
        architecture 'x86'
        targetsuffix '32d'

    filter { 'configurations:Debug', 'platforms:x64' }
        flags { 'Symbols' }
        defines { 'DEBUG' }
        architecture 'x86_64'
        targetsuffix '64d'
