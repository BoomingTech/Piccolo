#include <sol/sol.hpp>
#include <iostream>

// We capture the base objects
// lifetime style
// You can add more here,
// e.g. "Unique" for a unique pointer kind of lifestyle,
// but you would need to think about how to handle
// copying / moving in that case
enum class BaseObjectLifetime { Value, Pointer, Shared };

// The base object
// that we do inheritance and other work
// on
class BaseObject {
public:
	BaseObject() {
		objectType = 0;
	}

	unsigned int getObjectType() const {
		return objectType;
	}

	bool doArmorThing() const {
		return false;
	}
	bool doWeaponThing() const {
		return false;
	}

	// helper function defined later after we define all the
	// base classes we care about
	sol::object getAsRetyped(
	     lua_State* L, BaseObjectLifetime Lifetime) const;

	// For convenience with the customization points below
	int pushAsRetyped(
	     lua_State* L, BaseObjectLifetime Lifetime) const {
		return getAsRetyped(L, Lifetime).push(L);
	}

protected:
	unsigned int objectType;
};

class Armor : public BaseObject {
public:
	Armor() {
		objectType = 1;
	}

	bool doArmorThing() const {
		return true;
	}
};

class Weapon : public BaseObject {
public:
	Weapon() {
		objectType = 2;
	}

	bool doWeaponThing() const {
		return true;
	}
};

// Get the most-derived type
// that we care about from the base object,
// obeying the lifetime type
sol::object BaseObject::getAsRetyped(
     lua_State* L, BaseObjectLifetime Lifetime) const {
	switch (objectType) {
	case 1:
		std::cout << "Retyping as armor." << std::endl;
		switch (Lifetime) {
		case BaseObjectLifetime::Pointer:
			return sol::make_object(
			     L, static_cast<const Armor*>(this));
		case BaseObjectLifetime::Shared:
			return sol::make_object(L,
			     std::make_shared<Armor>(
			          *static_cast<const Armor*>(this)));
		case BaseObjectLifetime::Value:
		default:
			return sol::make_object(
			     L, *static_cast<const Armor*>(this));
		}
	case 2:
		std::cout << "Retyping as weapon." << std::endl;
		switch (Lifetime) {
		case BaseObjectLifetime::Pointer:
			return sol::make_object(
			     L, static_cast<const Weapon*>(this));
		case BaseObjectLifetime::Shared:
			return sol::make_object(L,
			     std::make_shared<Weapon>(
			          *static_cast<const Weapon*>(this)));
		case BaseObjectLifetime::Value:
		default:
			return sol::make_object(
			     L, *static_cast<const Weapon*>(this));
		}
	default:
		std::cout
		     << "Unknown type: falling back to base object."
		     << std::endl;
		// we have a normal type here, so that means we
		// must bypass typical customization points by using
		// sol::make_object_userdata/sol::make_reference_userdata
		// WARNING: IF THIS TYPE IS IN FACT NOT A BASE OBJECT,
		// BUT SOME DERIVED OBJECT, THEN RUNNING THIS CODE FOR
		// VALUE TYPES AND SHARED TYPES WILL "SLICE"
		// THE DERIVED BITS OFF THE BASE BITS PERMANENTLY
		// NEVER FORGET TO UPDATE THE SWITCH IF YOU ADD
		// NEW TYPES!!
		switch (Lifetime) {
		case BaseObjectLifetime::Value:
			return sol::make_object_userdata(L, *this);
		case BaseObjectLifetime::Shared:
			return sol::make_object_userdata(
			     L, std::make_shared<BaseObject>(*this));
		case BaseObjectLifetime::Pointer:
		default:
			return sol::make_object_userdata(L, this);
		}
	}
}

//
// sol customization points
//

// Defining a customization point that lets us put the correct
// object type on the stack.
int sol_lua_push(sol::types<BaseObject>, lua_State* L,
     const BaseObject& obj) {
	return obj.pushAsRetyped(L, BaseObjectLifetime::Value);
}
int sol_lua_push(sol::types<BaseObject*>, lua_State* L,
     const BaseObject* obj) {
	return obj->pushAsRetyped(L, BaseObjectLifetime::Pointer);
}
int sol_lua_push(sol::types<std::shared_ptr<BaseObject>>,
     lua_State* L, const std::shared_ptr<BaseObject>& obj) {
	return obj->pushAsRetyped(L, BaseObjectLifetime::Shared);
}

int main() {
	// test our customization points out
	std::cout << "=== Base object customization points ==="
	          << std::endl;

	sol::state lua;
	lua.open_libraries(
	     sol::lib::base, sol::lib::string, sol::lib::table);

	lua["objectCache"] = lua.create_table();

	// Do basic type binding.
	auto luaBaseObject
	     = lua.new_usertype<BaseObject>("tes3baseObject");
	luaBaseObject["objectType"]
	     = sol::readonly_property(&BaseObject::getObjectType);
	luaBaseObject["doArmorThing"] = &BaseObject::doArmorThing;
	luaBaseObject["doWeaponThing"] = &BaseObject::doWeaponThing;
	auto luaArmorObject = lua.new_usertype<Armor>("tes3armor");
	luaArmorObject[sol::base_classes]
	     = sol::bases<BaseObject>();
	luaArmorObject["doArmorThing"] = &Armor::doArmorThing;
	auto luaWeaponObject
	     = lua.new_usertype<Weapon>("tes3weapon");
	luaWeaponObject[sol::base_classes]
	     = sol::bases<BaseObject>();
	luaWeaponObject["doWeaponThing"] = &Weapon::doWeaponThing;

	// Objects we'll play with.
	BaseObject base;
	Armor armor;
	Weapon weapon;

	// Push some objects to lua.
	std::cout << "Normal pointers..." << std::endl;
	lua["ptrBase"] = &base;
	lua["ptrArmor"] = &armor;
	lua["ptrWeapon"] = &weapon;
	std::cout << std::endl;

	// Same objects but as base objects to test mapping.
	std::cout << "Base-cast pointers..." << std::endl;
	lua["ptrBaseAsBase"] = static_cast<BaseObject*>(&base);
	lua["ptrArmorAsBase"] = static_cast<BaseObject*>(&armor);
	lua["ptrWeaponAsBase"] = static_cast<BaseObject*>(&weapon);
	std::cout << std::endl;

	std::cout << "Smart direct pointers..." << std::endl;
	lua["sharedBase"] = std::make_shared<BaseObject>();
	lua["sharedArmor"] = std::make_shared<Armor>();
	lua["sharedArmor"] = std::make_shared<Weapon>();
	std::cout << std::endl;

	std::cout << "Smart pointers put as the base class..."
	          << std::endl;
	lua["sharedBaseAsBase"] = (std::shared_ptr<BaseObject>)
	     std::make_shared<BaseObject>();
	lua["sharedArmorAsBase"] = (std::shared_ptr<BaseObject>)
	     std::make_shared<Armor>();
	lua["sharedArmorAsBase"] = (std::shared_ptr<BaseObject>)
	     std::make_shared<Weapon>();
	std::cout << std::endl;

	return 0;
}
