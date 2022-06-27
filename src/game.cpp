#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "libs/bass.h"
#include <cmath>
#include <cmath>

using namespace std;

//some globals

HCHANNEL currentMusic=NULL;
//std::map<std::string, HSAMPLE*> AudioManager::sSamplesLoaded;
//AudioManager* AudioManager::audio = NULL;

Mesh* meshRoomFront = NULL;
Mesh* meshRoomBack = NULL;
Mesh* mesh = NULL;
Mesh* meshRoom = NULL;
Texture* texture = new Texture();
Texture* texture2 = new Texture();
Mesh* mesh_texture = NULL;
Shader* shader = NULL;
Shader* shaderAnimated = NULL;
Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;
boolean canJump;
boolean collidingDown;
boolean isJumping;
int numObstacles;
int numMonsters;
float secondsPassed;
float vel;
bool isJumpingAnimation;

class Entity {
public:
	Matrix44 model;
	Vector3 position;
	Texture* texture;
	Mesh* mesh;

};

class Monster {
public:
	Matrix44 model;
	Vector3 position;
	Texture* texture;
	Mesh* mesh;
	Vector3 pivot;
	float detectedCounter;
	bool isDetecting;
	float rotation;

};
float gravity = 9.8f;
class Character {
public:
	Matrix44 model;
	Vector3 position;
	Vector3 speed;
	float rotation;
	float movmentSpeed;
	float jumpSpeed;
	Texture* texture;
	Mesh* mesh;
};

vector<Monster*> monsters;
vector<Vector3> monstersPositions;
Character* character;
vector<Entity*> obstacles;
Entity* roomFront;
Entity* roomBack;
Entity* room;
Skeleton idleWalk;
Skeleton idleJump;
Skeleton JumpWalk;
string currentAnimation;
Game* Game::instance = NULL;



enum STAGE_ID {
	INTRO = 0,
	TUTORIAL = 1,
	PORTAL = 2,
	PLAY = 3,
	END = 4
};


class Stage {
public:

	virtual STAGE_ID GetId() = 0;
	virtual void Render(Camera* camera, float time) = 0;
	virtual void Update(float seconds_elapsed) = 0;
};
vector<Stage*> stages;



STAGE_ID currentStage = STAGE_ID::INTRO;

Stage* GetStage(STAGE_ID id) {
	return stages[(int)id];
}

Stage* GetCurrentStage() {
	return GetStage(currentStage);
}

void SetStage(STAGE_ID id) {
	currentStage = id;
}

float distance(Vector3 point1, Vector3 point2) {
	float xDistance = pow((point1.x - point2.x), 2);
	float yDistance = pow((point1.y - point2.y), 2);
	float zDistance = pow((point1.z - point2.z), 2);




	float distance = sqrt(xDistance + yDistance + zDistance);

	return distance;
}

bool chechCorrectPosition(Vector3 position, float minDistance) {
	if (obstacles.size() <= 0) {
		return true;
	}
	else {
		bool isCorrect = true;
		for each (Entity* obstacle in obstacles)
		{
			float distancee = distance(&obstacle->position, &position);
		

		


			
			if (distancee < minDistance) {
				isCorrect = false;
			}
		}
		return isCorrect;
	}
	
}

void RenderMonster() {
	shader->enable();

	Camera* cam = Game::instance->camera;
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", cam->viewprojection_matrix);

	shader->setUniform("u_time", time);
	for each (Monster* monster in monsters)
	{
		Matrix44 m3;
		monster->model = m3;
		monster->model.translate(monster->position.x, monster->position.y, monster->position.z);
		shader->setUniform("u_texture", character->texture, 0);
		shader->setUniform("u_model", monster->model);
		monster->mesh->render(GL_TRIANGLES);
	}
	
	shader->disable();
}

void  RenderObject() {
	shader->enable();

	Camera* cam = Game::instance->camera;
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", cam->viewprojection_matrix);
	
	shader->setUniform("u_time", time);
	
	
	Matrix44 m;
	roomFront->model = m;
	roomFront->model.scale(200, 200, 200);
	shader->setUniform("u_texture", roomFront->texture, 0);
	shader->setUniform("u_model", roomFront->model);
	roomFront->mesh->render(GL_TRIANGLES);
	

	Matrix44 room2;
	roomBack->model = m;
	roomBack->model.translate(4099, 0, 0);
	roomBack->model.rotate(180 * DEG2RAD, Vector3(0, 1, 0));
	roomBack->model.scale(200, 200, 200); //cambiar tamaño bola
	shader->setUniform("u_texture", roomBack->texture, 0);
	shader->setUniform("u_model", roomBack->model);
	roomFront->mesh->render(GL_TRIANGLES);
	
	
	/*for each (Entity * obstacle in obstacles)
	{
		Matrix44 m;
		obstacle->model = m;
		shader->setUniform("u_texture", obstacle->texture, 0);
		obstacle->model.translate(obstacle->position.x, obstacle->position.y, obstacle->position.z);
		shader->setUniform("u_model", obstacle->model);
		obstacle->mesh->render(GL_TRIANGLES);

	}*/
	//do the draw call
	shader->disable();
}

void  RenderObject2() {
	shader->enable();

	Camera* cam = Game::instance->camera;
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", cam->viewprojection_matrix);

	shader->setUniform("u_time", time);


	Matrix44 m;
	room->model = m;
	room->model.scale(450, 400, 400);
	shader->setUniform("u_texture", room->texture, 0);
	shader->setUniform("u_model", room->model);
	room->mesh->render(GL_TRIANGLES);


	
	shader->disable();
}

void RenderObject(Camera* camera, Character* character, Skeleton resultSk) {
	shaderAnimated->setUniform("u_color", Vector4(1, 1, 1, 1));
	shaderAnimated->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shaderAnimated->setUniform("u_texture", character->texture, 0);
	shaderAnimated->setUniform("u_model", character->model);
	shaderAnimated->setUniform("u_time", time);

	//do the draw call
	shaderAnimated->enable();
	character->mesh->renderAnimated(GL_TRIANGLES, &resultSk); //combinar dos
	shaderAnimated->disable();

	character->mesh->render(GL_TRIANGLES);
}

void RenderGUI(Shader* s, Texture* t) {
	int windoww = Game::instance->window_width;
	int windowh = Game::instance->window_height;
	Mesh quad;
	quad.createQuad(windoww / 2, windowh / 2, windoww, windowh, false);

	Camera cam2D;
	cam2D.setOrthographic(0, windoww, windowh, 0, -1, 1);

	Shader* a_shader = s;
	Texture* tex = t;

	if (!a_shader) return;
	a_shader->enable();

	a_shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	a_shader->setUniform("u_viewprojection", cam2D.viewprojection_matrix);
	if (tex != NULL) {
		a_shader->setUniform("u_texture", tex, 0);
	}
	a_shader->setUniform("u_time", time);
	a_shader->setUniform("u_tex_tiling", 1.0f);
	a_shader->setUniform("u_model", Matrix44());

	quad.render(GL_TRIANGLES);

	a_shader->disable();
}

HSAMPLE LoadSample(const char* fileName) {
	//El handler para un sample
		HSAMPLE hSample;

		//El handler para un canal
		HCHANNEL hSampleChannel;

		//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
		//use BASS_SAMPLE_LOOP in the last param to have a looped sound
		hSample = BASS_SampleLoad(false, fileName, 0, 0, 3, BASS_SAMPLE_LOOP);
		if (hSample == 0)
		{
			//file not found
			std::cout << " + ERROR load " << fileName << std::endl;
		}
		std::cout << " + AUDIO load " << fileName << std::endl;
		return hSample;
}


HCHANNEL PlayGameSound(const char* fileName) {
	
	HCHANNEL hSampleChannel;

	HSAMPLE hSample= LoadSample(fileName);


	hSampleChannel = BASS_SampleGetChannel(hSample, false);
	

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
	return hSampleChannel;
}



class IntroStage : public Stage {
public:
	STAGE_ID GetId() {
		return STAGE_ID::INTRO;
	}
	void Render(Camera* camera, float time) {
		
		Shader* a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
		Texture* tex = Texture::Get("data/stages/titulo.png");

		RenderGUI(a_shader, tex);

	}

	void Update(float seconds_elapsed) {
		if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
			STAGE_ID id = STAGE_ID::TUTORIAL;
			SetStage(id);
		}
	}
};

class TutorialStage : public Stage {
public:
	STAGE_ID GetId() {
		return STAGE_ID::TUTORIAL;
	}
	bool count = true;

	void Render(Camera* camera, float time) {

		
		
		Shader* a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
		Texture* tex = Texture::Get("data/stages/keys.png");

		Shader* a_shader2 = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
		Texture* tex2 = Texture::Get("data/stages/historia.png");

		if (count == true) {
			RenderGUI(a_shader, tex);
		}
		else {
			RenderGUI(a_shader2, tex2);
		}
		
		
	}

	void Update(float seconds_elapsed) {
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { 
					count = false;}
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) {
					count = true;}

		if (Input::isKeyPressed(SDL_SCANCODE_SPACE) & count == false) {
			STAGE_ID id = STAGE_ID::PORTAL;
			SetStage(id);
			currentMusic = PlayGameSound("data/music/sala.wav");
		}
		
	}
};

class PortalStage : public Stage {
public:
	STAGE_ID GetId() {
		return STAGE_ID::PORTAL;
	}
	void Render(Camera* camera, float time) {

		//currentMusic = AudioManager::audio->play("data/music/sala.wav");
		//currentMusic = PlayGameSound("data/music/sala.wav");

		character->model = Matrix44();

		character->model.translate(character->position.x-500, character->position.y+100, character->position.z-430);
		character->model.rotate(character->rotation * DEG2RAD*180, Vector3(0, 1, 0));

		camera->lookAt(character->model * Vector3(0, 70, -70), character->model * Vector3(0, 50, -5), Vector3(0, 1, 0));


		Animation* idle = Animation::Get("data/char/gidle.skanim");
		Animation* walk = Animation::Get("data/char/gwalk.skanim");

		Animation* jump = Animation::Get("data/char/gjump.skanim");

		//SOLO PARA UNO
		idle->assignTime(time);
		walk->assignTime(time);

		jump->assignTime(time);


		if (shader)
		{


			shaderAnimated->enable();

			//upload uniforms

			if (currentAnimation == "idle")
				RenderObject(camera, character, idle->skeleton);
			else if (currentAnimation == "walk")
				RenderObject(camera, character, walk->skeleton);
			else if (currentAnimation == "jump")
				RenderObject(camera, character, jump->skeleton);
			shaderAnimated->disable();

			RenderObject2();




		}

	}

	void Update(float seconds_elapsed) {

		Vector3 forward = character->model.rotateVector(Vector3(0, 0, -1));
		Vector3 right = character->model.rotateVector(Vector3(-1, 0, 0));
		Vector3 up = character->model.rotateVector(Vector3(0, -1, 0));


		character->speed = Vector3(0.0f, character->speed.y, 0.0f);
		Vector3 newPos = character->position;
		if (Input::isKeyPressed(SDL_SCANCODE_J)) {
			if (canJump) {
				character->speed = character->speed + Vector3(0.0f, character->jumpSpeed, 0.0f);
				canJump = false;
				std::cout << " jump "  << std::endl;
			}
		}
		vel = 0.0f;
		currentAnimation = "idle";
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) { character->speed = character->speed - forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) { character->speed = character->speed + forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) { character->speed = character->speed - right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { character->speed = character->speed + right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_C)) { character->rotation = character->rotation - (90 * seconds_elapsed); }
		if (Input::isKeyPressed(SDL_SCANCODE_V)) { character->rotation = character->rotation + (90 * seconds_elapsed); }
		if (Input::isKeyPressed(SDL_SCANCODE_P)) { cout << character->position.x << " - " << character->position.y << " - " << character->position.z << endl; }
		//to navigate with the mouse fixed in the middle



		newPos = newPos + character->speed * seconds_elapsed;

		if ((newPos.y + character->speed.y * seconds_elapsed) < 0) {
			character->speed.y = 0.0f;
			newPos.y = 0;
		}

		character->speed = character->speed - Vector3(0.0f, gravity, 0.0f);




		Vector3 character_center = newPos + Vector3(0, 20.0f, 0);
		bool isColliding = false;

		Vector3 collRay;
		Vector3 collnormRay;



		bool frontColl = (room->mesh->testRayCollision(room->model, newPos, forward, collRay, collnormRay, 10) );
		bool backColl = (room->mesh->testRayCollision(room->model, newPos, -1 * forward, collRay, collnormRay, 10) );
		bool rightColl = (room->mesh->testRayCollision(room->model, newPos, right, collRay, collnormRay, 10) );
		bool leftColl = (room->mesh->testRayCollision(room->model, newPos, -1 * right, collRay, collnormRay, 10) );
		bool upColl = (room->mesh->testRayCollision(room->model, newPos, up, collRay, collnormRay, 10) );
		bool downColl = (room->mesh->testRayCollision(room->model, newPos, -1 * up, collRay, collnormRay, 10) );

		if (!(frontColl || backColl || rightColl || leftColl)) {
			character->position = newPos;
		}

		if (Input::isKeyPressed(SDL_SCANCODE_2)) {
			STAGE_ID id = STAGE_ID::PLAY;
			SetStage(id);
			BASS_ChannelPause(currentMusic);
			currentMusic = PlayGameSound("data/music/room.wav");
		}
	}
};

class PlayStage : public Stage {
public:
	STAGE_ID GetId() {
		return STAGE_ID::PLAY;
	}
	void Render(Camera* camera, float time) {

		//AudioManager::audio->stop(currentMusic);
		//currentMusic = AudioManager::audio->play("data/music/room.wav");
		//AudioManager::audio->play("data/music/room.wav");

		//BASS_ChannelPause(currentMusic);
		//currentMusic= PlayGameSound("data/music/room.wav");
		//PlayGameSound("data/music/room.wav");

		character->model = Matrix44();


		character->model.translate(character->position.x, character->position.y, character->position.z);
		character->model.rotate(character->rotation * DEG2RAD, Vector3(0, 1, 0));

		camera->lookAt(character->model * Vector3(0, 70, -70), character->model * Vector3(0, 50, -5), Vector3(0, 1, 0));



		//BACKROOM DE DOS PARTES

		//m.translate(0, -100, 0);
		//m.rotate(angleDEG2RAD, Vector3(0, 1, 0));


		//create model matrix for sphere




		Animation* idle = Animation::Get("data/char/gidle.skanim");
		//Animation* run = Animation::Get("data/prun.skanim");
		Animation* walk = Animation::Get("data/char/gwalk.skanim");

		Animation* jump = Animation::Get("data/char/gjump.skanim");

		//SOLO PARA UNO
		idle->assignTime(time);
		//run->assignTime(time);
		walk->assignTime(time);

		jump->assignTime(time);
		//PARA MAS DE UNO

		/*float t = fmod(time, idle->duration) / idle->duration;
		idle->assignTime(t * idle->duration);
		walk->assignTime(t * walk->duration);
		jump->assignTime(t * jump->duration);
		//minuto 1.15
		blendSkeleton(&idle->skeleton, &walk->skeleton, vel, &idleWalk); //el numero decide como se quedara la animacion, 0 es la primera, 1 es la segunda
		blendSkeleton(&idle->skeleton, &jump->skeleton, vel, &idleJump); //el numero decide como se quedara la animacion, 0 es la primera, 1 es la segunda
		blendSkeleton(&idle->skeleton, &jump->skeleton, vel, &idleJump); //el numero decide como se quedara la animacion, 0 es la primera, 1 es la segunda
		*/





		if (shader)
		{


			//enable shader
			shaderAnimated->enable();

			//upload uniforms

			if (currentAnimation == "idle")
				RenderObject(camera, character, idle->skeleton);
			else if (currentAnimation == "walk")
				RenderObject(camera, character, walk->skeleton);
			else if (currentAnimation == "jump")
				RenderObject(camera, character, jump->skeleton);
			shaderAnimated->disable();

			RenderMonster();

			RenderObject();


			//disable shader


		}
	}

	void Update(float seconds_elapsed) {
		for each (Monster * monster in monsters)
		{
			vector<int> angles;
			for (int i = 0; i < 360; i += 3) {
				Matrix44 modelAux;
				modelAux.translate(monster->position.x, monster->position.y, monster->position.z);
				//modelAux.rotate(-monster->rotation * DEG2RAD, Vector3(0, 1, 0));
				modelAux.rotate(i * DEG2RAD, Vector3(0, 1, 0));
				Vector3 directionRay = modelAux.frontVector();
				Vector3 collRay;
				Vector3 collnormRay;
				int distanceWall = 10000;
				int distanceCharacter = 100000;
				bool distanceWallAssigned = false;
				bool distanceCharacterAssigned = false;
				for (int j = 0; j < 10000; j += 200) {
					if (!distanceWallAssigned) {
						if (roomFront->mesh->testRayCollision(roomFront->model, monster->position - Vector3(0.0f, 40.0f, 0.0f), directionRay, collRay, collnormRay, j) || roomBack->mesh->testRayCollision(roomBack->model, monster->position - Vector3(0.0f, 40.0f, 0.0f), directionRay, collRay, collnormRay, j)) {
							distanceWall = j;
							distanceWallAssigned = true;
						}
					}

					if (character->mesh->testRayCollision(character->model, monster->position - Vector3(0.0f, 40.0f, 0.0f), directionRay, collRay, collnormRay, j)) {
						distanceCharacter = j;
						distanceCharacterAssigned = true;
					}

					if (distanceCharacterAssigned)
						break;
				}

				if (distanceCharacter < distanceWall) {
					monster->rotation = i;
					monster->isDetecting = true;
					monster->detectedCounter = 0;
				}

			}

			if (monster->detectedCounter > 1.5f) {
				monster->isDetecting = false;
				monster->detectedCounter = 0.0f;
			}

			if (monster->isDetecting) {
				monster->detectedCounter += seconds_elapsed;
				Vector3 directionMonster = Vector3(character->position.x - monster->position.x - 80.0f, 0.0f, character->position.z - monster->position.z - 17.0f);
				//cout <<"Character: " << character->position.x << " - " << character->position.z << endl;
				//cout << "Monster: " << monster->position.x << " - " << monster->position.z << endl;

				monster->position = monster->position + directionMonster.normalize() * seconds_elapsed * 200;
			}
			else {
				if (distance(Vector3(monster->position.x, 0.0f, monster->position.z), Vector3(monster->pivot.x, 0.0f, monster->pivot.z)) > 10.0f) {
					Vector3 directionMonster = Vector3(monster->pivot.x - monster->position.x - 80.0f, 0.0f, monster->pivot.z - monster->position.z - 17.0f);
					//cout <<"Character: " << character->position.x << " - " << character->position.z << endl;
					//cout << "Monster: " << monster->position.x << " - " << monster->position.z << endl;

					monster->position = monster->position + directionMonster.normalize() * seconds_elapsed * 200;

				}
			}





		}

		//cout << "Character Speed: " << character->speed.x << " - " << character->position.z << endl;


		//example

		//mouse input to rotate the cam
		

		//async input to move the camera around
		/*if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f,0.0f, 0.0f) * speed);
		*/

		Vector3 forward = character->model.rotateVector(Vector3(0, 0, -1));
		Vector3 right = character->model.rotateVector(Vector3(-1, 0, 0));
		Vector3 up = character->model.rotateVector(Vector3(0, -1, 0));


		character->speed = Vector3(0.0f, character->speed.y, 0.0f);
		Vector3 newPos = character->position;
		if (Input::isKeyPressed(SDL_SCANCODE_J)) {
			if (canJump) {
				character->speed = character->speed + Vector3(0.0f, character->jumpSpeed, 0.0f);
				canJump = false;
				std::cout << " jump2 " << std::endl;
			}
			
		}
		vel = 0.0f;
		currentAnimation = "idle";
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) { character->speed = character->speed - forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) { character->speed = character->speed + forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) { character->speed = character->speed - right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { character->speed = character->speed + right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_C)) { character->rotation = character->rotation - (90 * seconds_elapsed); }
		if (Input::isKeyPressed(SDL_SCANCODE_V)) { character->rotation = character->rotation + (90 * seconds_elapsed); }
		if (Input::isKeyPressed(SDL_SCANCODE_P)) { cout << character->position.x << " - " << character->position.y << " - " << character->position.z << endl; }
		//to navigate with the mouse fixed in the middle
		


		newPos = newPos + character->speed * seconds_elapsed;

		if ((newPos.y + character->speed.y * seconds_elapsed) < 0) {
			character->speed.y = 0.0f;
			newPos.y = 0;
		}

		character->speed = character->speed - Vector3(0.0f, gravity, 0.0f);




		Vector3 character_center = newPos + Vector3(0, 20.0f, 0);
		bool isColliding = false;

		Vector3 collRay;
		Vector3 collnormRay;



		bool frontColl = (roomFront->mesh->testRayCollision(roomFront->model, newPos, forward, collRay, collnormRay, 10) || roomBack->mesh->testRayCollision(roomBack->model, newPos, forward, collRay, collnormRay, 10));
		bool backColl = (roomFront->mesh->testRayCollision(roomFront->model, newPos, -1 * forward, collRay, collnormRay, 10) || roomBack->mesh->testRayCollision(roomBack->model, newPos, -1 * forward, collRay, collnormRay, 10));
		bool rightColl = (roomFront->mesh->testRayCollision(roomFront->model, newPos, right, collRay, collnormRay, 10) || roomBack->mesh->testRayCollision(roomBack->model, newPos, right, collRay, collnormRay, 10));
		bool leftColl = (roomFront->mesh->testRayCollision(roomFront->model, newPos, -1 * right, collRay, collnormRay, 10) || roomBack->mesh->testRayCollision(roomBack->model, newPos, -1 * right, collRay, collnormRay, 10));
		bool upColl = (roomFront->mesh->testRayCollision(roomFront->model, newPos, up, collRay, collnormRay, 10) || roomBack->mesh->testRayCollision(roomBack->model, newPos, up, collRay, collnormRay, 10));
		bool downColl = (roomFront->mesh->testRayCollision(roomFront->model, newPos, -1 * up, collRay, collnormRay, 10) || roomBack->mesh->testRayCollision(roomBack->model, newPos, -1 * up, collRay, collnormRay, 10));

		if (!(frontColl || backColl || rightColl || leftColl)) {
			character->position = newPos;
		}



		for each (Monster * monster in monsters)
		{
			bool frontColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 20.0f, 0.0f), forward, collRay, collnormRay, 10);
			bool backColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 20.0f, 0.0f), -1 * forward, collRay, collnormRay, 10);
			bool rightColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 20.0f, 0.0f), right, collRay, collnormRay, 10);
			bool leftColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 20.0f, 0.0f), -1 * right, collRay, collnormRay, 10);
			bool upColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 20.0f, 0.0f), up, collRay, collnormRay, 10);
			bool downColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 20.0f, 0.0f), -1 * up, collRay, collnormRay, 10);

			if (frontColl || backColl || rightColl || leftColl) {
				cout << "Dead" << endl;
			}
		}



		/*
		for each (Entity * obstacle in obstacles)
		{
			Vector3 coll;
			Vector3 collnorm;




			if (!(obstacle->mesh->testSphereCollision(obstacle->model, character_center, 19.f, coll, collnorm))) {
				gravity = 9.8f;
			}
			else {
				isColliding = true;
				character->speed.y = 0;
				gravity = 0;
			}

		}
		*/


	}
};

class EndStage : public Stage {
public:
	STAGE_ID GetId() {
		return STAGE_ID::END;
	}
	void Render(Camera* camera, float time) {

		Shader* a_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
		Texture* tex = Texture::Get("data/stages/end.png");

		
		RenderGUI(a_shader, tex);
		

	}

	void Update(float seconds_elapsed) {

	}
};




/*
//----------------------------------------AudioManager----------------------------------------//

AudioManager::AudioManager() {
	//Inicializamos BASS al arrancar el juego (id_del_device, muestras por segundo, ...)
	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		std::cout << "error init bass" << std::endl;
	}
};

HCHANNEL AudioManager::play(const char* filename) {

	assert(filename);

	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		std::cout << " + ERROR load " << filename << std::endl;
	}
	std::cout << " + AUDIO load " << filename << std::endl;
	

	//El handler para un sample
	HSAMPLE hSample = NULL;
	//El handler para un canal
	HCHANNEL hSampleChannel = NULL;

	//check if loaded
	std::map<std::string, HSAMPLE*>::iterator it = sSamplesLoaded.find(filename);

	if (it != sSamplesLoaded.end())
	{
		hSample = *it->second;
		hSampleChannel = BASS_SampleGetChannel(hSample, false);
		BASS_ChannelPlay(hSampleChannel, true);
		return hSampleChannel;
	}
}

void AudioManager::stop(HCHANNEL hSampleChannel) {

	BASS_ChannelPause(hSampleChannel);
}

void AudioManager::loadSamples() {

	hSample1 = BASS_SampleLoad(false, "data/music/room.wav", 0, 0, 20, BASS_SAMPLE_LOOP);
	sSamplesLoaded["data/music/room.wav"] = &hSample1;
	hSample2 = BASS_SampleLoad(false, "data/music/sala.wav", 0, 0, 20, BASS_SAMPLE_LOOP);
	sSamplesLoaded["data/music/sala.wav"] = &hSample2;
}
*/
Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	canJump = true;
	collidingDown = false;
	isJumping = false;
	numObstacles = 0;
	numMonsters = 6;

	monstersPositions.push_back(Vector3(-947.0f, 90.0f, 1390.0f));
	monstersPositions.push_back(Vector3(1062.46, 90, -169.231));
	monstersPositions.push_back(Vector3(2048.18, 90, 19.0251));
	monstersPositions.push_back(Vector3(4412.49, 90, 739.279));
	monstersPositions.push_back(Vector3(5665, 90, -684.677));
	monstersPositions.push_back(Vector3(776.768, 90, -1402.27));

	stages.push_back(new IntroStage());
	stages.push_back(new TutorialStage());
	stages.push_back(new PortalStage());
	stages.push_back(new PlayStage());
	stages.push_back(new EndStage());

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	character = new Character();
	character->model = Matrix44();
	character->position = Vector3(0.0f, 0.0f, 0.0f);
	character->movmentSpeed = 300.0f;
	character->jumpSpeed = 1000.0f;
	character->rotation = 180.0f;
	character->texture = new Texture();

	character->speed = Vector3(0.0f, 0.0f, 0.0f);
	currentAnimation = "idle";




	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f, 100.f, 100.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f, window_width / (float)window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	character->texture->load("data/char/guy.png"); //si quieres cargar la textura del prota


	// example of loading Mesh from Mesh Manager
	//mesh = Mesh::Get("data/box.ASE");


	meshRoomBack = Mesh::Get("data/salas/back_part.obj"); //objeto pequeño
	meshRoomFront = Mesh::Get("data/salas/back_part.obj");
	meshRoom= Mesh::Get("data/salas/sala.obj");
	character->mesh = Mesh::Get("data/char/guy.mesh");

	// example of shader loading using the shaders manager
	shaderAnimated = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture_phong.fs"); //animation
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture_phong.fs");
	texture = Texture::Get("data/salas/color.tga");
	texture2= Texture::Get("data/salas/sala.png");
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse


	roomFront = new Entity();
	roomFront->model = Matrix44();
	roomFront->mesh = meshRoomFront;
	//roomFront->mesh = Mesh::Get("data/back_part.obj");
	//roomFront->texture = Texture::Get("data/salas/color.tga");
	roomFront->texture = texture;

	roomBack = new Entity();
	roomBack->model = Matrix44();
	roomBack->mesh = meshRoomBack;
	//roomBack->mesh = Mesh::Get("data/back_part.obj");
	//roomBack->texture = Texture::Get("data/texture.tga");
	roomBack->texture = texture;

	
	room = new Entity();
	room->model = Matrix44();
	room->mesh = meshRoom;
	room->texture = texture2;

	for (int i = 0; i < numMonsters; i++) {
		Monster* monster = new Monster();
		monster->position = monstersPositions[i];
		monster->pivot = monster->position;
		monster->model = Matrix44();
		monster->mesh = Mesh::Get("data/char/monster.obj");
		//monster->texture= Texture::Get("data/char/black.jpg");  //no encuentro donde se pone el color v:
		monster->detectedCounter = 0;
		monster->isDetecting = false;
		monster->rotation = 0;
		monsters.push_back(monster);

	}


	//AudioManager::getInstance()->loadSamples();

	//AudioManager::audio->play("data/music/sala.wav");

	//BASS
	
	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		std::cout << "error init bass" << std::endl;
	}

	//currentMusic= PlayGameSound("data/music/sala.wav");
}




//what to do when the image has to be draw
void Game::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	GetCurrentStage()->Render(camera, time);
   
	//create model matrix for cube
	

	//Draw the floor grid
	drawGrid();


	//render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}


void Game::update(double seconds_elapsed)
{
	//example

	GetCurrentStage()->Update(seconds_elapsed);
	



	
}




//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
		case SDLK_j:canJump = true; break;
	}
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1 : 0.9;
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}
