include "../libs"

project "json"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"

   location "../build"
   targetdir "%{prj.location}/%{cfg.buildcfg}"

   includedirs { "../include", "../libs/miniutf8", }
   files { "../src/**.cpp", "../include/**.h", }
   removefiles "../src/main.cpp"

   filter "toolset:gcc"
      buildoptions { "-Wall", "-Wextra", "-Wpedantic", "-Werror", }

   filter "toolset:msc"
      buildoptions { "/W4", "/WX", }

   filter "configurations:Debug"
      defines "JSON_DEBUG"
      symbols "On"

   filter "configurations:Release"
      optimize "Speed"
