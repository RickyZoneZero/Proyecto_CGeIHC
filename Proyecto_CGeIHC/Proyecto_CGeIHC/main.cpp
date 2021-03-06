/*Archivo de código correspondiente al proyecto de la asignatura de CGeIHC del semestre 2022-1
* Hernández Zamora José Enrique
* Jiménez Gutiérrez Miguel
*/
#include <Windows.h>

#include <glad/glad.h>
#include <glfw3.h>	//main
#include <stdlib.h>		
#include <glm/glm.hpp>	//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	//Texture

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>
#include <fstream>

//#pragma comment(lib, "winmm.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void my_input(GLFWwindow *window);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor *monitors;

void getResolution(void);

// camera
Camera camera(glm::vec3(0.0f, 30.0f, 200.0f));
float MovementSpeed = 0.1f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

//Lighting
glm::vec3 lightPosition(0.0f, 4.0f, -10.0f);
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);

// Posiciones segunda animación
float movimientoPez = 0.0f,
	  movimientoTorso = 0.0f,
	  rotBrazoPescador = 0.0f,
	  variable = 0.0f,
	  movimientoTren = 0.0f;
bool animacion_2 = false,
	 animacion_3 = false;

//Keyframes (Manipulación y dibujo)
float	posX = 0.0f,
		posY = 0.0f,
		posZ = 0.0f,
		rotBrazoDer = 0.0f,
		rotBrazoIzq = 0.0f,
		giroTorso = 0.0f,
		posX_2 = 0.0f,
		posY_2 = 0.0f,
		posZ_2 = 0.0f;
float	incX = 0.0f,
		incY = 0.0f,
		incZ = 0.0f,
		rotBrazoDerInc = 0.0f,
		rotBrazoIzqInc = 0.0f,
		giroTorsoInc = 0.0f,
		incX_2 = 0.0f,
		incY_2 = 0.0f,
		incZ_2 = 0.0f;

#define MAX_FRAMES 8
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float rotBrazoDer;
	float rotBrazoIzq;
	float giroTorso;
	float posX_2;
	float posY_2;
	float posZ_2;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 7;			//introducir datos
bool play = false;
int playIndex = 0;

void saveFrame(void)
{
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;

	KeyFrame[FrameIndex].rotBrazoDer = rotBrazoDer;
	KeyFrame[FrameIndex].rotBrazoIzq = rotBrazoIzq;
	KeyFrame[FrameIndex].giroTorso = giroTorso;

	KeyFrame[FrameIndex].posX_2 = posX_2;
	KeyFrame[FrameIndex].posY_2 = posY_2;
	KeyFrame[FrameIndex].posZ_2 = posZ_2;

	FrameIndex++;
}

void resetElements(void)
{
	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	rotBrazoDer = KeyFrame[0].rotBrazoDer;
	rotBrazoIzq = KeyFrame[0].rotBrazoIzq;
	giroTorso = KeyFrame[0].giroTorso;
	
	posX_2 = KeyFrame[0].posX_2;
	posY_2 = KeyFrame[0].posY_2;
	posZ_2 = KeyFrame[0].posZ_2;
}

void interpolation(void)
{
	incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;

	rotBrazoDerInc = (KeyFrame[playIndex + 1].rotBrazoDer - KeyFrame[playIndex].rotBrazoDer) / i_max_steps;
	rotBrazoIzqInc = (KeyFrame[playIndex + 1].rotBrazoIzq - KeyFrame[playIndex].rotBrazoIzq) / i_max_steps;
	giroTorsoInc = (KeyFrame[playIndex + 1].giroTorso - KeyFrame[playIndex].giroTorso) / i_max_steps;

	incX_2 = (KeyFrame[playIndex + 1].posX_2 - KeyFrame[playIndex].posX_2) / i_max_steps;
	incY_2 = (KeyFrame[playIndex + 1].posY_2 - KeyFrame[playIndex].posY_2) / i_max_steps;
	incZ_2 = (KeyFrame[playIndex + 1].posZ_2 - KeyFrame[playIndex].posZ_2) / i_max_steps;

}

void animate(void)
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
								  //Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;

			rotBrazoDer += rotBrazoDerInc;
			rotBrazoIzq += rotBrazoIzqInc;
			giroTorso += giroTorsoInc;

			posX_2 += incX_2;
			posY_2 += incY_2;
			posZ_2 += incZ_2;

			i_curr_steps++;
		}
	}

	//Pescador
	if (animacion_2)
	{
		if (movimientoPez >= 22.0f) {
			animacion_2 = false;
			movimientoPez = -10.0f;
			movimientoTorso = 0.0f;
			rotBrazoPescador = 0.0f;
		}
		else {
			movimientoPez += 0.2f;
			movimientoTorso = 1.0 * cos(variable);
			rotBrazoPescador = 5.0*cos(variable);
			variable += 0.5;
		}
	}
	//Tren
	if (animacion_3) {
		if (movimientoTren >= 500.0f) {
			animacion_3 = false;
			movimientoTren = 0.0f;
		}
		else {
			movimientoTren += 5.0;
		}
	}

}

void playMusic() {
	//Play Background music
	PlaySound(TEXT("resources/Music/Background_Music.wav"), NULL, SND_LOOP | SND_ASYNC);
}

void writeFile() {

	// Create and open a text file
	ofstream MyFile("resources/animation_files/animation_two.txt");

	// Write to the file
	MyFile.write((char*)&KeyFrame, sizeof(KeyFrame));
	cout << "Archivo escrito";

	// Close the file
	MyFile.close();
}

void readFile(string fileName) {
	int arraySize = sizeof(KeyFrame) / sizeof(KeyFrame[0]);

	// Read from the text file
	ifstream MyReadFile("resources/animation_files/"+ fileName +".txt");
	FRAME temp[MAX_FRAMES];

	MyReadFile.read((char*)&temp, sizeof(temp));

	for (int idx = 0; idx < arraySize; idx++)
	{
		KeyFrame[idx].posX = temp[idx].posX;
		KeyFrame[idx].posY = temp[idx].posY;
		KeyFrame[idx].posZ = temp[idx].posZ;
		KeyFrame[idx].rotBrazoDer = temp[idx].rotBrazoDer;
		KeyFrame[idx].rotBrazoIzq = temp[idx].rotBrazoIzq;
		KeyFrame[idx].giroTorso = temp[idx].giroTorso;
		KeyFrame[idx].posX_2 = temp[idx].posX_2;
		KeyFrame[idx].posY_2 = temp[idx].posY_2;
		KeyFrame[idx].posZ_2 = temp[idx].posZ_2;

		cout << "KeyFrame [" << idx << "].posX = " << temp[idx].posX << ";" << endl;
		cout << "KeyFrame [" << idx << "].posY = " << temp[idx].posY << ";" << endl;
		cout << "KeyFrame [" << idx << "].posZ = " << temp[idx].posZ << ";" << endl;
		cout << "KeyFrame [" << idx << "].rotBrazoDer = " << temp[idx].rotBrazoDer << ";" << endl;
		cout << "KeyFrame [" << idx << "].rotBrazoIzq = " << temp[idx].rotBrazoIzq << ";" << endl;
		cout << "KeyFrame [" << idx << "].giroTorso = " << temp[idx].giroTorso << ";" << endl;
		cout << "KeyFrame [" << idx << "].posX_2 = " << temp[idx].posX_2 << ";" << endl;
		cout << "KeyFrame [" << idx << "].posY_2 = " << temp[idx].posY_2 << ";" << endl;
		cout << "KeyFrame [" << idx << "].posZ_2 = " << temp[idx].posZ_2 << ";" << endl;
	}

	// Close the file
	MyReadFile.close();
}


void getResolution()
{	
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	// --------------------
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CGeIHC", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights.fs");
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

	vector<std::string> faces
	{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// load models
	// -----------
	Model piso("resources/objects/piso/piso.obj");
	Model cabana("resources/objects/CASA_FINAL/cabin.obj");
	Model carro("resources/objects/Carro/car.obj");
	Model pino("resources/objects/Pino/Spruce.obj");
	Model pinos("resources/objects/Pino/Pinos.obj");
	Model fogata("resources/objects/Fogata/camp_fire.obj");
	Model espacio("resources/objects/Fogata/space.obj");
	Model tunel("resources/objects/Tunel/Tunnel.obj");
	Model tren("resources/objects/Tren/train.obj");
	Model vias("resources/objects/Tren/rails.obj");
	Model torsoHombre("resources/objects/Hombre/Torso.obj");
	Model piernas("resources/objects/Hombre/Piernas.obj");
	Model brazoDerecho("resources/objects/Hombre/BrazoDerechoLego.obj");
	Model brazoIzquierdo("resources/objects/Hombre/BrazoIzquierdoLego.obj");
	Model piernasPescador("resources/objects/Pescador/piernas.obj");
	Model cuerpoPescador("resources/objects/Pescador/Lego.obj");
	Model brazoDerPescador("resources/objects/Pescador/brazoPescador.obj");
	Model pez("resources/objects/Pez/fish.obj");
	Model canoa("resources/objects/Canoa/boat.obj");
	Model cania("resources/objects/Pescador/fishingRod.obj");
	Model rioCongelado("resources/objects/Rio/rio_congelado.obj");
	Model canasta_baloncesto("resources/objects/Canasta_Baloncesto/Basketball_Board.obj");
	Model balon("resources/objects/Balon/basketball_OBJ.obj");
	Model baseColumpio("resources/objects/Columpio/base.obj");
	Model asientoColumpio("resources/objects/Columpio/asiento.obj");
	Model torsoLegoColumpio("resources/objects/Columpio/torsoLego.obj");
	Model piernasLegoColumpio("resources/objects/Columpio/piernasLego.obj");
	Model ventana("resources/objects/CASA_FINAL/ventana.obj");
	Model techo("resources/objects/CASA_FINAL/techo.obj");

	ModelAnim personaje_1("resources/objects/Personajes_Bailando/Personaje_1.dae");
	personaje_1.initShaders(animShader.ID);

	ModelAnim personaje_2("resources/objects/Personajes_Bailando/Personaje_2.dae");
	personaje_2.initShaders(animShader.ID);



	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		

		skyboxShader.setInt("skybox", 0);
		
		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		//my_input(window);
		animate();

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		staticShader.use();
		//Setup Advanced Lights
		staticShader.setVec3("viewPos", camera.Position);
		staticShader.setVec3("dirLight.direction", lightDirection);
		staticShader.setVec3("dirLight.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
		staticShader.setVec3("dirLight.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("dirLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));

		staticShader.setVec3("pointLight[0].position", lightPosition);
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[0].constant", 0.08f);
		staticShader.setFloat("pointLight[0].linear", 0.009f);
		staticShader.setFloat("pointLight[0].quadratic", 0.032f);

		staticShader.setVec3("pointLight[1].position", glm::vec3(-80.0, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[1].constant", 1.0f);
		staticShader.setFloat("pointLight[1].linear", 0.009f);
		staticShader.setFloat("pointLight[1].quadratic", 0.032f);

		staticShader.setFloat("material_shininess", 32.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 model2 = glm::mat4(1.0f);
		glm::mat4 model3 = glm::mat4(1.0f);
		glm::mat4 temporal = glm::mat4(1.0f);
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//// Light
		glm::vec3 lightColor = glm::vec3(0.6f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);
		

		// -------------------------------------------------------------------------------------------------------------------------
		// Animacion
		// -------------------------------------------------------------------------------------------------------------------------
		//Remember to activate the shader with the animation
		animShader.use();
		animShader.setMat4("projection", projection);
		animShader.setMat4("view", view);
	
		animShader.setVec3("material.specular", glm::vec3(0.5f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);

		// -------------------------------------------------------------------------------------------------------------------------
		// Personajes de animación
		// -------------------------------------------------------------------------------------------------------------------------

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-210.0f, -1.75f, -200.0f));
		model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		animShader.setMat4("model", model);
		personaje_1.Draw(animShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-250.0f, -1.75f, -200.0f));
		model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		animShader.setMat4("model", model);
		personaje_2.Draw(animShader);


		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);


		model = glm::mat4(1.0f);
		//Cabaña
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.75f, 0.0f));
		staticShader.setMat4("model", model);
		cabana.Draw(staticShader);

		model3 = glm::translate(model3, glm::vec3(0.0f, -40.0f, 0.0f));
		staticShader.setMat4("model", model3);
		techo.Draw(staticShader);
		//agregue esto
		//ventana
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, glm::vec3(29.0f, 31.0f, 111.5f));
		staticShader.setMat4("model", model2);
		ventana.Draw(staticShader);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		//Elementos exteriores
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f));
		staticShader.setMat4("model", model);
		piso.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-100.0f, -1.75f, 0.0f));
		staticShader.setMat4("model", model);
		carro.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, -1.75f, -250.0f));
		staticShader.setMat4("model", model);
		pino.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		pinos.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f - movimientoTren, -1.75f, -450.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		tren.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-340.0f, -1.75f, -450.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		tunel.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.75f, -450.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		vias.Draw(staticShader);

		//Fogata
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-230.0f, -1.75f, -200.0f));
		staticShader.setMat4("model", model);
		fogata.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-230.0f, -2.0f, -200.0f));
		staticShader.setMat4("model", model);
		espacio.Draw(staticShader);

		//Hombre jugando baloncesto
		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, 5.25f + posY, 350.0f));
		temporal = model = glm::rotate(model, glm::radians(giroTorso), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		torsoHombre.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, -1.75f + posY, 350.0f));
		staticShader.setMat4("model", model);
		piernas.Draw(staticShader);

		model = glm::translate(temporal, glm::vec3(-2.5f, 9.5f, 0.0f));
		model = glm::rotate(model, glm::radians(rotBrazoDer), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		brazoDerecho.Draw(staticShader);

		model = glm::translate(temporal, glm::vec3(2.5f, 9.5f, 0.0f));
		model = glm::rotate(model, glm::radians(rotBrazoIzq), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		brazoIzquierdo.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f + posX_2 , -1.75f + posY_2 , 350.0f + posZ_2));
		model = glm::scale(model, glm::vec3(0.8f));
		staticShader.setMat4("model", model);
		balon.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, -1.75f, 420.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		canasta_baloncesto.Draw(staticShader);
		

		//Pescador en lago congelado
		temporal = model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, -1.75f, 350.0f));
		model = glm::rotate(model, glm::radians(movimientoTorso), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		cuerpoPescador.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, -1.75f, 350.0f));
		staticShader.setMat4("model", model);
		piernasPescador.Draw(staticShader);

		model = glm::translate(temporal, glm::vec3(11.3f, 16.0f, 5.0f));
		model = glm::rotate(model, glm::radians(rotBrazoPescador), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		brazoDerPescador.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, -1.75f, 350.0f));
		staticShader.setMat4("model", model);
		canoa.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, -1.75f + movimientoPez, 350.0f));
		staticShader.setMat4("model", model);
		pez.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, -1.75f, 350.0f));
		staticShader.setMat4("model", model);
		cania.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, -2.0f, 350.0f));
		staticShader.setMat4("model", model);
		rioCongelado.Draw(staticShader);

		//Columpio
		model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, -1.75f, 350.0f));
		staticShader.setMat4("model", model);
		baseColumpio.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 45.0f, 350.0f));
		model = glm::rotate(model, glm::radians(posZ_2), glm::vec3(0.0f, 0.0f, 1.0f));
		staticShader.setMat4("model", model);
		asientoColumpio.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(155.0f + posX, -0.75f + posY, 350.0f + posZ));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		torsoLegoColumpio.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(155.0f + posX, 5.25f + posY, 350.0f + posZ));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(giroTorso), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		piernasLegoColumpio.Draw(staticShader);


		// -------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario
		// -------------------------------------------------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------------
		// draw skybox as last
		// -------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, view, projection, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		//std::cout <<"frame time = " << frameTime << " milli sec"<< std::endl;
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.Terminate();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);
	//Para configurar modelos
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		posZ++;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		posZ--;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		posX--;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		posX++;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		posY++;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		posY--;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		posZ_2++;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		posZ_2--;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		posX_2++;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		posX_2--;
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		posY_2++;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		posY_2--;
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		giroTorso++;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		giroTorso--;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		rotBrazoDer--;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		rotBrazoDer++;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		rotBrazoIzq--;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		rotBrazoIzq++;
	//Animación de hombre jugando baloncesto
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		readFile("animation_one");
	//Animación de hombre pescando
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		animacion_2 ^= true;
	//Animación movimiento tren
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		animacion_3 ^= true;
	//Animación columpio
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		readFile("animation_two");
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
		playMusic();
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
		writeFile();

	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}