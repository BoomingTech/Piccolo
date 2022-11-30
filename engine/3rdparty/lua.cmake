set(lua_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.4.4)

add_library(lua_static STATIC 
${lua_SOURCE_DIR_}/lapi.c
${lua_SOURCE_DIR_}/lauxlib.c
${lua_SOURCE_DIR_}/lbaselib.c
${lua_SOURCE_DIR_}/lcode.c
${lua_SOURCE_DIR_}/lcorolib.c
${lua_SOURCE_DIR_}/lctype.c
${lua_SOURCE_DIR_}/ldblib.c
${lua_SOURCE_DIR_}/ldebug.c
${lua_SOURCE_DIR_}/ldo.c
${lua_SOURCE_DIR_}/ldump.c
${lua_SOURCE_DIR_}/lfunc.c
${lua_SOURCE_DIR_}/lgc.c
${lua_SOURCE_DIR_}/linit.c
${lua_SOURCE_DIR_}/liolib.c
${lua_SOURCE_DIR_}/llex.c
${lua_SOURCE_DIR_}/lmathlib.c
${lua_SOURCE_DIR_}/lmem.c
${lua_SOURCE_DIR_}/loadlib.c
${lua_SOURCE_DIR_}/lobject.c
${lua_SOURCE_DIR_}/lopcodes.c
${lua_SOURCE_DIR_}/loslib.c
${lua_SOURCE_DIR_}/lparser.c
${lua_SOURCE_DIR_}/lstate.c
${lua_SOURCE_DIR_}/lstring.c
${lua_SOURCE_DIR_}/lstrlib.c
${lua_SOURCE_DIR_}/ltable.c
${lua_SOURCE_DIR_}/ltablib.c
#${lua_SOURCE_DIR_}/ltests.c
${lua_SOURCE_DIR_}/ltm.c
#${lua_SOURCE_DIR_}/lua.c
${lua_SOURCE_DIR_}/lundump.c
${lua_SOURCE_DIR_}/lutf8lib.c
${lua_SOURCE_DIR_}/lvm.c
${lua_SOURCE_DIR_}/lzio.c
#${lua_SOURCE_DIR_}/onelua.c
)

target_include_directories(lua_static PUBLIC ${lua_SOURCE_DIR_})