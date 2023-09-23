IG_ROOT = os.getenv("IG_ROOT")
FBX_SDK = "D:/Dev/FBX SDK/2020.2.1"

workspace "IGBConverter"
	kind "ConsoleApp"
	language "C++"
	system "Windows"
	systemversion "latest"
	location "build"
	configurations { "Debug", "Release" }
	targetdir "bin/%{cfg.buildcfg}"
	staticruntime "on"
	
	filter "configurations:Debug"
		libdirs { path.join(IG_ROOT, "DirectX9/libdbg"), path.join(FBX_SDK, "lib/vs2019/x86/debug") }
		defines { "WIN32", "_DEBUG", "_CONSOLE", "IG_COMPILER_MSVC", "IG_TARGET_WIN32", "IG_TARGET_TYPE_WIN32", "IG_GFX_DX9", "IG_ALCHEMY_DLL=1", "IG_DEBUG" }
		symbols "on"
		postbuildcommands "copy /y \"$(TargetDir)\""
	filter {}
	
	filter "configurations:Release"
		libdirs { path.join(IG_ROOT, "DirectX9/lib"), path.join(FBX_SDK, "lib/vs2019/x86/release") }
		defines { "WIN32", "NDEBUG", "_CONSOLE", "IG_COMPILER_MSVC", "IG_TARGET_WIN32", "IG_TARGET_TYPE_WIN32", "IG_GFX_DX9", "IG_ALCHEMY_DLL=1" }
		optimize "on"
		postbuildcommands "copy /y \"$(TargetDir)\""
	filter {}
	
	includedirs { path.join(IG_ROOT, "include"), path.join(FBX_SDK, "include") }
	
	links { "libIGCore", "libIGSg", "libIGMath", "libIGAttrs", "libIGGfx", "libIGOpt" }
	links { "libfbxsdk-mt", "libxml2-mt", "zlib-mt" }
   
	files { "src/*.*" }
   
project "IGBConverter"