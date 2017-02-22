#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>

double PI = 4.0*atan(1.0);

GLFWwindow* window;
double prevx = -1, prevy = -1;
double resx = 1920,resy = 1080;

// "camera": specify x and y coordinate of lower left pixel
double dy = 2.0;                    // height
double dx = dy*resx/float(resy);    // scaled width
double minx = -2.5;                 // x from -2.0 to 0.5
double miny = -1.0;                 // y from -1.0 to 1.0

int clickedButtons = 0;
enum buttonMaps { FIRST_BUTTON=1, SECOND_BUTTON=2, THIRD_BUTTON=4, FOURTH_BUTTON=8, FIFTH_BUTTON=16, NO_BUTTON=0 };
enum modifierMaps { CTRL=2, SHIFT=1, ALT=4, META=8, NO_MODIFIER=0 };

GLuint programID, computeProgramID;
GLuint VertexArrayID;

int texSizeX = 1920, texSizeY = 1080;

static const GLfloat g_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,
};

static const GLfloat g_uv_buffer_data[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
};

GLuint vertexbuffer, uvbuffer;

char *readFile(const char *filename) {
    printf("Reading %s\n", filename);
    FILE *f = fopen(filename, "rb");

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("Filesize = %d\n", int(fsize));

    char *string = (char*)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    string[fsize] = '\0';
    fclose(f);

    return string;
}

GLuint LoadComputeShader(const char * compute_file_path, GLuint texHandle){

    // Create the shaders
    GLuint ComputeShaderID   = glCreateShader(GL_COMPUTE_SHADER);
    char *ComputeShaderCode   = readFile(compute_file_path);


    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", compute_file_path);
    glShaderSource(ComputeShaderID, 1, (const char**)&ComputeShaderCode , NULL);
    glCompileShader(ComputeShaderID);

    // Check Vertex Shader
    glGetShaderiv(ComputeShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(ComputeShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( InfoLogLength > 0 ){
        char ComputeShaderErrorMessage[InfoLogLength];
        glGetShaderInfoLog(ComputeShaderID, InfoLogLength, NULL, ComputeShaderErrorMessage);
        printf("%s\n", ComputeShaderErrorMessage);
    }


    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, ComputeShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( InfoLogLength > 0 ){
        GLchar ProgramErrorMessage[InfoLogLength+1];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDeleteShader(ComputeShaderID);

    glUseProgram(ProgramID);
    
    glUniform1i(glGetUniformLocation(ProgramID, "destTex"), 0);

    return ProgramID;
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

    // Create the shaders
    GLuint VertexShaderID   = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    char *VertexShaderCode   = readFile(vertex_file_path);
    char *FragmentShaderCode = readFile(fragment_file_path);


    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    glShaderSource(VertexShaderID, 1, (const char**)&VertexShaderCode , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( InfoLogLength > 0 ){
        char VertexShaderErrorMessage[InfoLogLength];
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, VertexShaderErrorMessage);
        printf("%s\n", VertexShaderErrorMessage);
    }


    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    glShaderSource(FragmentShaderID, 1, (const char**)&FragmentShaderCode , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        char FragmentShaderErrorMessage[InfoLogLength];
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, FragmentShaderErrorMessage);
        printf("%s\n", FragmentShaderErrorMessage);
    }


    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

    if ( InfoLogLength > 0 ){
        GLchar ProgramErrorMessage[InfoLogLength+1];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}


void initGL();
void Draw();

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void mousebutton_callback(GLFWwindow* win, int button, int action, int mods);
void mousepos_callback(GLFWwindow* win, double xpos, double ypos);
void mousewheel_callback(GLFWwindow* win, double xoffset, double yoffset);
void windowsize_callback(GLFWwindow *win, int width, int height);

GLuint texHandle;


int main() {
    initGL();

    while ( !glfwWindowShouldClose(window)) {   
        Draw();
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);

    glfwTerminate();

    return 0;
}

void Draw() {
    static int frame  = 0;
    glUseProgram(computeProgramID);
    glUniform1f(glGetUniformLocation(computeProgramID, "roll"), (float)(frame++)*0.01f);
    GLint loc;

    loc = glGetUniformLocation(computeProgramID, "resx");
    if (loc != -1) glUniform1f(loc, resx);
    loc = glGetUniformLocation(computeProgramID, "resy");
    if (loc != -1) glUniform1f(loc, resy);
    loc = glGetUniformLocation(computeProgramID, "minx");
    if (loc != -1) glUniform1f(loc, minx);
    loc = glGetUniformLocation(computeProgramID, "miny");
    if (loc != -1) glUniform1f(loc, miny);
    loc = glGetUniformLocation(computeProgramID, "dx");
    if (loc != -1) glUniform1f(loc, dx);
    loc = glGetUniformLocation(computeProgramID, "dy");
    if (loc != -1) glUniform1f(loc, dy);

    glDispatchCompute(resx/8, resy/8, 1);
    
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(programID);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    // Swap buffers
    glfwSwapBuffers(window);
}
void initGL() {
    printf("Initializing OpenGL/GLFW\n"); 
    if (!glfwInit()) {
        printf("Could not initialize\n");
        exit(-1);
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(resx, resy, "Mandelbrot", 0, 0);
    if (!window) {
        printf("Could not open glfw window\n");
        glfwTerminate();
        exit(-2);
    }
    glfwMakeContextCurrent(window); 

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(-3);
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);


    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "vertex_shader.vs", "fragment_shader.fs" );

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

    
    glGenTextures(1, &texHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resx, resy, 0, GL_RED, GL_FLOAT, NULL);
    glBindImageTexture(0, texHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    computeProgramID = LoadComputeShader("compute_shader.cs", texHandle);

    glfwSwapInterval(1);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mousebutton_callback);
    glfwSetScrollCallback(window, mousewheel_callback);
    glfwSetCursorPosCallback(window, mousepos_callback);
    glfwSetWindowSizeCallback(window, windowsize_callback);

}


void windowsize_callback(GLFWwindow *win, int width, int height) {
    // called if the window size is changed
    resx = width;
    resy = height;
    dy = 2.0;                    // height
    dx = dy*resx/double(resy);    // scaled width

    glViewport(0, 0, resx, resy);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resx, resy, 0, GL_RED, GL_FLOAT, NULL);

    
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }
}

void mousebutton_callback(GLFWwindow* win, int button, int action, int mods) {
    glfwGetCursorPos(win,&prevx,&prevy);

    if (action == 1)
        clickedButtons |= (1 << button);
    else
        clickedButtons &= ~(1 << button);

    if (clickedButtons&FIRST_BUTTON) {
        
    } else if (clickedButtons&SECOND_BUTTON) {

    } else if (clickedButtons&THIRD_BUTTON) {

    } else if (clickedButtons&FOURTH_BUTTON) {

    } else if (clickedButtons&FIFTH_BUTTON) {

    }
}

void mousepos_callback(GLFWwindow* win, double xpos, double ypos) {
    if (clickedButtons&FIRST_BUTTON) {
        minx -= (xpos-prevx)*dx/resx;
        miny += (ypos-prevy)*dy/resy;

        prevx = xpos;
        prevy = ypos;
    } else if (clickedButtons&SECOND_BUTTON) {

    } else if (clickedButtons&THIRD_BUTTON) {

    } else if (clickedButtons&FOURTH_BUTTON) {

    } else if (clickedButtons&FIFTH_BUTTON) {

    }
}

void mousewheel_callback(GLFWwindow* win, double xoffset, double yoffset) {
    double zoomFactor = pow(0.95,yoffset);

    minx = minx + dx*(1.0 - zoomFactor)/2.0;
    miny = miny + dy*(1.0 - zoomFactor)/2.0;
    dx *= zoomFactor;
    dy *= zoomFactor;
}
