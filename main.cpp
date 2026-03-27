#include <SDL2/SDL.h>
#include <emscripten.h>
#include <SDL2/SDL_opengles2.h>
#include <iostream>
#include <vector>
#include <cmath>

// Global variables
SDL_Window* window = nullptr;
SDL_GLContext glContext = nullptr;
float angle = 0.f;
bool running = true;

// Shader program
GLuint program = 0;

// Vertex buffer objects
GLuint vbo = 0;
GLuint ibo = 0;

// Attribute locations
GLint a_position = -1;
GLint a_color = -1;
GLint u_mvp = -1;

// Cube vertices (position and color)
struct Vertex {
    float x, y, z;
    float r, g, b;
};

std::vector<Vertex> vertices;
std::vector<unsigned short> indices;

void initCube() {
    // Define cube vertices with colors
    vertices = {
        // Front face (red)
        {-1, -1,  1, 1, 0, 0},  // 0
        { 1, -1,  1, 1, 0, 0},  // 1
        { 1,  1,  1, 1, 0, 0},  // 2
        {-1,  1,  1, 1, 0, 0},  // 3
        
        // Back face (green)
        {-1, -1, -1, 0, 1, 0},  // 4
        {-1,  1, -1, 0, 1, 0},  // 5
        { 1,  1, -1, 0, 1, 0},  // 6
        { 1, -1, -1, 0, 1, 0},  // 7
        
        // Left face (blue)
        {-1, -1, -1, 0, 0, 1},  // 8
        {-1, -1,  1, 0, 0, 1},  // 9
        {-1,  1,  1, 0, 0, 1},  // 10
        {-1,  1, -1, 0, 0, 1},  // 11
        
        // Right face (yellow)
        { 1, -1, -1, 1, 1, 0},  // 12
        { 1,  1, -1, 1, 1, 0},  // 13
        { 1,  1,  1, 1, 1, 0},  // 14
        { 1, -1,  1, 1, 1, 0},  // 15
        
        // Top face (magenta)
        {-1,  1, -1, 1, 0, 1},  // 16
        {-1,  1,  1, 1, 0, 1},  // 17
        { 1,  1,  1, 1, 0, 1},  // 18
        { 1,  1, -1, 1, 0, 1},  // 19
        
        // Bottom face (cyan)
        {-1, -1, -1, 0, 1, 1},  // 20
        { 1, -1, -1, 0, 1, 1},  // 21
        { 1, -1,  1, 0, 1, 1},  // 22
        {-1, -1,  1, 0, 1, 1}   // 23
    };
    
    // Define indices for triangles (2 triangles per face)
    indices = {
        // Front
        0, 1, 2,  2, 3, 0,
        // Back
        4, 5, 6,  6, 7, 4,
        // Left
        8, 9, 10, 10, 11, 8,
        // Right
        12, 13, 14, 14, 15, 12,
        // Top
        16, 17, 18, 18, 19, 16,
        // Bottom
        20, 21, 22, 22, 23, 20
    };
}

GLuint createShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void initShaders() {
    const char* vertexShaderSource = 
        "attribute vec3 a_position;\n"
        "attribute vec3 a_color;\n"
        "varying vec3 v_color;\n"
        "uniform mat4 u_mvp;\n"
        "void main() {\n"
        "    gl_Position = u_mvp * vec4(a_position, 1.0);\n"
        "    v_color = a_color;\n"
        "}\n";
    
    const char* fragmentShaderSource = 
        "precision mediump float;\n"
        "varying vec3 v_color;\n"
        "void main() {\n"
        "    gl_FragColor = vec4(v_color, 1.0);\n"
        "}\n";
    
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    if (!vertexShader || !fragmentShader) {
        return;
    }
    
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Get attribute and uniform locations
    a_position = glGetAttribLocation(program, "a_position");
    a_color = glGetAttribLocation(program, "a_color");
    u_mvp = glGetUniformLocation(program, "u_mvp");
}

void initBuffers() {
    // Create VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    // Create IBO
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void setPerspective(float fov, float aspect, float near, float far) {
    float f = 1.0f / tan(fov * M_PI / 380.0f);
    float mvp[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far + near) / (near - far), -2,
        0, 0, (far * near) / (near - far), 0
    };
    
    // Apply rotation
    float rad = angle * M_PI / 180.0f;
    float cosA = cos(rad);
    float sinA = sin(rad);
    
    float rotMatrix[16] = {
        cosA, 0, -sinA, 0,
        0, 1, 0, 0,
        sinA, 0, cosA, 0,
        0, 0, 0, 1
    };
    
    float transMatrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -5,
        0, 0, 0, 1
    };
    
    // Multiply matrices: final = projection * rotation * translation
    float temp[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[i*4+j] = 0;
            for (int k = 0; k < 4; k++) {
                temp[i*4+j] += rotMatrix[i*4+k] * transMatrix[k*4+j];
            }
        }
    }
    
    float final[16];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            final[i*4+j] = 0;
            for (int k = 0; k < 4; k++) {
                final[i*4+j] += mvp[i*4+k] * temp[k*4+j];
            }
        }
    }
    
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, final);
}

void renderCube() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!program) return;
    
    glUseProgram(program);
    
    // Set up matrices
    float aspect = 800.0f / 600.0f;
    setPerspective(70.0f, aspect, 0.1f, 100.0f);
    
    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    // Position attribute
    glEnableVertexAttribArray(a_position);
    glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    
    // Color attribute
    glEnableVertexAttribArray(a_color);
    glVertexAttribPointer(a_color, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    
    // Bind IBO and draw
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
    
    // Disable attributes
    glDisableVertexAttribArray(a_position);
    glDisableVertexAttribArray(a_color);
    
    // Unbind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glUseProgram(0);
    
    SDL_GL_SwapWindow(window);
}

void mainLoop() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }
    }
    
    if (!running) {
        emscripten_cancel_main_loop();
        return;
    }
    
    // Update angle
    angle += 1.0f;
    if (angle > 360.0f) angle -= 360.0f;
    
    renderCube();
}

int main() {
    std::cout << "PixiModel - WebGL Version\n";
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Request OpenGL ES 2.0 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    window = SDL_CreateWindow(
        "PixiModel - Rotating Cube",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize cube data
    initCube();
    
    // Set up shaders and buffers
    initShaders();
    initBuffers();
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    emscripten_set_main_loop(mainLoop, 0, 1);
    
    // Cleanup
    if (vbo) glDeleteBuffers(1, &vbo);
    if (ibo) glDeleteBuffers(1, &ibo);
    if (program) glDeleteProgram(program);
    
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}