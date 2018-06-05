/*
** EPITECH PROJECT, 2018
** bomberman
** File description:
** PlayerEntity.hpp
*/

#pragma once

#include "../AEntity.hpp"
#include "../AMovable.hpp"
#include "../Controllable.hpp"

class PlayerEntity : public Controllable,
		     public AEntity,
		     public AMovable<float> {
public:
	PlayerEntity();
	void update();
	void updateRender() override;
	void dump(std::ostream &s) const;
	void load(std::istream &s);

private:
	struct serialize {
	};
};
