/*
** EPITECH PROJECT, 2018
** bomberman
** File description:
** EntitiesPool.cpp
*/

#include "../../include/Game/EntitiesMap.hpp"
#include "../../include/Game/Entities/PotEntity.hpp"
#include "../../include/Game/BKeyboardController.hpp"
#include "../../include/Game/Entities/PlayerEntity.hpp"
#include "../../include/Game/Entities/BlockEntity.hpp"
#include "../../include/Game/BIAController.hpp"
#include "../../include/Singletons/AssetsPool.hpp"

const std::unordered_map<char, std::function<AEntity *(const std::vector<int> &)>>
	EntitiesMap::_generationMap = {{'X', [](const std::vector<int> &) {
	return new BlockEntity();
}}, {'1', [](const std::vector<int> &IAState) {
	bool ia = IAState.at(0);
	return EntitiesMap::generatePlayer(1, ia);
}}, {'2', [](const std::vector<int> &IAState) {
	bool ia = IAState.at(1);
	return EntitiesMap::generatePlayer(2, ia);
}}, {'3', [](const std::vector<int> &IAState) {
	bool ia = IAState.at(2);
	return EntitiesMap::generatePlayer(3, ia);
}}, {'4', [](const std::vector<int> &IAState) {
	bool ia = IAState.at(3);
	return EntitiesMap::generatePlayer(4, ia);
}}, {'0', [](const std::vector<int> &) {
	AEntity *e = nullptr;
//	if ((rand() % 6) < 4)
//		e = new PotEntity();
	return e;
}}};

const std::vector<std::string> EntitiesMap::_mapTemplate = {
	"XXXXXXXXXXXXXXXXXXX", "X1 0000000000000 2X", "X X0X0X0X0X0X0X0X X",
	"X00000000000000000X", "X0X0X0X0X0X0X0X0X0X", "X00000000000000000X",
	"X0X0X0X0X0X0X0X0X0X", "X00000000000000000X", "X0X0X0X0X0X0X0X0X0X",
	"X00000000000000000X", "X0X0X0X0X0X0X0X0X0X", "X00000000000000000X",
	"X X0X0X0X0X0X0X0X X", "X3 0000000000000 4X", "XXXXXXXXXXXXXXXXXXX",};

PlayerEntity *EntitiesMap::generatePlayer(unsigned playerSkinId, bool ia)
{
	if (ia) {
		auto *controller = new BKeyboardController(playerSkinId);
		EntityFactory fac;
		PlayerEntity *player = new PlayerEntity(
			(playerSkinId < 5 ? playerSkinId : 1));
		AController::bindEntityToController(*controller, *player);
		return player;
	} else {
		auto *controller = new BIAController(playerSkinId);

		PlayerEntity *player = new PlayerEntity(
			(playerSkinId < 5 ? playerSkinId : 1));
		AController::bindEntityToController(*controller, *player);
		return player;
	}
}

bool EntitiesMap::generate(const std::vector<int> &IAState)
{
	for (size_t y = 0; y < HEIGHT; y++) {
		for (size_t x = 0; x < WIDTH; x++) {
			auto type = _mapTemplate[y][x];
			AEntity *e = nullptr;
			if (EntitiesMap::_generationMap.count(type) > 0)
				e = EntitiesMap::_generationMap.at(type)(IAState);
			if (e) {
				insert(e,
					{static_cast<irr::s32>(WIDTH - (x + 1)),
						static_cast<irr::s32>(y)});
			}
		}
	}
	return true;
}

// region List

bool EntitiesMap::insert(AEntity *e, const irr::core::vector2di &v)
{
	auto fct = [e](const InsertTrans &d) {
		return (d.e == e);
	};
	if (std::find_if(_toInsert.begin(), _toInsert.end(), fct)  != _toInsert.end())
		return false;
	InsertTrans data = {e, v};
	_toInsert.push_back(data);
	return false;
}

bool EntitiesMap::erase(AEntity *e)
{
	auto fct = [e](const EraseTrans &d) {
		return (d.e == e);
	};
	if (std::find_if(
		_toErase.begin(), _toErase.end(), fct)  != _toErase.end())
		return false;
	EraseTrans data = {e};
	_toErase.push_back(data);
	return true;
}

bool EntitiesMap::moveTo(AEntity *e, const irr::core::vector2di &v)
{
	auto fct = [e](const MoveTrans &d) {
		return (d.e == e);
	};
	if (std::find_if(
		_toMove.begin(), _toMove.end(), fct)  != _toMove.end())
		return false;
	MoveTrans data = {e, v};
	_toMove.push_back(data);
	return true;
}


void EntitiesMap::updateInsert()
{
	std::vector<std::unique_ptr<AEntity>> trash;

	for (auto &n : _toInsert) {
		auto x = n.v.X;
		auto y = n.v.Y;
		if (!canInsertTo(n.v)) {
			trash.push_back(std::unique_ptr<AEntity>(n.e));
			continue;
		}
		for (auto &s : _map[y][x]) {
			s->collide(*(n.e));
			n.e->collide(*(s.get()));
		}
		_map[y][x].push_back(std::unique_ptr<AEntity>(n.e));
		n.e->setPosition(n.v);
	}
	_toInsert.clear();
}

void EntitiesMap::updateErase()
{
	for (auto &n : _toErase) {
		auto e = n.e;
		auto x = n.e->getPosition().X;
		auto y = n.e->getPosition().Y;
		if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
			continue;
		auto &list = _map[y][x];
		auto finder = [e](const std::unique_ptr<AEntity> &p) {
			return (e == p.get());
		};
		if (n.e->getType() == "player")
			_orderDie.push_back(dynamic_cast<PlayerEntity *>(n.e)->getPlayerSkinId());
		auto elem = std::find_if(list.begin(), list.end(), finder);
		if (elem != list.end())
			list.erase(elem);
	}
	_toErase.clear();
}

void EntitiesMap::updateMove()
{
	for (auto &n : _toMove) {
		auto e = n.e;
		auto ox = n.e->getPosition().X;
		auto oy = n.e->getPosition().Y;
		if (ox < 0 || ox >= WIDTH || oy < 0 || oy >= HEIGHT)
			continue;
		auto dx = n.v.X;
		auto dy = n.v.Y;
		if (dx < 0 || dx >= WIDTH || dy < 0 || dy >= HEIGHT)
			continue;
		auto &list = _map[oy][ox];
		auto finder = [e](const std::unique_ptr<AEntity> &p) {
			return (e == p.get());
		};
		auto elem = std::find_if(list.begin(), list.end(), finder);
		if (elem != list.end()) {
			for (auto &s : _map[dy][dx]) {
				s->collide(*(n.e));
				n.e->collide(*(s.get()));
			}
			elem->release();
			list.erase(elem);
			_map[dy][dx].push_back(std::unique_ptr<AEntity>(n.e));
			n.e->setPosition(n.v);
		}
	}
	_toMove.clear();
}

bool EntitiesMap::canInsertTo(const irr::core::vector2di &v)
{
	if (v.X < 0 || v.X >= WIDTH || v.Y < 0 || v.Y >= HEIGHT)
		return false;
	for (auto &n : _map[v.Y][v.X])
		if (!n->isInsertable())
			return false;
	return true;
}

bool EntitiesMap::canMoveTo(const irr::core::vector2di &v, const AEntity *entity)
{
	if (v.X < 0 || v.X >= WIDTH || v.Y < 0 || v.Y >= HEIGHT)
		return false;
	for (auto &n : _map[v.Y][v.X])
		if (!n->isStackable(entity))
			return false;
	return true;
}

//endregion

EntitiesMap::EntitiesMap()
: _map(), _orderDie()
{
	_orderDie.reserve(4);
	_map.resize(HEIGHT);
	for (auto &n : _map)
		n.resize(WIDTH);
}

void EntitiesMap::updateRender()
{
	for (auto &n : _map)
		for (auto &eList : n)
			for (auto &e : eList)
				e->updateRender();
}

size_t EntitiesMap::update()
{
	size_t ret = 0;

	updateInsert();
	for (auto &n : _map)
		for (auto &eList : n)
			for (auto &e : eList) {
				e->update(this);
				ret += (e->getType() == "player");
			}
	updateErase();
	updateMove();
	AssetsPool::getInstance().cleanSound();
	return ret;
}

EntitiesMap::EMap &EntitiesMap::getMap()
{
	return _map;
}

std::vector<int> EntitiesMap::getPodium()
{
	return _orderDie;
}

int EntitiesMap::getPodiumN(int n)
{
	if (_orderDie.size() <= n)
		return -1;
	return _orderDie[n];
}

void EntitiesMap::setPodium(const std::vector<int> &orderDie)
{
	_orderDie = orderDie;
}
