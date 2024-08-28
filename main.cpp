#define API_COSMIC_WEB
#include "include/GL/glew.h"
#include "include/GLFW/glfw3.h"
#include <gl/gl.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "include/stb_truetype.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <cstdlib> // For system() function
#include <cmath>

// Structure to hold character information
struct Character {
    GLuint TextureID;  // ID handle of the glyph texture
    unsigned int Width, Height;  // Size of glyph
    int BearingX, BearingY;  // Offset from baseline to left/top of glyph
    unsigned int Advance;  // Offset to advance to next glyph
};

std::map<char, Character> Characters;

// Function prototypes
void handleInput(GLFWwindow* window, std::string &inputText, bool &quit);
void renderText(const std::string& text, float x, float y, float scale, GLuint program, GLuint VAO, GLuint VBO, std::map<char, Character>& Characters);

// Global variables for shaders and VAO/VBO
GLuint program;
GLuint VAO, VBO;

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = texture(text, TexCoords);
    color = vec4(textColor, 1.0) * sampled;
}
)";

// Compile and link shaders into a program
GLuint compileShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Load and render text using stb_truetype
void loadFont(const char* fontPath) {
    // Read font file into buffer
    std::ifstream fontFile(fontPath, std::ios::binary);
    std::vector<unsigned char> fontBuffer((std::istreambuf_iterator<char>(fontFile)), std::istreambuf_iterator<char>());

    // Initialize stb_truetype
    stbtt_fontinfo font;
    stbtt_InitFont(&font, fontBuffer.data(), stbtt_GetFontOffsetForIndex(fontBuffer.data(), 0));

    int fontSize = 48;
    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++) {
        int width, height, xoff, yoff;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &xoff, &yoff);
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &width, &height, &xoff, &yoff);

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            width,
            height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            bitmap
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            static_cast<unsigned int>(width),
            static_cast<unsigned int>(height),
            xoff,
            yoff,
            static_cast<unsigned int>(stbtt_GetCodepointKernAdvance(&font, c, c))
        };
        Characters[c] = character;

        // Free bitmap memory
        stbtt_FreeBitmap(bitmap, nullptr);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Handle input events and update the search text
void handleInput(GLFWwindow* window, std::string &inputText, bool &quit) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        quit = true;
    }

    if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS && !inputText.empty()) {
        inputText.pop_back();
    }

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        std::string searchQuery = "https://www.google.com/search?q=" + inputText;
        std::string command = "xdg-open \"" + searchQuery + "\"";  // For Linux
        // std::string command = "start " + searchQuery;  // For Windows
        // std::string command = "open " + searchQuery;  // For macOS

        // Execute the command
        system(command.c_str());

        std::cout << "Searching for: " << inputText << std::endl;
        inputText.clear();  // Clear the text after "searching"
    }

    // Handle other keys (for example, adding characters to inputText)
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        inputText += 'a';
    }
    // Handle other keys...
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Web Browser Home Page", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Compile shaders and use the resulting shader program
    program = compileShaders();
    glUseProgram(program);

    // Load font and characters using stb_truetype
    loadFont("arial.ttf");

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Main loop
    bool quit = false;
    std::string inputText = "";

    while (!glfwWindowShouldClose(window) && !quit) {
        // Handle input
        handleInput(window, inputText, quit);

        // Render
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background
        glClear(GL_COLOR_BUFFER_BIT);

        // Render search bar
        renderText(inputText, 0.0f, 0.5f, 1.0f, program, VAO, VBO, Characters);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (auto& c : Characters) {
        glDeleteTextures(1, &c.second.TextureID);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}

// Render text using OpenGL and stb_truetype
void renderText(const std::string& text, float x, float y, float scale, GLuint program, GLuint VAO, GLuint VBO, std::map<char, Character>& Characters) {
    glUseProgram(program);
    glUniform3f(glGetUniformLocation(program, "textColor"), 0.0f, 0.0f, 0.0f);  // Black color
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (char c : text) {
        Character ch = Characters[c];

        float xpos = x + ch.BearingX * scale;
        float ypos = y - (ch.Height - ch.BearingY) * scale;

        float w = ch.Width * scale;
        float h = ch.Height * scale;
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Advance cursors for next glyph (advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
