#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

using namespace std;

const GLchar* vertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec3 position;\n"
"layout(location = 1) in vec2 texCoord;\n"
"out vec2 vertexTexCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"    vertexTexCoord = texCoord;\n"
"}\0";

const GLchar* fragmentShaderSource =
"#version 330 core\n"
"in vec2 vertexTexCoord;\n"
"out vec4 fragmentColor;\n"
"uniform sampler2D texture1;\n"
"uniform float mixValue;\n"
"void main()\n"
"{\n"
"    vec4 texColor1 = texture(texture1, vertexTexCoord);\n"
"    fragmentColor = mix(texColor1, vec4 (0.0,0.0,0.0,1.0), mixValue); \n"
"}\0";
int keyPressed = 0;
int triangleCount = 12; // Liczba trójkątów budujących koło
int numTriangles = 30;
int colorIndex = 0;
bool showBothShapes = false;
float mixValue = 0.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    mixValue += yoffset * 0.1f;
    mixValue = max(0.0f, min(1.0f, mixValue));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_1)
            keyPressed = 1;
        if (key == GLFW_KEY_2)
            keyPressed = 2;
        if (key == GLFW_KEY_3)
            keyPressed = 3;
    }
}

GLuint createShaderProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Sprawdź, czy kompilacja programu shadera przebiegła pomyślnie
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    return shaderProgram;
}
GLuint compileShader(const GLchar* source, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Sprawdź, czy kompilacja shadera przebiegła pomyślnie
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader compilation failed:\n" << infoLog << std::endl;
    }

    return shader;
}

GLuint loadTexture(const char* path)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, numChannels;
    unsigned char* data = stbi_load(path, &width, &height, &numChannels, 0);
    if (data)
    {
        GLenum format;
        if (numChannels == 1)
            format = GL_RED;
        else if (numChannels == 3)
            format = GL_RGB;
        else if (numChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Failed to load texture: " << path << endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void drawSquare(GLuint shaderProgram, GLuint texture1)
{
    GLfloat vertices[] = {
        // Positions     // Texture coordinates
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom left
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom right
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // Top right
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f    // Top left
    };
    GLuint indices[] = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);


    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void drawTriangle(GLuint shaderProgram, GLuint texture1)
{
    GLfloat vertices[] = {
        // Positions     // Texture coordinates
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom left
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom right
        0.0f, 0.5f, 0.0f, 0.5f, 1.0f    // Top center
    };
    GLuint indices[] = { 0, 1, 2 };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void drawReverseTriangle(GLuint shaderProgram, GLuint texture1)
{
    GLfloat vertices[] = {
        // Positions     // Texture coordinates
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom left
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom right
        0.0f, -1.0f, 0.0f, 0.5f, 1.0f    // Top center
    };
    GLuint indices[] = { 0, 1, 2 };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniform1f(glGetUniformLocation(shaderProgram, "mixValue"), mixValue);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

int main()
{
    // Initialize GLFW and OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    const unsigned int windowWidth = 1000;
    const unsigned int windowHeight = 800;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Computer Graphics", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Compile and link shaders
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    GLuint shaderProgram = createShaderProgram(vertexShader, fragmentShader);

    // Load textures
    GLuint texture1 = loadTexture("texture1.jpg");
    GLuint texture2 = loadTexture("texture2.jpg");
    // Configure OpenGL settings
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up input callbacks
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    
    //add default value for key pressed
    keyPressed = 1;

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (keyPressed == 1)
        {
            drawSquare(shaderProgram, texture1);
        }
        if (keyPressed == 2)
        {
            drawTriangle(shaderProgram, texture2);
        }
        if (keyPressed == 3)
        {
            drawSquare(shaderProgram, texture1);
            drawReverseTriangle(shaderProgram, texture2);
        }

        // Swap the buffers and poll for events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up resources
    glDeleteTextures(1, &texture1);
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}