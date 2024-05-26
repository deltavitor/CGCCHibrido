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

// Um struct auxiliar para facilitar a criação dos floats com os atributos dos vertices
struct Vertex {
	float x, y, z;		// Positioning
	float s, t;			// Texture
	float r, g, b;		// Color
};

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
GLuint setupTexture(string path);
vector<Vertex> setupObj(string path);
string setupMtl(string path);
int setupShader();
int setupGeometry(vector<Vertex>& vertices);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"layout (location = 2) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"out vec2 finalTexCoord;\n"
"void main()\n"
"{\n"
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"finalTexCoord = texCoord;\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 finalTexCoord;\n"
"in vec4 finalColor;\n"
"uniform sampler2D texture1;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = texture(texture1, finalTexCoord); // Por enquanto, os valores de cor não são usados\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;
float translateX = 0.0f, translateY = 0.0f, translateZ = 0.0f;
float scale = 0.4f;

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

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	vector<Vertex> vertices = setupObj("../../3D_Models/Suzanne/SuzanneTriTextured.obj");
	GLuint VAO = setupGeometry(vertices);

	// Será diferente no futuro quando os outros atributos do mtl forem incorporados
	string textureName = setupMtl("../../3D_Models/Suzanne/SuzanneTriTextured.mtl");
	GLuint textureID = setupTexture(textureName);

	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0);

	glUseProgram(shaderID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	// Models do glm para aumentar/diminuir e mover os vértices
	glm::vec3 scaleModel = glm::vec3(10.0f, 10.0f, 10.0f);

	glm::vec3 translationModel = glm::vec3(1.0f, 1.0f, 1.0f);

	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		//glLineWidth(10);
		//glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

		model = glm::mat4(1);

		scaleModel = glm::vec3(scale, scale, scale);
		model = glm::scale(model, scaleModel);

		if (rotateX)
		{
			model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));

		}
		else if (rotateY)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		}
		else if (rotateZ)
		{
			model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

		}

		// O cálculo da posição leva em conta o valor digitado pelo teclado (translateX, Y e Z) + os valores de cada cubo do array
		// de cubos
		//translationModel = glm::vec3(translateX + cubes[i + 0], translateY + cubes[i + 1], translateZ + cubes[i + 2]);
		//model = glm::translate(model, translationModel);

		glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));
		// Chamada de desenho - drawcall
		// Poligono Preenchido - GL_TRIANGLES

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() * 2);

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
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

				vertices.push_back(vertex);
			}
		}
	}

	file.close();
	return vertices;
}

// Por enquanto, estou lendo apenas o nome da textura e retornando esse valor, mas acho que no futuro essa função
// Ira retornar algo diferente que contenha as outras informações do mtl
string setupMtl(string path) {
	string texturePath;
	ifstream file(path);
	string line;

	if (!file.is_open()) {
		cerr << "Failed to open file" << path << endl;
		return "";
	}

	while (getline(file, line)) {
		istringstream ss(line);
		string type;
		ss >> type;

		if (type == "map_Kd") {
			ss >> texturePath;
		}
	}

	file.close();
	return texturePath;
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

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}
	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	// Por mais que as teclas para aumentar e diminuir sejam [ e ] no código,
	// na prática, se usa [ para aumentar e ´ para diminuir. Creio que isso ocorre em função
	// das diferenças no teclado americano e o ABNT
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		if (scale > 0.1f) scale -= 0.1f;
	}

	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
		if (scale < 2.0f) scale += 0.1f;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		translateX -= 0.1f;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		translateX += 0.1f;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		translateZ -= 0.1f;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		translateZ += 0.1f;
	}

	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		translateY -= 0.1f;
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		translateY += 0.1f;
	}

	// Reset da posição do cubo
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		translateX = 0.0f;
		translateY = 0.0f;
		translateZ = 0.0f;
	}

}

//Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no iniçio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
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

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

