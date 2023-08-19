project "json"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"

   location "../build"
   targetdir "%{prj.location}/%{cfg.buildcfg}"

   includedirs "../include"
   files { "../src/**.cpp", "../include/**.h", }
   removefiles "../src/main.cpp"

   filter "toolset:gcc"
      buildoptions { "-Wall", "-Wextra", }

   filter "toolset:msc"
      buildoptions "/W3"

   filter "configurations:Debug"
      defines "JSON_DEBUG"
      symbols "On"

   filter "configurations:Release"
      optimize "Speed"
