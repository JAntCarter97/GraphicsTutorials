#pragma once

// External Deps/Library Includes
#include <GL/glew.h>
#include <SDL/SDL.h>

// Bengine Includes
#include <Bengine/Bengine.h>
#include <Bengine/Window.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/InputManager.h>
#include <Bengine/Camera2D.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/SpriteFont.h>
#include <Bengine/AudioEngine.h>
#include <Bengine/ParticleEngine2D.h>
#include <Bengine/ParticleBatch2D.h>

// File Includes
#include "Level.h"
#include "Player.h"
#include "Bullet0.h"

class Zombie;

enum class GameState { PLAY, EXIT };

class MainZombieGame
{
public:

	MainZombieGame();

	~MainZombieGame();

	// Runs Zombie game
	void run();

private:

	// Initialize Systems
	void initSystems();

	// Initializes level and sets up shit
	void initLevel();

	// Initialize the shaders
	void initShaders();

	// Main Game Loop
	void gameLoop();

	// Updates all agents
	void updateAgents(float deltaTime);

	// Updates bullets
	void updateBullets(float deltaTime);

	// Check for victory
	void checkVictory();

	// Handles all Input Processing
	void processInput();

	// Renders the game
	void drawGame();

	// Draws the HUD
	void drawHud();

	// Draws the Blood particles
	void addBlood(const glm::vec2& position, int numParticles);

	// Member Variables
	Bengine::Window m_window; ///< The game window

	Bengine::GLSLProgram m_textureProgram; ///< The shader program

	Bengine::InputManager m_inputManager; ///< Handles input

	Bengine::Camera2D m_camera; ///< Main Camera
	Bengine::Camera2D m_hudCamera; ///< HUD Camera

	Bengine::SpriteBatch m_agentSpriteBatch; ///< Draws all agents
	Bengine::SpriteBatch m_hudSpriteBatch;
	
	Bengine::ParticleEngine2D m_particleEngine;
	Bengine::ParticleBatch2D* m_bloodParticleBatch;

	std::vector<Level*> m_levels; ///< Vector of all levels

	int m_screenWidth;
	int m_screenHeight;
	float m_fps;
	int m_currentLevel;

	Player* m_player;
	std::vector<Human*> m_humans; ///< Vector of all Humans
	std::vector<Zombie*> m_zombies; ///< Vector of all Zombies
	std::vector<Bullet0> m_bullets;

	int m_numHumansKilled; ///< Humans murdered by player
	int m_numZombiesKilled; ///< Zombies killed by player

	Bengine::SpriteFont* m_spriteFont;

	Bengine::AudioEngine m_audioEngine;

	GameState m_gameState;
};

