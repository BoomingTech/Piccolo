newoption {
   trigger     = "with-zlib",
   description = "Build with zlib."
}

newoption {
   trigger     = "with-zstd",
   description = "Build with ZStandard compression."
}

newoption {
   trigger     = "clang",
   description = "Use clang compiler."
}

newoption {
   trigger     = "asan",
   description = "Enable AddressSanitizer(gcc or clang only)."
}

solution "objview"
	-- location ( "build" )
	configurations { "Release", "Debug" }
	platforms {"native", "x64", "x32"}
	
	project "objview"

	kind "ConsoleApp"
	language "C++"
	files { "viewer.cc", "trackball.cc" }
	includedirs { "./" }
	includedirs { "../../" }

	flags { "c++11" }

        if _OPTIONS['clang'] then
           toolset "clang"
        end

	if _OPTIONS['with-zlib'] then
		defines { 'ENABLE_ZLIB' }
		links { 'z' }
	end	

	if _OPTIONS['asan'] then
		buildoptions { '-fsanitize=address' }
		linkoptions { '-fsanitize=address' }
	end	

	if _OPTIONS['with-zstd'] then
		print("with-zstd")
		defines { 'ENABLE_ZSTD' }
		-- Set path to zstd installed dir.
		includedirs { '$$HOME/local/include' }
		libdirs { '$$HOME/local/lib' }
		links { 'zstd' }
	end	

	-- Uncomment if you want address sanitizer(gcc/clang only)
	--buildoptions { "-fsanitize=address" }
	--linkoptions { "-fsanitize=address" }

	configuration { "linux" }
		linkoptions { "`pkg-config --libs glfw3`" }
		links { "GL", "GLU", "m", "GLEW", "X11", "Xrandr", "Xinerama", "Xi", "Xxf86vm", "Xcursor", "dl" }
		linkoptions { "-pthread" }

	configuration { "windows" }
		-- Path to GLFW3
		includedirs { '../../../local/glfw-3.2.bin.WIN64/include' }
		libdirs { '../../../local/glfw-3.2.bin.WIN64/lib-vc2015' }
		-- Path to GLEW
		includedirs { '../../../local/glew-1.13.0/include' }
		libdirs { '../../../local/glew-1.13.0/lib/Release/x64' }

		links { "glfw3", "glew32", "gdi32", "winmm", "user32", "glu32","opengl32", "kernel32" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
		defines { "NOMINMAX" }

	configuration { "macosx" }
		includedirs { "/usr/local/include" }
		buildoptions { "-Wno-deprecated-declarations" }
		libdirs { "/usr/local/lib" }
		links { "glfw3", "GLEW" }
		linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols"}

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize"}

