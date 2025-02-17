#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"

#include <cmath>

//some globals
Mesh* mesh = NULL;
Mesh* mesh2 = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
Animation* anim = NULL;
float angle = 180;
float mouse_speed = 100.0f;
FBO* fbo = NULL;

Game* Game::instance = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	//load one texture without using the Texture Manager (Texture::Get would use the manager)
	texture = new Texture();
 	//texture->load("data/monster.jpg");  // monster
	texture->load("data/color.tga");  //backroom
	//texture->load("data/guy.png");  //  guy

	// example of loading Mesh from Mesh Manager
	//mesh = Mesh::Get("data/guy.mesh");
	//mesh = Mesh::Get("data/monster.mesh");
	mesh2 = Mesh::Get("data/back_part.obj"); //objeto peque�o 
	mesh = Mesh::Get("data/back_part.obj"); // objecto peque�o

	// example of shader loading using the shaders manager
	//phong le da max tetxura
	// 
	//shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture_phong.fs"); //animation
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture_phong.fs"); // object


	


	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
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
   

	//BACKROOM DE DOS PARTES
	Matrix44 m;
	//m.rotate(angle*DEG2RAD, Vector3(0, 1, 0));
	m.scale(20, 20, 20);

	//create model matrix for sphere
	Matrix44 m2;
	m2.translate(407, 0, 0);
	m2.rotate(angle* DEG2RAD, Vector3(0, 1, 0));
	//m2.rotateGlobal(angle * DEG2RAD, Vector3(0, 1, 0));
	m2.scale(20, 20, 20); //cambiar tama�o bola
	

	/*
	//ANIMACIONES PERSONAJES
	
	Matrix44 m;
	m.scale(20, 20, 20); //cambiar tama�o

	Animation* idle = Animation::Get("data/midle.skanim");
	//Animation* run = Animation::Get("data/prun.skanim");
	Animation* walk = Animation::Get("data/mwalk.skanim");

	//SOLO PARA UNO
	idle->assignTime(time);
	//run->assignTime(time);
	walk->assignTime(time);

	//PARA MAS DE UNO
	
	float t = fmod(time, idle->duration) / idle->duration;
	idle->assignTime(t * idle->duration);
	walk->assignTime(t * walk->duration);

	//minuto 1.15
	//float vel= //si se mueve 1 si no 0 // guy.isMoving()

	Skeleton resultSk;
	blendSkeleton(&idle->skeleton, &walk->skeleton, 1.0f, &resultSk); //el numero decide como se quedara la animacion, 0 es la primera, 1 es la segunda
	*/
	

	if(shader)
	{
		//enable shader
		shader->enable();

		//upload uniforms
		shader->setUniform("u_color", Vector4(1,1,1,1));
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix );
		shader->setUniform("u_texture", texture, 0);
		shader->setUniform("u_model", m);
		shader->setUniform("u_time", time);


		//ANIMATION CALL

		//mesh->renderAnimated( GL_TRIANGLES, &walk->skeleton);  // para uno
		//mesh->renderAnimated(GL_TRIANGLES, &resultSk); //combinar dos

		//OBJECT CALL
		mesh->render(GL_TRIANGLES);

		shader->setUniform("u_model", m2);
		mesh2->render(GL_TRIANGLES);
		
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
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	//example
	//angle += (float)seconds_elapsed * 10.0f;

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked ) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f,-1.0f,0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector( Vector3(-1.0f,0.0f,0.0f)));
	}

	//async input to move the camera around
	if(Input::isKeyPressed(SDL_SCANCODE_LSHIFT) ) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f,0.0f, 0.0f) * speed);

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
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

