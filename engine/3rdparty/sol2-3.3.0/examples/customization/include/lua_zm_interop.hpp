#pragma once

#ifndef PROGRAM_LUA_ZM_INTEROP_HPP
#define PROGRAM_LUA_ZM_INTEROP_HPP

#define SOL_ALL_SAFETIES_ON 1
#include <sol/forward.hpp>

#include <zm/vec3.hpp>

#include <functional>

bool sol_lua_check(sol::types<zm::vec3>, lua_State* L, int index,
     std::function<sol::check_handler_type> handler,
     sol::stack::record& tracking);

zm::vec3 sol_lua_get(sol::types<zm::vec3>, lua_State* L,
     int index, sol::stack::record& tracking);

int sol_lua_push(
     sol::types<zm::vec3>, lua_State* L, const zm::vec3& v);

#endif // PROGRAM_LUA_ZM_INTEROP_HPP
