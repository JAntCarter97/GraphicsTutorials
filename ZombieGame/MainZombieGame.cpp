#include "MainZombieGame.h"
#include <iostream>
#include <random>
#include <ctime>
#include <glm/gtx/rotate_vector.hpp>
#include <Bengine/Timing.h>
#include <Bengine/BengineErrors.h>
#include <Bengine/ResourceManager.h>
#include <algorithm>
#include "Zombie.h"
#include "Gun.h"

// GLOBAL CONSTANTS::
const float HUMAN_SPEED = 0.5f;
const float ZOMBIE_SPEED = 1.0f;
const float PLAYER_SPEED = 2.0f;

// FUNCTIONS

// IMPLEMENTATIONS

MainZombieGame::MainZombieGame() : m_screenWidth(1024),
								   m_screenHeight(768),
								   m_gameState(GameState::PLAY),
								   m_fps(0),
								   m_player(nullptr),
								   m_numHumansKilled(0),
								   m_numZombiesKilled(0)
{

}


MainZombieGame::~MainZombieGame()
{
	// Delete Levels
	for (int i = 0; i < m_levels.size(); i++)
	{
		delete m_levels[i];
	}
	
	for (int i = 0; i < m_humans.size(); i++)
	{
		delete m_humans[i];
	}

	for (int i = 0; i < m_zombies.size(); i++)
	{
		delete m_zombies[i];
	}
}


// Runs Zombie game
void MainZombieGame::run()
{
	initSystems();

	initLevel();

	Bengine::Music music = m_audioEngine.loadMusic("Sound/XYZ.ogg");
	music.play(-1);

	gameLoop();
}

// Initialize Systems
void MainZombieGame::initSystems()
{
	Bengine::init();

	// Initialize sound , must happen after Bengine::init
	m_audioEngine.init();

	m_window.create("Zombie In Your Pooper", m_screenWidth, m_screenHeight, 0);
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	initShaders();

	m_agentSpriteBatch.init();
	m_hudSpriteBatch.init();

	// Initialize sprite font (must be initialized after openGL and SDL or else will cause errors)
	m_spriteFont = new Bengine::SpriteFont("Fonts/chintzy.ttf", 64);

	m_camera.init(m_screenWidth, m_screenHeight);
	m_hudCamera.init(m_screenWidth, m_screenHeight);
	m_hudCamera.setPosition(glm::vec2(m_screenWidth / 2, m_screenHeight / 2));

	// Initialize Particles w/ a lamda for init @params #4
	m_bloodParticleBatch = new Bengine::ParticleBatch2D;
	m_bloodParticleBatch->init(1000, 
							   0.01f, 
							   Bengine::ResourceManager::getTexture("Textures/particle.png"), 
							   [](Bengine::Particle2D& particle, float deltaTime)
							   {
									particle.position += particle.velocity * deltaTime;
									particle.color.a = (GLubyte)(particle.life * 255.0f);
							   });
	m_particleEngine.addParicleBatch(m_bloodParticleBatch);

}

// Initializes level and sets up shit
void MainZombieGame::initLevel()
{
	// Level 1
	m_levels.push_back(new Level("Levels/level1.txt"));
	m_currentLevel = 0;

	m_player = new Player();
	m_player->init(PLAYER_SPEED, m_levels[m_currentLevel]->getStartPlayerPos(), &m_inputManager, &m_camera, &m_bullets);

	m_humans.push_back(m_player);

	std::mt19937 randomEngine;
	randomEngine.seed(time(nullptr));
	std::uniform_int_distribution<int> randX(2, m_levels[m_currentLevel]->getWidth() - 2);
	std::uniform_int_distribution<int> randY(2, m_levels[m_currentLevel]->getHeight() - 2);

	// Add all the random humans
	for (int i = 0; i < m_levels[m_currentLevel]->getNumHumans(); i++)
	{
		m_humans.push_back(new Human);
		glm::vec2 pos(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH);
		m_humans.back()->init(HUMAN_SPEED, pos);
	}

	// add the zombies
	const std::vector<glm::vec2>& zombiePositions = m_levels[m_currentLevel]->getZombieStartPositions();
	for (int i = 0; i < zombiePositions.size(); i++)
	{
		m_zombies.push_back(new Zombie);
		m_zombies.back()->init(ZOMBIE_SPEED, zombiePositions[i]);
	}

	// Set up the player's guns
	const float BULLET_SPEED = 20.0f;
	m_player->addGun(new Gun("Magnum", 40, 1, 0.2f, 100.0f, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/shots/pistol.wav")));
	m_player->addGun(new Gun("Shotgun", 40, 10, 0.2f, 50.0f, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/shots/shotgun.wav")));
	m_player->addGun(new Gun("MP5", 10, 1, 0.2f, 80.0f, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/shots/cg1.wav")));
}

// Initialize the shaders
void MainZombieGame::initShaders()
{
	m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
	m_textureProgram.addAttribute("vertexPosition");
	m_textureProgram.addAttribute("vertexColor");
	m_textureProgram.addAttribute("vertexUV");
	m_textureProgram.linkShaders();
}

// Main Game Loop
void MainZombieGame::gameLoop()
{
	const float DESIRED_FPS = 60.0f;
	const int MAX_PHYSICS_STEPS = 6;

	Bengine::FpsLimiter fpsLimiter;
	fpsLimiter.setMaxFPS(60.0f);

	//Scale camera out
	const float CAMERA_SCALE = 1.0f / 2.0f;
	m_camera.setScale(CAMERA_SCALE);

	const float MS_PER_SECOND = 1000;
	const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS;
	const float MAX_DELTA_TIME = 1.0f;
	
	// Returns the number of milliseconds since sdl was initialized
	float previousTicks = SDL_GetTicks();

	while (m_gameState == GameState::PLAY)
	{
		fpsLimiter.begin();

		float newTicks = SDL_GetTicks();
		float frameTime = SDL_GetTicks() - previousTicks;
		previousTicks = newTicks;
		float totalDeltaTime = frameTime / DESIRED_FRAMETIME;

		checkVictory();

		m_inputManager.update();

		processInput();

		int i = 0;
		while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS)
		{
			float deltaTime = std::min(totalDeltaTime, MAX_DELTA_TIME);
			// Update Agents
			updateAgents(deltaTime);
			// Update Bullets
			updateBullets(deltaTime);
			// Update particles
			m_particleEngine.update(deltaTime);
			i++;
		}

		// Make sure the camera is bound to the player position
		m_camera.setPosition(m_player->getPosition());

		m_camera.update();

		m_hudCamera.update();

		drawGame();

		m_fps = fpsLimiter.end();

		//Print fps to console
		std::cout << m_fps << std::endl;
	}
}

// Updates all agents
void MainZombieGame::updateAgents(float deltaTime)
{
	for (int i = 0; i < m_humans.size(); i++)
	{
		m_humans[i]->update(m_levels[m_currentLevel]->getLevelData(), m_humans, m_zombies, deltaTime);
	}

	for (int i = 0; i < m_zombies.size(); i++)
	{
		m_zombies[i]->update(m_levels[m_currentLevel]->getLevelData(), m_humans, m_zombies, deltaTime);
	}

	// Update Zombie collisions
	for (int i = 0; i < m_zombies.size(); i++)
	{
		// Collide with other zombies
		for (int j = i + 1; j < m_zombies.size(); j++)
		{
			m_zombies[i]->collideWithAgent(m_zombies[j]);
		}
		//Collide with humans MUAHAHA
		for (int j = 1; j < m_humans.size(); j++)
		{
			if (m_zombies[i]->collideWithAgent(m_humans[j]))
			{
				// Add the new zombie
				m_zombies.push_back(new Zombie);
				m_zombies.back()->init(ZOMBIE_SPEED, m_humans[j]->getPosition());
				// Delete the human (now is zombie)
				delete m_humans[j];
				m_humans[j] = m_humans.back();
				m_humans.pop_back();
			}
		}

		// Collide with player
		if (m_zombies[i]->collideWithAgent(m_player))
		{
			Bengine::fatalError("YOU WERE BITTEN");
		}
	}

	// Update Human collisions
	for (int i = 0; i < m_humans.size(); i++)
	{
		for (int j = i + 1; j < m_humans.size(); j++)
		{
			m_humans[i]->collideWithAgent(m_humans[j]);
		}
	}
}

void MainZombieGame::updateBullets(float deltaTime)
{
	// Update and collide wth world
	for (int i = 0; i < m_bullets.size(); )
	{
		// If update returns true, the bullet collided with a wall
		if (m_bullets[i].update(m_levels[m_currentLevel]->getLevelData(), deltaTime))
		{
			m_bullets[i] = m_bullets.back();
			m_bullets.pop_back();
		}
		else
		{
			i++;
		}
	}

	bool wasBulletRemoved;

	// Collide with humans and zombies
	for (int i = 0; i < m_bullets.size(); i++)
	{
		wasBulletRemoved = false;
		// Loop through zombies
		for (int j = 0; j < m_zombies.size(); )
		{
			// Check collision
			if (m_bullets[i].collideWithAgent(m_zombies[j]))
			{
				// Add Blood
				addBlood(m_bullets[i].getPosition(), 5);

				// Damage zombie, and kill it if it's out of health
				if (m_zombies[j]->applyDamage(m_bullets[i].getDamage()))
				{
					// If the zombie died, remove him
					delete m_zombies[j];
					m_zombies[j] = m_zombies.back();
					m_zombies.pop_back();
					m_numZombiesKilled++;
				}
				else
				{
					j++;
				}

				// Remove the bullet
				m_bullets[i] = m_bullets.back();
				m_bullets.pop_back();
				wasBulletRemoved = true;
				i--; // Make sure we don't skip a bullet

				// since the bullet died, no need to loop through any more zombies
				break;
			}
			else
			{
				j++;
			}
		}
		// Loop through humans
		if (wasBulletRemoved == false)
		{
			for (int j = 1; j < m_humans.size(); )
			{
				// Check collision
				if (m_bullets[i].collideWithAgent(m_humans[j]))
				{

					// Add Blood
					addBlood(m_bullets[i].getPosition(), 20);

					// Damage human, and kill it if it's out of health
					if (m_humans[j]->applyDamage(m_bullets[i].getDamage()))
					{
						// If the human died, remove him
						delete m_humans[j];
						m_humans[j] = m_humans.back();
						m_humans.pop_back();
						m_numHumansKilled++;
					}
					else
					{
						j++;
					}

					// Remove the bullet
					m_bullets[i] = m_bullets.back();
					m_bullets.pop_back();
					wasBulletRemoved = true;
					i--; // Make sure we don't skip a bullet

					// since the bullet died, no need to loop through any more humans
					break;
				}
				else
				{
					j++;
				}
			}
		}

	}


}

void MainZombieGame::checkVictory()
{
	// TODO: Support for multiple levels!
	//	_currentLevel++; initLevel(...);
	
	// If all zombies are dead we win!
	if (m_zombies.empty())
	{
		std::printf("**** You Tango'd. Groovy ****\n**** You murdered %d humans, and killed %d zombies. ****\n**** %d/%d humans lived. ****\n", m_numHumansKilled, m_numZombiesKilled, m_humans.size() - 1, m_levels[m_currentLevel]->getNumHumans());
		Bengine::fatalError("");
	}
}

// Handles all Input Processing
void MainZombieGame::processInput()
{
	SDL_Event evnt;
	while (SDL_PollEvent(&evnt))
	{
		switch (evnt.type)
		{
		case SDL_QUIT:
			// Exits the game
			m_gameState = GameState::EXIT;
			break;
		case SDL_MOUSEMOTION:
			m_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
			break;
		case SDL_KEYDOWN:
			m_inputManager.pressKey(evnt.key.keysym.sym);
			break;
		case SDL_KEYUP:
			m_inputManager.releaseKey(evnt.key.keysym.sym);
			break;
		case SDL_MOUSEBUTTONDOWN:
			m_inputManager.pressKey(evnt.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			m_inputManager.releaseKey(evnt.button.button);
			break;
		}
	}

}

// Renders the game
void MainZombieGame::drawGame()
{
	// Set the base depth to 1.0
	glClearDepth(1.0);
	// Clear color and Depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_textureProgram.use();

	// Draw code here
	glActiveTexture(GL_TEXTURE0);

	// Make sure the shader uses texture 0
	GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
	glUniform1i(textureUniform, 0);

	// Grab the camera matrix
	glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
	GLint pUniform = m_textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	// Draw Level
	m_levels[m_currentLevel]->draw();

	// Begin drawing agents
	m_agentSpriteBatch.begin();
	
	// Const only if all agents are the same size (in our case they are)
	const glm::vec2 agentDims(AGENT_RADIUS * 2.0f);

	// Draw humans
	for (int i = 0; i < m_humans.size(); i++)
	{
		if (m_camera.isBoxInView(m_humans[i]->getPosition(), agentDims))
		{
			m_humans[i]->draw(m_agentSpriteBatch);
		}
	}

	// Draw Zombies
	for (int i = 0; i < m_zombies.size(); i++)
	{
		if (m_camera.isBoxInView(m_zombies[i]->getPosition(), agentDims))
		{
			m_zombies[i]->draw(m_agentSpriteBatch);
		}
	}

	// Draw Bullets
	for (int i = 0; i < m_bullets.size(); i++)
	{
		m_bullets[i].draw(m_agentSpriteBatch);
	}

	m_agentSpriteBatch.end();

	m_agentSpriteBatch.renderBatch();

	// Render the particles
	m_particleEngine.draw(&m_agentSpriteBatch);

	// Render the HUD
	drawHud();
	
	m_textureProgram.unuse();

	// Swap our buffer and draw everything to the screen!
	m_window.swapBuffer();
}

void MainZombieGame::drawHud()
{
	char buffer[256];

	glm::mat4 projectionMatrix = m_hudCamera.getCameraMatrix();
	GLint pUniform = m_textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	m_hudSpriteBatch.begin();

	// Render Human count to screen
	sprintf_s(buffer, "Num Humans %d", m_humans.size());
	m_spriteFont->draw(m_hudSpriteBatch, buffer, glm::vec2(0, 0),
					  glm::vec2(0.5f), 0.0f, Bengine::ColorRGBA8(255, 255, 255, 255));

	// Render Zombie count to screen
	sprintf_s(buffer, "Num Zombies %d", m_zombies.size());
	m_spriteFont->draw(m_hudSpriteBatch, buffer, glm::vec2(0, 36),
					  glm::vec2(0.5f), 0.0f, Bengine::ColorRGBA8(255, 255, 255, 255));

	m_hudSpriteBatch.end();

	m_hudSpriteBatch.renderBatch();
}

void MainZombieGame::addBlood(const glm::vec2& position, int numParticles)
{
	static std::mt19937 randEngine(time(nullptr));
	static std::uniform_real_distribution<float> randAngle(0.0f, 360.0f);

	glm::vec2 vel(1.0f, 0.0f);
	Bengine::ColorRGBA8 col(255, 0, 0, 255);

	for (int i = 0; i < numParticles; i++)
	{
		m_bloodParticleBatch->addParticle(position, glm::rotate(vel, randAngle(randEngine)), col, 10.0f);
	}
}