/* Hello Triangle - c�digo adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Processamento Gr�fico - Jogos Digitais - Unisinos
 * Vers�o inicial: 7/4/2017
 * �ltima atualiza��o em 12/05/2023
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

// Um struct auxiliar para facilitar a cria��o dos floats com os atributos dos vertices
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

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Prot�tipo da fun��o de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Prot�tipos das fun��es
GLuint setupTexture(string path);
vector<Vertex> setupObj(string path);
Material setupMtl(string path);
void setupMtlUniforms(GLuint shaderProgram, Material material);
int setupShader();
int setupGeometry(vector<Vertex>& vertices);

// Dimens�es da janela (pode ser alterado em tempo de execu��o)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// C�digo fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec2 texCoord;\n"
"layout (location = 2) in vec3 color;\n"
"layout (location = 3) in vec3 normal;\n"
"uniform mat4 model;\n"
"out vec3 fragPos;\n"
"out vec4 finalColor;\n"
"out vec2 finalTexCoord;\n"
"out vec3 scaledNormal;\n"
"void main()\n"
"{\n"
"fragPos = vec3(model * vec4(position, 1.0));\n"
"gl_Position = vec4(fragPos, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"finalTexCoord = texCoord;\n"
"scaledNormal = normal;\n"
"}\0";

//C�difo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec3 fragPos;\n"
"in vec2 finalTexCoord;\n"
"in vec4 finalColor;\n"
"in vec3 scaledNormal;\n"
"uniform sampler2D texture1;\n"
"uniform float kaR, kaG, kaB;\n"
"uniform float kdR, kdG, kdB;\n"
"uniform float ksR, ksG, ksB;\n"
"uniform float ns;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"vec3 ambient = vec3(kaR, kaG, kaB) * lightColor;\n"
"vec3 N = normalize(scaledNormal);\n"
"vec3 L = normalize(lightPos - fragPos);\n"
"float diff = max(dot(N, L), 0.0);\n"
"vec3 diffuse = vec3(kdR, kdG, kdB) * diff * lightColor;\n"
"vec3 V = normalize(fragPos);\n"
"vec3 R = normalize(reflect(-L, N));\n"
"float spec = max(dot(R, V), 0.0);\n"
"spec = pow(spec, ns);\n"
"vec3 specular = vec3(ksR, ksG, ksB) * spec * lightColor;\n"
"vec4 finalTexture = texture(texture1, finalTexCoord);\n"
"vec3 result = (ambient + diffuse) * vec3(finalTexture.r, finalTexture.g, finalTexture.b) + specular;\n"
"color = vec4(result, 1.0);\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;
float translateX = 0.0f, translateY = 0.0f, translateZ = 0.0f;
float scale = 0.3f;

// Fun��o MAIN
int main()
{
	// Inicializa��o da GLFW
	glfwInit();

	//Muita aten��o aqui: alguns ambientes n�o aceitam essas configura��es
	//Voc� deve adaptar para a vers�o do OpenGL suportada por sua placa
	//Sugest�o: comente essas linhas de c�digo para desobrir a vers�o e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Cria��o da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modulo 2 -- Vitor Hugo!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da fun��o de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d fun��es da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informa��es de vers�o
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimens�es da viewport com as mesmas dimens�es da janela da aplica��o
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	vector<Vertex> vertices = setupObj("../../3D_Models/Suzanne/SuzanneTriTextured.obj");
	GLuint VAO = setupGeometry(vertices);

	Material material = setupMtl("../../3D_Models/Suzanne/SuzanneTriTextured.mtl");
	GLuint textureID = setupTexture(material.textureName);

	glUseProgram(shaderID);
	glUniform1i(glGetUniformLocation(shaderID, "texture1"), 0);

	setupMtlUniforms(shaderID, material);
	// Com esses valores, a SuzanneTriTextured fica com o rosto iluminado
	glUniform3f(glGetUniformLocation(shaderID, "lightPos"), 2.0f, 5.0f, -10.0f);
	glUniform3f(glGetUniformLocation(shaderID, "lightColor"), 1.0f, 1.0f, 0.0f);


	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	// Models do glm para aumentar/diminuir e mover os v�rtices
	glm::vec3 scaleModel = glm::vec3(10.0f, 10.0f, 10.0f);

	glm::vec3 translationModel = glm::vec3(1.0f, 1.0f, 1.0f);

	//
	model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);


	// Loop da aplica��o - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as fun��es de callback correspondentes
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

		// O c�lculo da posi��o leva em conta o valor digitado pelo teclado (translateX, Y e Z) + os valores de cada cubo do array
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
	// Finaliza a execu��o da GLFW, limpando os recursos alocados por ela
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
		glGenerateMipmap(GL_TEXTURE_2D); // gera��o do mipmap
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
		
	}

	stbi_image_free(data);

	return textureId;
}

// Fun��o de callback de teclado - s� pode ter uma inst�ncia (deve ser est�tica se
// estiver dentro de uma classe) - � chamada sempre que uma tecla for pressionada
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

	// Por mais que as teclas para aumentar e diminuir sejam [ e ] no c�digo,
	// na pr�tica, se usa [ para aumentar e � para diminuir. Creio que isso ocorre em fun��o
	// das diferen�as no teclado americano e o ABNT
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) {
		if (scale > 0.01f) scale -= 0.01f;
	}

	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) {
		if (scale < 2.0f) scale += 0.01f;
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

	// Reset da posi��o do cubo
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		translateX = 0.0f;
		translateY = 0.0f;
		translateZ = 0.0f;
	}

}

//Esta fun��o est� basntante hardcoded - objetivo � compilar e "buildar" um programa de
// shader simples e �nico neste exemplo de c�digo
// O c�digo fonte do vertex e fragment shader est� nos arrays vertexShaderSource e
// fragmentShader source no ini�io deste arquivo
// A fun��o retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compila��o (exibi��o via log no terminal)
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
	// Checando erros de compila��o (exibi��o via log no terminal)
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

// Esta fun��o est� bastante harcoded - objetivo � criar os buffers que armazenam a 
// geometria de um tri�ngulo
// Apenas atributo coordenada nos v�rtices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A fun��o retorna o identificador do VAO
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

