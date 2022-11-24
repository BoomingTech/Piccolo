#pragma once

#ifndef PROGRAM_ENTITY_HPP
#define PROGRAM_ENTITY_HPP

#include <zm/vec3.hpp>

class entity {
private:
	zm::vec3 position_;

public:
	entity() {
		this->position_ = { 1, 1, 1 };
	}

	zm::vec3 get_position() const {
		return this->position_;
	}
	void set_position(zm::vec3 v) {
		this->position_ = v;
	}
};

#endif // PROGRAM_ENTITY_HPP
