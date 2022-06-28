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

Mesh* meshRoomFront = NULL;
Mesh* meshRoomBack = NULL;
Mesh* mesh = NULL;
Mesh* meshRoom = NULL;
Mesh* dimensionalPortal = NULL;
Texture* texture = new Texture();
Texture* texture2 = new Texture();
Texture* texturedimensionalPortal = new Texture();
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
int numMonsters=6;
int numPortals = 8;
float secondsPassed;
float vel;
bool isJumpingAnimation;
int raysPerDirectio = 1;
float checkfps;
int portalsReached = 0;
bool dead;
int num = 0;

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

float gravity = 9.8;

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
vector<Vector3> portalPositions;
vector<Entity*> dimensionalPortals;
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

void setPlayPosition() {
	
	character->position = Vector3(-1799.51, 0, -901.637);
}

void setPortalPosition() {

	character->position = Vector3(380.7, 0, -13.1936);
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


void positionadePortals() {
	dimensionalPortals.clear();
	for (int i = 0; i < numPortals; i++) {
		Entity* portal = new Entity();
		portal->mesh = Mesh::Get("data/salas/agujero.obj");
		portal->model = Matrix44();
		portal->texture = Texture::Get("data/salas/agujero.png");
		portal->position = portalPositions[i];
		dimensionalPortals.push_back(portal);
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
		shader->setUniform("u_texture", monster->texture, 0); 
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
	
	for each (Entity* dimensionalPortal in dimensionalPortals)
	{
		Matrix44 dimensionalModel;
		dimensionalPortal->model = dimensionalModel;
		dimensionalPortal->model.translate(dimensionalPortal->position.x, dimensionalPortal->position.y, dimensionalPortal->position.z);
		dimensionalPortal->model.rotate(DEG2RAD * 90, Vector3(0, 0, 1));
		dimensionalPortal->model.scale(50, 50, 50);
		shader->setUniform("u_texture", dimensionalPortal->texture, 0);
		shader->setUniform("u_model", dimensionalPortal->model);
		dimensionalPortal->mesh->render(GL_TRIANGLES);
	}
	

	roomBack->model = m;
	roomBack->model.translate(4099, 0, 0);
	roomBack->model.rotate(180 * DEG2RAD, Vector3(0, 1, 0));
	roomBack->model.scale(200, 200, 200); //cambiar tamaño bola
	shader->setUniform("u_texture", roomBack->texture, 0);
	shader->setUniform("u_model", roomBack->model);
	roomFront->mesh->render(GL_TRIANGLES);
	
	
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
	room->model.scale(400, 400, 400);
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
HSAMPLE LoadSample2(const char* fileName) {
	//El handler para un sample
	HSAMPLE hSample;

	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, fileName, 0, 0, 3, 0);
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

	HSAMPLE hSample = LoadSample(fileName);


	hSampleChannel = BASS_SampleGetChannel(hSample, false);


	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
	return hSampleChannel;
}

HCHANNEL PlaySound(const char* fileName) {

	HCHANNEL hSampleChannel;

	HSAMPLE hSample = LoadSample2(fileName);


	hSampleChannel = BASS_SampleGetChannel(hSample, false);


	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);
	return hSampleChannel;
}

bool isPortal() {
	
	return(distance(Vector3(428.69, 0, 578.108), character->position) <=50 || distance(Vector3(460.366, 0, 757.86), character->position)<=50);

}

void Init_Bass() {
	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		std::cout << "error init bass" << std::endl;
	}
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

		Texture* tex2 = Texture::Get("data/stages/historia.png");

		Texture* a = Texture::Get("data/stages/h1.png");
		Texture* b = Texture::Get("data/stages/h2.png");
		Texture* c = Texture::Get("data/stages/h3.png");
		Texture* d = Texture::Get("data/stages/h4.png");

		if (count == true) {
			RenderGUI(a_shader, tex);
		}
		else {
			if (num == 1) {
				RenderGUI(a_shader, a);
			}
			if (num == 2) {
				RenderGUI(a_shader, b);
			}
			if (num == 3) {
				RenderGUI(a_shader,c);
			}
			if (num == 4) {
				RenderGUI(a_shader, d);
			}
			
		}
		
		
	}

	void Update(float seconds_elapsed) {
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { 
					count = false;}

		if (Input::wasKeyPressed(SDL_SCANCODE_D) || Input::wasKeyPressed(SDL_SCANCODE_RIGHT)) {
			num += 1;

			if (num == 5) {
				count = true;
				num = 0;
			}
			
			
		}
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) {
					count = true;
					num = 0;
		}
		
		if (Input::isKeyPressed(SDL_SCANCODE_SPACE) & count == false) {
			STAGE_ID id = STAGE_ID::PORTAL;
			SetStage(id);
			setPortalPosition();
			currentMusic = PlayGameSound("data/music/sala.wav");
			BASS_ChannelSetAttribute(currentMusic, BASS_ATTRIB_VOL, 0.1);
				
		}
		
	}
};

class PortalStage : public Stage {
public:
	STAGE_ID GetId() {
		return STAGE_ID::PORTAL;
	}
	void Render(Camera* camera, float time) {


		character->model = Matrix44();

		character->model.translate(character->position.x-500, character->position.y+100, character->position.z-430);
		character->model.rotate(character->rotation * DEG2RAD*180, Vector3(0, 1, 0));

		camera->lookAt(character->model * Vector3(0, 70, -70), character->model * Vector3(0, 50, -5), Vector3(0, 1, 0));


		Animation* idle = Animation::Get("data/char/gidle.skanim");
		Animation* walk = Animation::Get("data/char/gwalk.skanim");


		//SOLO PARA UNO
		idle->assignTime(time);
		walk->assignTime(time);



		if (shader)
		{


			shaderAnimated->enable();

			//upload uniforms

			if (currentAnimation == "idle")
				RenderObject(camera, character, idle->skeleton);
			else if (currentAnimation == "walk")
				RenderObject(camera, character, walk->skeleton);
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
		
		vel = 0.0f;
		currentAnimation = "idle";
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) { character->speed = character->speed - forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) { character->speed = character->speed + forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) { character->speed = character->speed - right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { character->speed = character->speed + right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_C)) { character->rotation = character->rotation - 1 * seconds_elapsed; }
		if (Input::isKeyPressed(SDL_SCANCODE_V)) { character->rotation = character->rotation + 1 * seconds_elapsed; }
		if (Input::isKeyPressed(SDL_SCANCODE_P)) { cout << character->position.x << " - " << character->position.y << " - " << character->position.z << endl; }
		//to navigate with the mouse fixed in the middle



		newPos = newPos + character->speed * seconds_elapsed;


		if ((newPos.y + character->speed.y * seconds_elapsed) < 0) {
			character->speed.y = 0.0f;
			newPos.y = 0;
		}

		character->speed = character->speed - Vector3(0.0f, gravity, 0.0f);




		Vector3 character_center = newPos + Vector3(-500, 20.0f, -450.0f);
		bool isColliding = false;

		Vector3 collRay;
		Vector3 collnormRay;



		bool frontColl = (room->mesh->testRayCollision(room->model, character_center, forward, collRay, collnormRay, 10));
		bool backColl = (room->mesh->testRayCollision(room->model, character_center, -1 * forward, collRay, collnormRay, 10) );
		bool rightColl = (room->mesh->testRayCollision(room->model, character_center, right, collRay, collnormRay, 10) );
		bool leftColl = (room->mesh->testRayCollision(room->model, character_center, -1 * right, collRay, collnormRay, 10) );
		bool upColl = (room->mesh->testRayCollision(room->model, character_center, up, collRay, collnormRay, 10));
		bool downColl = (room->mesh->testRayCollision(room->model, character_center, -1 * up, collRay, collnormRay, 10));

		if (!(frontColl || backColl || rightColl || leftColl)) {
			character->position = newPos;
		}
		else {
			if (isPortal()) {
				STAGE_ID id = STAGE_ID::PLAY;
				SetStage(id);
				setPlayPosition();
				BASS_ChannelPause(currentMusic);
				currentMusic = PlayGameSound("data/music/room.mp3");
				
			}

		}

		
	}
};

class PlayStage : public Stage {
public:
	STAGE_ID GetId() {
		return STAGE_ID::PLAY;
	}
	void Render(Camera* camera, float time) {


		character->model = Matrix44();


		character->model.translate(character->position.x, character->position.y, character->position.z);
		character->model.rotate(character->rotation * DEG2RAD, Vector3(0, 1, 0));

		camera->lookAt(character->model * Vector3(0, 70, -70), character->model * Vector3(0, 50, -5), Vector3(0, 1, 0));


		Animation* idle = Animation::Get("data/char/gidle.skanim");
		Animation* walk = Animation::Get("data/char/gwalk.skanim");


		//SOLO PARA UNO
		idle->assignTime(time);
		walk->assignTime(time);



		if (shader)
		{


			//enable shader
			shaderAnimated->enable();

			//upload uniforms

			if (currentAnimation == "idle")
				RenderObject(camera, character, idle->skeleton);
			else if (currentAnimation == "walk")
				RenderObject(camera, character, walk->skeleton);
			shaderAnimated->disable();

			RenderMonster();

			RenderObject();




		}
	}

	void Update(float seconds_elapsed) {

		if (portalsReached == numPortals) {
			dead = false;
			STAGE_ID id = STAGE_ID::END;
			SetStage(id);
		}


		int fps = Game::instance->fps;
		if (fps < 40) {
			checkfps += seconds_elapsed;
			if (checkfps > 1.0f) {
				checkfps = 0.0f;
				raysPerDirectio += 1;
			}
		}
		if (fps > 60) {
			checkfps += seconds_elapsed;
			if (checkfps > 1.0f) {
				checkfps = 0.0f;
				if(raysPerDirectio>1)
					raysPerDirectio -= 1;
			}
		}
		for each (Monster * monster in monsters)
		{
			vector<int> angles;
			for (int i = 0; i < 360; i += raysPerDirectio) {
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

		

		Vector3 forward = character->model.rotateVector(Vector3(0, 0, -1));
		Vector3 right = character->model.rotateVector(Vector3(-1, 0, 0));
		Vector3 up = character->model.rotateVector(Vector3(0, -1, 0));


		character->speed = Vector3(0.0f, character->speed.y, 0.0f);
		Vector3 newPos = character->position;

		

		vel = 0.0f;
		currentAnimation = "idle";
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) { character->speed = character->speed - forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) { character->speed = character->speed + forward * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) { character->speed = character->speed - right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { character->speed = character->speed + right * character->movmentSpeed; vel = 1.0f; currentAnimation = "walk"; }
		if (Input::isKeyPressed(SDL_SCANCODE_C)) { character->rotation = character->rotation - 90 * seconds_elapsed;}
		if (Input::isKeyPressed(SDL_SCANCODE_V)) { character->rotation = character->rotation + 90 * seconds_elapsed;}
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
			bool frontColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 40.0f, 0.0f), forward, collRay, collnormRay, 10);
			bool backColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 40.0f, 0.0f), -1 * forward, collRay, collnormRay, 10);
			bool rightColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 40.0f, 0.0f), right, collRay, collnormRay, 10);
			bool leftColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 40.0f, 0.0f), -1 * right, collRay, collnormRay, 10);
			bool upColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 40.0f, 0.0f), up, collRay, collnormRay, 10);
			bool downColl = monster->mesh->testRayCollision(monster->model, character->position + Vector3(0, 40.0f, 0.0f), -1 * up, collRay, collnormRay, 10);

			if (frontColl || backColl || rightColl || leftColl) {
				PlaySound("data/music/chillido.wav");
				dead = true;
				STAGE_ID id = STAGE_ID::END;
				SetStage(id);
			}
		}

		int index = 0;
		for each (Entity* portal in dimensionalPortals)
		{
			bool frontColl = portal->mesh->testRayCollision(portal->model, newPos + Vector3(0, 40.0f, 0.0f), forward, collRay, collnormRay, 10);
			bool backColl = portal->mesh->testRayCollision(portal->model, newPos + Vector3(0, 40.0f, 0.0f), -1 * forward, collRay, collnormRay, 10);
			bool rightColl = portal->mesh->testRayCollision(portal->model, newPos + Vector3(0, 40.0f, 0.0f), right, collRay, collnormRay, 10);
			bool leftColl = portal->mesh->testRayCollision(portal->model, newPos + Vector3(0, 40.0f, 0.0f), -1 * right, collRay, collnormRay, 10);
			bool upColl = portal->mesh->testRayCollision(portal->model, newPos + Vector3(0, 40.0f, 0.0f), up, collRay, collnormRay, 10);
			bool downColl = portal->mesh->testRayCollision(portal->model, newPos + Vector3(0, 40.0f, 0.0f), -1 * up, collRay, collnormRay, 10);

			if (frontColl || backColl || rightColl || leftColl) {
				dimensionalPortals.erase(dimensionalPortals.begin() + index);
				portalsReached++;
			}
			index++;
		}

		


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

		Texture* tex2 = Texture::Get("data/stages/end2.png");

		if (dead) {
			RenderGUI(a_shader, tex2);
		}
		else {
			RenderGUI(a_shader, tex);
		}
		
		

	}

	void Update(float seconds_elapsed) {
		if (Input::isKeyPressed(SDL_SCANCODE_R)) {
			SetStage(STAGE_ID::PORTAL);
			setPortalPosition();
			positionadePortals();
			portalsReached = 0;
		}
	}
};


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


	portalPositions.push_back(Vector3(248.923, 70, -335.892));
	portalPositions.push_back(Vector3(-272.704, 70, -620.34));
	portalPositions.push_back(Vector3(1295.12, 70, -49.6268));
	portalPositions.push_back(Vector3(2754.64, 70, 397.593));
	portalPositions.push_back(Vector3(4637.31, 70, 648.944));
	portalPositions.push_back(Vector3(6057.67, 70, 817.688));
	portalPositions.push_back(Vector3(5115.37, 70, -1357.62));
	portalPositions.push_back(Vector3(-883.924, 70, 1363.92));
	positionadePortals();



	 //okay

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

	Texture* a = Texture::Get("data/stages/h1.png");
	Texture* b = Texture::Get("data/stages/h2.png");
	Texture* c = Texture::Get("data/stages/h3.png");
	Texture* d = Texture::Get("data/stages/h4.png");


	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse


	roomFront = new Entity();
	roomFront->model = Matrix44();
	roomFront->mesh = meshRoomFront;
	roomFront->texture = texture;

	roomBack = new Entity();
	roomBack->model = Matrix44();
	roomBack->mesh = meshRoomBack;
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
		monster->texture= Texture::Get("data/char/black.jpg");  //no encuentro donde se pone el color v:
		monster->detectedCounter = 0;
		monster->isDetecting = false;
		monster->rotation = 0;
		monsters.push_back(monster);

	}



	//BASS
	Init_Bass();
	
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
	drawText(750, 2, to_string(portalsReached)+"/"+to_string(numPortals), Vector3(0, 0, 0), 2);

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
		case SDLK_f: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
	
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
