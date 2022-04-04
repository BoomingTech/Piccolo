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

		configuration { "linux" }
			linkoptions { "`pkg-config --libs glfw3`" }
			links { "GL", "GLU", "m", "GLEW", "X11", "Xrandr", "Xinerama", "Xi", "Xxf86vm", "Xcursor", "dl" }
			linkoptions { "-pthread" }

		configuration { "windows" }
			-- Path to GLFW3
			includedirs { '../../../../local/glfw-3.1.2.bin.WIN64/include' }
			libdirs { '../../../../local/glfw-3.1.2.bin.WIN64/lib-vc2013' }
			-- Path to GLEW
			includedirs { '../../../../local/glew-1.13.0/include' }
			libdirs { '../../../../local/glew-1.13.0/lib/Release/x64' }

			links { "glfw3", "glew32", "gdi32", "winmm", "user32", "glu32","opengl32", "kernel32" }
			defines { "_CRT_SECURE_NO_WARNINGS" }

		configuration { "macosx" }
			includedirs { "/usr/local/include" }
			buildoptions { "-Wno-deprecated-declarations" }
			libdirs { "/usr/local/lib" }
			links { "glfw3", "GLEW" }
			linkoptions { "-framework OpenGL", "-framework Cocoa", "-framework IOKit", "-framework CoreVideo" }

		configuration "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "ExtraWarnings"}

		configuration "Release"
			defines { "NDEBUG" }
			flags { "Optimize", "ExtraWarnings"}

