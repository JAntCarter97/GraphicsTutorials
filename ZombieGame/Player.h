#pragma once

#include "Human.h"
#include <Bengine/InputManager.h>
#include <Bengine/Camera2D.h>
#include <vector>
#include "Bullet0.h"

class Gun;

class Player : public Human
{
public:

	Player();

	~Player();

	void init(float speed, glm::vec2 pos, Bengine::InputManager* inputManager, Bengine::Camera2D* camera, std::vector<Bullet0>* bullets);

	void addGun(Gun* gun);

	virtual void update(const std::vector<std::string>& levelData, std::vector<Human*>& humans, std::vector<Zombie*>& zombies, float deltaTime) override;

private:

	Bengine::InputManager* _inputManager;

	std::vector<Gun*> _guns;
	int _currentGunIndex;

	Bengine::Camera2D* _camera;
	std::vector<Bullet0>* _bullets;
};

