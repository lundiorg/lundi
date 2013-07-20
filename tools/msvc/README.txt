This directory contains solution and project files to build lua and test lundi
with lua versions 5.1 and 5.2.

== Getting Lua ==
Use the Get-Lua.ps1 script to download and unpack lua in the place the project
expects it, that is at the root of the project tree.

The versions we use for visual studio currently are 5.1.5 and 5.2.2 respectively.

Call the script with the version as parameter:

  PS C:\Users\kbok\lundi\tools\msvc> .\Get-Lua.ps1 -version "5.1.5"

The PS script expects you to have 7-zip installed (http://www.7-zip.org/).
If you don't, you can download and unpack lua manually, but you really should
use 7-zip because it's better.

If you decide to not use the default 7-zip install path, you have to edit the
unpack script accordingly.

== Building and testing ==
Lundi depends on boost. The oldest version supported is unknown at the time,
but it's known to be working with 1.53.0.

Use the solution file, lundi.sln.

The projects lundi-lua5.1 and lundi-lua5.2 have C:\boost_1_53_0 into their 
include path. You can either unpack boost there, grep/sed/whatever it into
.vcxproj files, or edit the property page from visual studio.

Once lua is unpacked and boost is configured, you're ready to build. Both
versions should build and pass all tests. If not, feel free to send a bug
report :)
