#pragma once

#ifndef PROGRAM_LUA_INTEROP_HPP
#define PROGRAM_LUA_INTEROP_HPP

#include <lua_zm_interop.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/forward.hpp>

void register_lua(sol::state& lua);

#endif // PROGRAM_LUA_INTEROP_HPP
