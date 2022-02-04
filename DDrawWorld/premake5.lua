-- Generate Sln
-- cmd> premake5.exe vs2022

workspace "HelloWorld"
   startproject   "DDrawSample"
   language       "C++"
   cppdialect     "C++17"
   architecture   "x86_64"

   warnings       "Extra"        -- /W4
   flags          {
      "FatalWarnings",           -- /WX
      "MultiProcessorCompile"    -- /MP
   }
   buildoptions   {"/utf-8" }
   rtti           ("Off")        -- /GR-  -- Properties> C/C++> Language> Enable Run-Time Type Information
   disablewarnings { "26451" }

   configurations { "Debug", "Release" }
   location       "_PremakeGenerated"
   
	files
	{
		".editorconfig"
	}

----------------------------------
-- Util
----------------------------------
project "Util"
   targetdir "_PremakeGenerated/bin/%{cfg.buildcfg}"
   kind "SharedLib"
   defines     {"F_SHARED_COMPILE"}

   systemversion("latest") -- Properties> General> Windows SDK Version 

   includedirs {"Common/public"}
   files       {"Common/public/**.h"}

   includedirs {"Util/public",      "Util/private" }
   files       {"Util/public/**.h", "Util/private/**"}

   pchheader "Util/precompiled.h"
   pchsource "Util/private/precompiled.cpp"

   filter "configurations:Debug"
      defines { "DEBUG" }
      optimize "Off" -- /Od
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On" -- /Ox

----------------------------------
-- ImageData
----------------------------------
project "ImageData"
   targetdir "_PremakeGenerated/bin/%{cfg.buildcfg}"
   kind "SharedLib"
   defines     {"F_SHARED_COMPILE"}


   includedirs {"Common/public"}
   files       {"Common/public/**.h"}

   includedirs {"ImageData/public",      "ImageData/private" }
   files       {"ImageData/public/**.h", "ImageData/private/**"}

   pchheader "ImageData/precompiled.h"
   pchsource "ImageData/private/precompiled.cpp"

   filter "configurations:Debug"
      defines { "DEBUG" }
      optimize "Off" -- /Od
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On" -- /Ox

----------------------------------
-- DDrawLib
----------------------------------
project "DDrawLib"
   targetdir "_PremakeGenerated/bin/%{cfg.buildcfg}"
   kind "SharedLib"
   defines     {"F_SHARED_COMPILE"}

   -- systemversion: https://github.com/premake/premake-core/issues/935
   -- systemversion("10.0.19041.0") -- Properties> General> Windows SDK Version 
   systemversion("latest") -- Properties> General> Windows SDK Version 
   -- libdirs { "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64" }
   links { "ddraw.lib", "dxguid.lib" }

   links       {"Util",        "ImageData"}
   includedirs {"Util/public", "ImageData/public"}

   includedirs {"Common/public"}
   files       {"Common/public/**.h"}
   
   includedirs {"DDrawLib/public",      "DDrawLib/private" }
   files       {"DDrawLib/public/**.h", "DDrawLib/private/**"}

   pchheader "ddrawlib/precompiled.h"
   pchsource "DDrawLib/private/precompiled.cpp"

   filter "configurations:Debug"
      defines { "DEBUG" }
      optimize "Off" -- /Od
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On" -- /Ox

----------------------------------
-- DDrawSample
----------------------------------
project "DDrawSample"
   debugdir "./"
   targetdir "_PremakeGenerated/bin/%{cfg.buildcfg}"
   kind "WindowedApp"
   
   implibdir ("_PremakeGenerated/bin/%{cfg.buildcfg}")    -- for .dll -- default - sameas targetdir
   libdirs     {"_PremakeGenerated/bin/%{cfg.buildcfg}"}  -- for .lib
   links       {"Util",        "DDrawLib",        "ImageData"}
   includedirs {"Util/public", "DDrawLib/public", "ImageData/public"}

   includedirs {"Common/public"}
   files       {"Common/public/**.h"}

   includedirs {"DDrawSample/public",      "DDrawSample/private",    "DDrawSample/Resource"}
   files       {"DDrawSample/public/**.h", "DDrawSample/private/**", "DDrawSample/Resource/**"}

   pchheader "ddrawsample/precompiled.h"
   pchsource "DDrawSample/private/precompiled.cpp"

   filter "configurations:Debug"
      defines { "DEBUG" }
      optimize "Off" -- /Od
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On" -- /Ox
	
-- ref: https://premake.github.io/docs/newaction/
-- cmd> premake5 --help
newaction {
   trigger     = "clean",
   description = "[custom] clean the software",
   execute     = function ()
      print("clean the build...")
      os.rmdir("./_PremakeGenerated")
      print("done.")
   end
}