/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gráfico - Jogos Digitais - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 12/05/2023
 *
 */
#define STB_IMAGE_IMPLEMENTATION
#include "../../Common/include/stb_image.h"

#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>

#include "Shader.h"

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Um struct auxiliar para facilitar a criação dos floats com os atributos dos vertices
struct Vertex {
	float x, y, z;		// Positioning
	float s, t;			// Texture
	float r, g, b;		// Color
	float nx, ny, nz;   // Normal
};

struct Material {
	float kaR, kaG, kaB;   // Ka
	float kdR, kdG, kdB;   // Kd
	float ksR, ksG, ksB;   // Ks
	float ns;     
	std::string textureName; // map_Kd
};

// Struct auxiliar para agrupar os dados da curva de Bezier
struct BezierCurve {
	GLuint VAO;
	std::vector<glm::vec3> curvePoints;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Protótipos das funções
GLuint setupTexture(string path);
vector<Vertex> setupObj(string path);
Material setupMtl(string path);
void setupMtlUniforms(GLuint shaderProgram, Material material);
int setupGeometry(vector<Vertex>& vertices);
vector<glm::vec3> generateCircleControlPoints(glm::vec3 referencePoint, float radius);
GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints);
BezierCurve createBezierCurve(vector <glm::vec3> controlPoints, int pointsPerSegment);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX=false, rotateY=false, rotateZ=false;
float translateX = 0.0f, translateY = 0.0f, translateZ = 0.0f;
float scale = 0.3f;

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -3.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
bool firstMouse = true;
float lastX, lastY;
float sensitivity = 0.1f;
float cameraSpeed = 0.04f;
float pitch = 0.0, yaw = -90.0;

bool moveW = false, moveA = false, moveS = false, moveD = false;

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modulo 2 -- Vitor Hugo!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Planeta
	vector<Vertex> vertices = setupObj("../../3D_Models/Suzanne/bola.obj");
	GLuint VAO = setupGeometry(vertices);

	Material material = setupMtl("../../3D_Models/Suzanne/bola.mtl");
	GLuint textureID = setupTexture(material.textureName);

	// Lua
	vector<Vertex> verticesLua = setupObj("../../3D_Models/Planetas/planeta.obj");
	GLuint VAOLua = setupGeometry(verticesLua);

	Material materialLua = setupMtl("../../3D_Models/Planetas/planeta.mtl");
	GLuint textureIDLua = setupTexture(materialLua.textureName);

	// Compilando e buildando o programa de shader
	Shader objectShader = Shader("../shaders/Object.vs", "../shaders/Object.fs");
	glUseProgram(objectShader.getId());

	glUniform1i(glGetUniformLocation(objectShader.getId(), "texture1"), 0);

	setupMtlUniforms(objectShader.getId(), material);

	// Câmera
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 3.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(objectShader.getId(), "view"), 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(objectShader.getId(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(objectShader.getId(), "model");

	// Scale model do planeta
	glm::vec3 scaleModel = glm::vec3(10.0f, 10.0f, 10.0f);
	// Scale model da Lua
	glm::vec3 scaleModelLua = glm::vec3(3.0f, 3.0f, 3.0f);

	glm::vec3 translationModel = glm::vec3(1.0f, 1.0f, 1.0f);

	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	// Curvas

	// Aqui, o planetReferencePoint é o centro do VAO do planeta e vai ser usado para gerar o círculo de órbita da Lua
	// O centro dele é calculado a partir de todas as coordenadas os vertices (e depois por uma divisao)
	glm::vec3 planetReferencePoint;
	float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;
	for (Vertex vertex : vertices) {
		sumX += vertex.x;
		sumY += vertex.y;
		sumZ += vertex.z;
	}
	cout << sumX / vertices.size();
	planetReferencePoint = glm::vec3(sumX / vertices.size(), sumY / vertices.size(), sumZ / vertices.size());

	// Fonte de luz (Sol no futuro)
	glUniform3f(glGetUniformLocation(objectShader.getId(), "lightPos"), planetReferencePoint.x + 9.0f, planetReferencePoint.y, planetReferencePoint.z + 9.0f);
	glUniform3f(glGetUniformLocation(objectShader.getId(), "lightColor"), 1.0f, 1.0f, 1.0f);

	vector<glm::vec3> controlPoints = generateCircleControlPoints(planetReferencePoint, 3.0f);
	GLuint pointsVAO = generateControlPointsBuffer(controlPoints);

	BezierCurve moonOrbitCurve = createBezierCurve(controlPoints, 10000);
	Shader lineShader = Shader("../shaders/Line.vs", "../shaders/Line.fs");

	glUseProgram(lineShader.getId());

	glUniformMatrix4fv(glGetUniformLocation(lineShader.getId(), "view"), 1, GL_FALSE, glm::value_ptr(view));

	glUniformMatrix4fv(glGetUniformLocation(lineShader.getId(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glEnable(GL_DEPTH_TEST);

	int nbCurvePoints = moonOrbitCurve.curvePoints.size();
	int i = 0;
	float angle = 0.0f;

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//glLineWidth(10);
		glPointSize(10);

		glUseProgram(objectShader.getId());

		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(glGetUniformLocation(objectShader.getId(), "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Ao invés de atualizar a posição da câmera no callback de keyboard event, estou atualizando ela
		// aqui para criar a sensação de um movimento de câmera mais suave. No callback, apenas atualizo a(s)
		// tecla(s) que está/estão sendo apertada(s), e com base nisso faço os cálculos aqui, na atualização do frame
		// (atualiza muito mais rápido do que a chamada do callback do glfw)
		if (moveW) cameraPos += cameraFront * cameraSpeed;
		if (moveA) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (moveS) cameraPos -= cameraFront * cameraSpeed;
		if (moveD) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

		glUniform3f(glGetUniformLocation(objectShader.getId(), "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

		// O cálculo da posição leva em conta o valor digitado pelo teclado (translateX, Y e Z) + os valores de cada cubo do array
		// de cubos
		//translationModel = glm::vec3(translateX + cubes[i + 0], translateY + cubes[i + 1], translateZ + cubes[i + 2]);
		//model = glm::translate(model, translationModel);

		// glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));
		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES

		// Planeta

		model = glm::mat4(1);
		scaleModel = glm::vec3(scale, scale, scale);
		// O planeta é rotacionado a cada frame no eixo Y
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, scaleModel);
		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glBindVertexArray(0);

		// Lua

		model = glm::mat4(1);
		scaleModel = glm::vec3(scale / 3.0f, scale / 3.0f , scale / 3.0f);
		// A Lua é inicialmente posicionada no 0,0, então para posiciona-la no ponto atual da curva, uso o translate
		model = glm::translate(model, moonOrbitCurve.curvePoints[i]);
		model = glm::scale(model, scaleModel);
		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

		glBindVertexArray(VAOLua);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureIDLua);
		glDrawArrays(GL_TRIANGLES, 0, verticesLua.size());
		glBindVertexArray(0);

		// Curvas

		glUseProgram(lineShader.getId());

		glUniformMatrix4fv(glGetUniformLocation(lineShader.getId(), "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Curva vermelha do circulo
		glUniform4f(glGetUniformLocation(lineShader.getId(), "finalColor"), 1.0f, 0.0f, 0.0f, 1.0f);
		glBindVertexArray(moonOrbitCurve.VAO);
		glDrawArrays(GL_LINE_STRIP, 0, moonOrbitCurve.curvePoints.size());
		glBindVertexArray(0);

		// Pontos amarelos que indicam os pontos de controle
		glUniform4f(glGetUniformLocation(lineShader.getId(), "finalColor"), 1.0f, 1.0f, 0.0f, 1.0f);
		glBindVertexArray(pointsVAO);
		glDrawArrays(GL_POINTS, 0, controlPoints.size());
		glBindVertexArray(0);

		// Linhas verdes que conectam os pontos de controle
		glUniform4f(glGetUniformLocation(lineShader.getId(), "finalColor"), 0.0f, 1.0f, 0.0f, 1.0f);
		glBindVertexArray(pointsVAO);
		glDrawArrays(GL_LINE_STRIP, 0, controlPoints.size());
		glBindVertexArray(0);

		i = (i + 20) % nbCurvePoints;
		// Valor de angle é usado para rotacionar os objetos
		if (angle < 360.0f) angle += 0.1f;
		else angle = 0.0f;

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &moonOrbitCurve.VAO);
	glDeleteVertexArrays(1, &pointsVAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

vector<Vertex> setupObj(string path) {
	vector<Vertex> vertices;
	ifstream file(path);
	string line;
	vector<glm::vec3> temp_positions;
	vector<glm::vec2> temp_texcoords;
	vector<glm::vec3> temp_normals;

	if (!file.is_open()) {
		cerr << "Failed to open file" << path << endl;
		return vertices;
	}

	while (getline(file, line)) {
		istringstream ss(line);
		string type;
		ss >> type;

		if (type == "v") {
			glm::vec3 position;
			ss >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (type == "vt") {
			glm::vec2 texcoord;
			ss >> texcoord.x >> texcoord.y;
			temp_texcoords.push_back(texcoord);
		}
		else if (type == "vn") {
			glm::vec3 normal;
			ss >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (type == "f") {
			string vertex1, vertex2, vertex3;
			ss >> vertex1 >> vertex2 >> vertex3;
			int vIndex[3], tIndex[3], nIndex[3];

			for (int i = 0; i < 3; i++) {
				string vertex = (i == 0) ? vertex1 : (i == 1) ? vertex2 : vertex3;
				size_t pos1 = vertex.find('/');
				size_t pos2 = vertex.find('/', pos1 + 1);

				vIndex[i] = stoi(vertex.substr(0, pos1)) - 1;
				tIndex[i] = stoi(vertex.substr(pos1 + 1, pos2 - pos1 - 1)) - 1;
				nIndex[i] = stoi(vertex.substr(pos2 + 1)) - 1;
			}

			for (int i = 0; i < 3; i++) {
				Vertex vertex;
				vertex.x = temp_positions[vIndex[i]].x;
				vertex.y = temp_positions[vIndex[i]].y;
				vertex.z = temp_positions[vIndex[i]].z;

				vertex.s = temp_texcoords[tIndex[i]].x;
				vertex.t = temp_texcoords[tIndex[i]].y;

				vertex.nx = temp_normals[nIndex[i]].x;
				vertex.ny = temp_normals[nIndex[i]].y;
				vertex.nz = temp_normals[nIndex[i]].z;

				vertices.push_back(vertex);
			}
		}
	}

	file.close();
	return vertices;
}

Material setupMtl(string path) {
	string texturePath;
	ifstream file(path);
	string line;
	Material material;

	if (!file.is_open()) {
		cerr << "Failed to open file" << path << endl;
		return material;
	}

	while (getline(file, line)) {
		istringstream ss(line);
		string type;
		ss >> type;

		if (type == "Ka") {
			ss >> material.kaR >> material.kaG >> material.kaB;
		}
		else if (type == "Kd") {
			ss >> material.kdR >> material.kdG >> material.kdB;
		}
		else if (type == "Ks") {
			ss >> material.ksR >> material.ksG >> material.ksB;
		}
		else if (type == "Ns") {
			ss >> material.ns;
		}
		else if (type == "map_Kd") {
			ss >> material.textureName;
		}
	}

	file.close();
	return material;
}

void setupMtlUniforms(GLuint shaderProgram, Material material) {
	glUniform1f(glGetUniformLocation(shaderProgram, "kaR"), material.kaR);
	glUniform1f(glGetUniformLocation(shaderProgram, "kaG"), material.kaG);
	glUniform1f(glGetUniformLocation(shaderProgram, "kaB"), material.kaB);
	glUniform1f(glGetUniformLocation(shaderProgram, "kdR"), material.kdR);
	glUniform1f(glGetUniformLocation(shaderProgram, "kdG"), material.kdG);
	glUniform1f(glGetUniformLocation(shaderProgram, "kdB"), material.kdB);
	glUniform1f(glGetUniformLocation(shaderProgram, "ksR"), material.ksR);
	glUniform1f(glGetUniformLocation(shaderProgram, "ksG"), material.ksG);
	glUniform1f(glGetUniformLocation(shaderProgram, "ksB"), material.ksB);
	glUniform1f(glGetUniformLocation(shaderProgram, "ns"), material.ns);
}

GLuint setupTexture(string filename) {
	GLuint textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		if (nrChannels == 3) //jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else //png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D); // geração do mipmap
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
		
	}

	stbi_image_free(data);

	return textureId;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_W && action == GLFW_PRESS) moveW = true;
	if (key == GLFW_KEY_W && action == GLFW_RELEASE) moveW = false;
	if (key == GLFW_KEY_A && action == GLFW_PRESS) moveA = true;
	if (key == GLFW_KEY_A && action == GLFW_RELEASE) moveA = false;
	if (key == GLFW_KEY_S && action == GLFW_PRESS) moveS = true;
	if (key == GLFW_KEY_S && action == GLFW_RELEASE) moveS = false;
	if (key == GLFW_KEY_D && action == GLFW_PRESS) moveD = true;
	if (key == GLFW_KEY_D && action == GLFW_RELEASE) moveD = false;

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float offsetx = xpos - lastX;
	float offsety = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	offsetx *= sensitivity;
	offsety *= sensitivity;

	pitch += offsety;
	yaw += offsetx;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry(vector<Vertex>& vertices) {
	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Positioning
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Normal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

/**
	Gera pontos de controle que quando combinados, gerarão um círculo ao redor do ponto de referência
	("referencePoint") com raio "radius". O ponto de referencia é um valor no espaço 3d (x, y, z)
*/
vector<glm::vec3> generateCircleControlPoints(glm::vec3 referencePoint, float radius) {
	vector <glm::vec3> controlPoints;

	float refX = referencePoint.x;
	float refY = referencePoint.y;
	float refZ = referencePoint.z;

	// Como estou usando curvas de Bezier para fazer o círculo, alguns cálculos precisam ser feitos
	// Basicamente, 4 pontos são criados a partir do ponto de referência para formar um quadrado. A distância
	// do ponto de ref é o radius. Esse quadrado será usado para criar o círculo.
	// Depois que tenho os 4 pontos de controle, crio 2 pontos novos para cada 
	// (um posicionado "antes" e outro "depois" do ponto de controle). Esses pontos são usados como pontos
	// auxiliares para direcionar a curva. Assim, para criar o círculo, o primeiro e o último ponto são os pontos de controle
	// P0, P1, P2 e P3 e os pontos do meio que direcionam a curva. Assim, nessa ordem, são criados 4 curvas que formam os 4 "quadrantes"
	// ou segmentos do círculo que quero gerar. 

	// Esse valor é um valor específico que, quando multiplicado pelo raio que estou usando, dará as curvas que mais se parecerão
	// com os 4 segmentos de um círculo. Fonte: https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves
	float auxControlPoint = 0.552284749831f * radius;

	glm::vec3 P0 = glm::vec3(refX + radius, refY, refZ + radius);   // Top (vértice do quadrado)
	glm::vec3 P0Aux0 = glm::vec3(P0.x - auxControlPoint, P0.y, P0.z + auxControlPoint);
	glm::vec3 P0Aux1 = glm::vec3(P0.x + auxControlPoint, P0.y, P0.z - auxControlPoint);

	glm::vec3 P1 = glm::vec3(refX + radius, refY, refZ - radius);    // Left
	glm::vec3 P1Aux0 = glm::vec3(P1.x + auxControlPoint, P1.y, P1.z + auxControlPoint);
	glm::vec3 P1Aux1 = glm::vec3(P1.x - auxControlPoint, P1.y, P1.z - auxControlPoint);

	glm::vec3 P2 = glm::vec3(refX - radius, refY, refZ - radius);   // Bottom
	glm::vec3 P2Aux0 = glm::vec3(P2.x + auxControlPoint, P2.y, P2.z - auxControlPoint);
	glm::vec3 P2Aux1 = glm::vec3(P2.x - auxControlPoint, P2.y, P2.z + auxControlPoint);

	glm::vec3 P3 = glm::vec3(refX - radius, refY, refZ + radius);   // Right
	glm::vec3 P3Aux0 = glm::vec3(P3.x - auxControlPoint, P3.y, P3.z - auxControlPoint);
	glm::vec3 P3Aux1 = glm::vec3(P3.x + auxControlPoint, P3.y, P3.z + auxControlPoint);

	// Primeira curva do círculo (P0 a P1)...
	controlPoints.push_back(P0);
	controlPoints.push_back(P0Aux1);
	controlPoints.push_back(P1Aux0);
	controlPoints.push_back(P1);

	controlPoints.push_back(P1Aux1);
	controlPoints.push_back(P2Aux0);
	controlPoints.push_back(P2);

	controlPoints.push_back(P2Aux1);
	controlPoints.push_back(P3Aux0);
	controlPoints.push_back(P3);

	controlPoints.push_back(P3Aux1);
	controlPoints.push_back(P0Aux0);
	controlPoints.push_back(P0);

	return controlPoints;
}

GLuint generateControlPointsBuffer(vector <glm::vec3> controlPoints) {

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat) * 3, controlPoints.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

BezierCurve createBezierCurve(std::vector <glm::vec3> controlPoints, int pointsPerSegment) {

	glm::mat4 M = glm::mat4(
		-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0
	);

	BezierCurve bezierCurve;
	GLuint VAO;
	vector<glm::vec3> curvePoints;
	float step = 1.0 / (float)pointsPerSegment;
	float t = 0;
	int nControlPoints = controlPoints.size();

	for (int i = 0; i < nControlPoints - 3; i += 3)
	{
		for (float t = 0.0; t <= 1.0; t += step)
		{
			glm::vec3 p;

			glm::vec4 T(t * t * t, t * t, t, 1);

			glm::vec3 P0 = controlPoints[i];
			glm::vec3 P1 = controlPoints[i + 1];
			glm::vec3 P2 = controlPoints[i + 2];
			glm::vec3 P3 = controlPoints[i + 3];

			glm::mat4x3 G(P0, P1, P2, P3);

			p = G * M * T;  //---------

			curvePoints.push_back(p);
		}
	}


	//Gera o VAO
	GLuint VBO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(GLfloat) * 3, curvePoints.data(), GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	bezierCurve.VAO = VAO;
	bezierCurve.curvePoints = curvePoints;

	return bezierCurve;
}