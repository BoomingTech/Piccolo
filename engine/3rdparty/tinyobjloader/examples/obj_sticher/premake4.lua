lib_sources = {
   "../../tiny_obj_loader.cc"
}

sources = {
   "obj_sticher.cc",
   "obj_writer.cc",
   }

-- premake4.lua
solution "ObjStickerSolution"
   configurations { "Release", "Debug" }

   if (os.is("windows")) then
      platforms { "x32", "x64" }
   else
      platforms { "native", "x32", "x64" }
   end

   includedirs {
      "../../"
   }

   -- A project defines one build target
   project "obj_sticher"
      kind "ConsoleApp"
      language "C++"
      files { lib_sources, sources }

      configuration "Debug"
         defines { "DEBUG" } -- -DDEBUG
         flags { "Symbols" }
         targetname "obj_sticher_debug"

      configuration "Release"
         -- defines { "NDEBUG" } -- -NDEBUG
         flags { "Symbols", "Optimize" }
         targetname "obj_sticher"
