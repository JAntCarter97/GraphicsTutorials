#pragma once

#include <vector>


namespace Bengine
{


// Forward declarations inside headers saves compiler time.
// Remember to #include in implementation files.
class ParticleBatch2D;
class SpriteBatch;



class ParticleEngine2D
{
public:
	ParticleEngine2D();
	~ParticleEngine2D();
	
	// After adding a particle batch, the ParticleEngine2D becomes
	// responsible for deallocation. 
	void addParicleBatch(ParticleBatch2D* particleBatch);

	void update(float deltaTime);

	void draw(SpriteBatch* spriteBatch);

private:
	std::vector<ParticleBatch2D*> m_batches;

};



}