term.pushColor(term.yellow)
print("WARNING: IF YOU SEE THIS MESSAGE AND YOU ARE TRYING TO INCLUDE THE JSON PROJECT, YOU ARE ACTUALLY INCLUDING THE WORKSPACE FILE.")
print("      -> If so, make sure to include the json folder instead.")
term.popColor()

workspace "json"
   architecture "x64"
   configurations { "Debug", "Release", }
   startproject "json-dev"

include "json"

project "json-dev"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"

   location "build"
   targetdir "%{prj.location}/%{cfg.buildcfg}"

   includedirs { "include", "libs/miniutf8", }
   files { "src/main.cpp" }
   links "json"

   filter "toolset:gcc"
      buildoptions { "-Wall", "-Wextra", }

   filter "toolset:msc"
      buildoptions "/W3"

   filter "configurations:Debug"
      defines "JSON_DEBUG"
      symbols "On"

   filter "configurations:Release"
      optimize "Speed"
