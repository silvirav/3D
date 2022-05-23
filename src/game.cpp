#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"

#include <cmath>

using namespace std;

//some globals

Mesh* mesh = NULL;
Texture* texture = new Texture();
Mesh* mesh_texture = NULL;
Shader* shader = NULL;
Animation* anim = NULL;
float angle = 0;
float mouse_speed = 100.0f;
FBO* fbo = NULL;
boolean canJump;
boolean collidingDown;
boolean isJumping;
int numObstacles;

class Entity {
public:
	Matrix44 model;
	Vector3 position;
	Texture* texture;
	Mesh* mesh;

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

Character* character;
vector<Entity*> obstacles;

Game* Game::instance = NULL;

bool chechCorrectPosition(Vector3 position, float minDistance) {
	if (obstacles.size() <= 0) {
		return true;
	}
	else {
		bool isCorrect = true;
		for each (Entity * obstacle in obstacles)
		{
			float xDistance = pow((obstacle->position.x - position.x), 2);
			float yDistance = pow((obstacle->position.y - position.y), 2);
			float zDistance = pow((obstacle->position.z - position.z), 2);

		


			float distance = sqrt(xDistance + yDistance + zDistance);
			if (distance < minDistance) {
				isCorrect = false;
			}
		}
		return isCorrect;
	}
	
}

void  RenderObject(Camera* camera, Entity* entity) {
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_texture", texture, 0);
	shader->setUniform("u_model", entity->model);
	shader->setUniform("u_time", time);

	//do the draw call
	entity->mesh->render(GL_TRIANGLES);
}

void  RenderObject(Camera* camera, Character* character) {
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_texture", character->texture, 0);
	shader->setUniform("u_model", character->model);
	shader->setUniform("u_time", time);

	//do the draw call
	character->mesh->render(GL_TRIANGLES);
}

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
	numObstacles = 1;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;
	
	character =  new Character();
	character->model = Matrix44();
	character->position = Vector3(0.0f, 0.0f, 0.0f);
	character->movmentSpeed = 300.0f;
	character->jumpSpeed = 500.0f;
	character->rotation = 180.0f;
	character->texture = new Texture();

	character->speed = Vector3(0.0f,0.0f,0.0f);

	
	


	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	character->texture->load("data/guy2.png"); //si quieres cargar la textura del prota
	
	texture->load("data/texture.tga");

	// example of loading Mesh from Mesh Manager
	//mesh = Mesh::Get("data/box.ASE");
	
	character->mesh = Mesh::Get("data/guy.obj"); 
	mesh_texture = Mesh::Get("data/back.obj"); //objeto pequeño

	// example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse


	for (int i = 0; i < numObstacles; i++) {
		Entity* obstacle = new Entity();
		obstacle->model = Matrix44();
		obstacle->mesh = Mesh::Get("data/box.ASE");
		int tries = 0;
		Vector3 position = Vector3((rand() % 1000) - 500, (rand() % 50), (rand() % 1000) - 500);
		while (!chechCorrectPosition(position,100)) {
			position = Vector3((rand() % 1000) - 500, (rand() % 50), (rand() % 1000) - 500);
			cout << tries << endl;
		}
		obstacle->position = position;
		obstacles.push_back(obstacle);
	}
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
   
	//create model matrix for cube
	for each (Entity* obstacle in obstacles)
	{
		obstacle->model = Matrix44();
		obstacle->model.translate(obstacle->position.x, obstacle->position.y, obstacle->position.z);

	}
	character->model = Matrix44();


	character->model.translate(character->position.x, character->position.y, character->position.z);
	character->model.rotate(character->rotation * DEG2RAD, Vector3(0, 1, 0));

	camera->lookAt(character->model * Vector3(0, 70, -70),character->model * Vector3(0,50,-5),Vector3(0,1,0));

	

	Matrix44 m2;
	m2.translate(150, 0, 0); //esto gira con el objeto 1
	m2.scale(40, 40, 40); //aumentar tamaño si pequeño
	if(shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		
		RenderObject(camera, character);
		for each (Entity* obstacle in obstacles)
		{
			RenderObject(camera, obstacle);

		}


		//disable shader
		shader->disable();


	}

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

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
	}

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
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) { 
		if (canJump) {
			character->speed = character->speed + Vector3(0.0f, character->jumpSpeed, 0.0f);
			canJump = false;
		}
	}
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) { character->speed = character->speed - forward * character->movmentSpeed; }
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) { character->speed = character->speed + forward * character->movmentSpeed; }
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) { character->speed = character->speed - right * character->movmentSpeed;}
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { character->speed = character->speed + right * character->movmentSpeed; }
	if (Input::isKeyPressed(SDL_SCANCODE_C)) { character->rotation = character->rotation - (90 * seconds_elapsed); }
	if (Input::isKeyPressed(SDL_SCANCODE_V)) { character->rotation = character->rotation + (90 * seconds_elapsed); }
		//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();

	
	newPos = newPos + (character->speed * seconds_elapsed);

	if ((newPos.y + character->speed.y * seconds_elapsed) < 0) {
		character->speed.y = 0.0f;
		newPos.y = 0;
	}
	 
	character->speed = character->speed - Vector3(0.0f, gravity, 0.0f);
		 
	 
		

	Vector3 character_center = newPos + Vector3(0,20.0f,0);
	Vector3 coll;
	Vector3 collnorm;
	for each (Entity* obstacle in obstacles)
	{
		if (!(obstacle->mesh->testSphereCollision(obstacle->model, character_center, 19.f, coll, collnorm))) {
			character->position = newPos;
			gravity = 9.8f;
		}
		else {

			character->speed.y = 0;
			gravity = 0;
		}

	}
		
		
		
	
	
	

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
		case SDLK_SPACE:canJump = true; break;
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

