#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils/utils.h"
#include "math/vec.h"
#include "math/math.h"

struct GLManager {
    GLFWwindow* window;
    const int WIDTH = 800; const int HEIGHT = 800;
    float lastTime = 0.0f;

    GLuint activeProgram;

    std::vector<GLuint> VBOs;
    std::vector<GLuint> VAOs;

    GLManager(){
        glfwInit();

        // Initiate OpenGL, Glad, GLFW
        window = glfwCreateWindow(WIDTH, HEIGHT, "Gravity simulator", NULL, NULL);
        if (!window)
            throw std::runtime_error("Couldn't create window.");
            
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw std::runtime_error("Failed to initialize GLAD");
        

        gladLoadGL();   
    }

    GLuint setShader(GLenum sType, GLsizei id, std::string source) {
        std::string shaderSourceString = loadShaderFrom(source);
        const char* shaderSource = shaderSourceString.c_str();

        GLuint shader = glCreateShader(sType);
        glShaderSource(shader, id, &shaderSource, NULL);
        glCompileShader(shader);

        int success;
        char err[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, err);
            throw std::runtime_error("Error ocurred while compiling shader...");
            std::cout << err << std::endl;
        }

        if (!activeProgram)
            activeProgram = glCreateProgram();
        glAttachShader(activeProgram, shader);

        return shader;
    }

    unsigned int setVBO(GLuint size, vec<3>* data)
    {
        GLuint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Current attrib pointer, won't change it until is necessary.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        VBOs.insert(VBOs.end(), VBO);
        VAOs.insert(VAOs.end(), VAO);

        return VBO;
    }

    float getDeltaTime()
    {
        float time = glfwGetTime();
        float deltaTime = time - lastTime;
        lastTime = time;
        return deltaTime;
    }
    
};
GLManager engine;

struct Body {
    glm::vec3 pos = {0.0f, 0.0f, 0.0f};
    size_t modelVertices;
    float radius;
    float mass;
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
    glm::vec4 color = {1.0f, 0.5f, 0.2f, 1.0f};

    Body(size_t verticesNum, float rad, float m, glm::vec3 p, glm::vec3 initVelocity, glm::vec4 col)
        : modelVertices(verticesNum), radius(rad), mass(m), pos(p), velocity(initVelocity), color(col) {}

    void calcInitVelocity(const Body& center)
    {
        const float UNIVERSAL_GRAVITY = 0.1f; 

        glm::vec3 delta = center.pos - pos;
        float r = glm::length(delta);

        if (r < 1e-4f) return;

        glm::vec3 dir = glm::cross(delta, {0.0f, 0.0f, 1.0f});
        if (glm::length(dir) < 1e-4f)
            dir = glm::cross(delta, glm::vec3(0.0f, 1.0f, 0.0f));
        
        dir = glm::normalize(dir);
        float magnitude = std::sqrt(UNIVERSAL_GRAVITY * center.mass / r);

        velocity = magnitude * dir;
    }

    void draw(GLuint shaderProgram, GLint modelLoc, GLint colorLoc) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::scale(model, glm::vec3(radius));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(colorLoc, color.x, color.y, color.z, color.w);

        glDrawArrays(GL_TRIANGLES, 0, modelVertices);
    }
};

class Camera {
private:
    glm::mat4 projection;
    glm::mat4 view;

    GLuint viewLoc;
    GLuint projectionLoc;

    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
        cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));
    }

public:
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;

    double mouseX = 0.0;
    double mouseY = 0.0;
    bool firstMouse = true;

    float yaw = -90.0f;
    float pitch = 0.0f;
    const float SENSIBILITY = 0.1f;

    glm::vec3 pos = {0.0f, 0.0f, 5.0f};

    Camera(GLuint shaderProgram, GLFWwindow* window, float windowWidth, float windowHeight) {
        updateCameraVectors();

        projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.1f, 1000.0f);
        view = glm::lookAt(pos, pos + cameraFront, cameraUp);

        projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        viewLoc       = glGetUniformLocation(shaderProgram, "view");

        if (projectionLoc == -1 || viewLoc == -1)
            throw std::runtime_error("Error while getting uniform value location.");

        glfwSetWindowUserPointer(window, this);
    }

    void use() {
        view = glm::lookAt(pos, pos + cameraFront, cameraUp);
        
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    void processMouseInput(double posx, double posy) {
        if (firstMouse) {
            mouseX = posx;
            mouseY = posy;
            firstMouse = false;
        }

        float offsetX = static_cast<float>(posx - mouseX);
        float offsetY = static_cast<float>(mouseY - posy); 

        mouseX = posx;
        mouseY = posy;

        offsetX *= SENSIBILITY;
        offsetY *= SENSIBILITY;

        yaw   += offsetX;
        pitch += offsetY;

        if (pitch >  89.0f) pitch =  89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateCameraVectors();
    }
};

void mouseCallback(GLFWwindow* window, double posx, double posy) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera) {
        camera->processMouseInput(posx, posy);
    }
}

void processInput(GLFWwindow* window, Camera& camera, float& deltaTime)
{
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    const float cameraSpeed = 24.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        velocity = camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        velocity = -camera.cameraFront;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        velocity = glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        velocity = -glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp));
    
    camera.pos += velocity * cameraSpeed * deltaTime;
}

void calcAcceleration(std::vector<Body>& bodies)
{
    const float UNIVERSAL_GRAVITY = 0.1f;// Modified for this simulation, original: 6.67f * pow(10.0f, -11.0f)
    const float epsilon = 0.1f;
    
    for (size_t i = 0; i<bodies.size(); i++)
    {
        for (size_t j = i + 1; j<bodies.size(); j++)
        {
            glm::vec3 delta = bodies[j].pos - bodies[i].pos; 
            float distSqr = glm::dot(delta, delta) + (epsilon * epsilon);

            if (distSqr < 1e-7f) continue; 

            float dist = std::sqrt(distSqr); // Vectorial distance
            float distCubed = distSqr * dist;

            float commonFactor = UNIVERSAL_GRAVITY / distCubed; 

            glm::vec3 accel_i = commonFactor * bodies[j].mass * delta;
            glm::vec3 accel_j = commonFactor * bodies[i].mass * (-delta);

            // Acumular accelerations in bodies
            bodies[i].acceleration += accel_i;
            bodies[j].acceleration += accel_j;

            //std::cout << accel_i.x << ", " << accel_i.y << ", " << accel_i.z << std::endl;
        }
    }
}

void applyPhysics(std::vector<Body>& bodies, float deltaTime)
{
    for (size_t i = 0; i<bodies.size(); i++)
        bodies[i].acceleration = {0.0f, 0.0f, 0.0f};
    calcAcceleration(bodies);

    for (size_t i = 0; i<bodies.size(); i++)
    {
        //std::cout << bodies[i].velocity.x << std::endl;
        bodies[i].velocity += bodies[i].acceleration * deltaTime;
        bodies[i].pos += bodies[i].velocity * deltaTime;
    }
}

int main()
{
    unsigned int vertexShader = engine.setShader(
        GL_VERTEX_SHADER,
        1,
        "src/shaders/vertexShader.vert"
    );
    unsigned int fragmentShader = engine.setShader(
        GL_FRAGMENT_SHADER,
        1,
        "src/shaders/fragmentShader.frag"
    );
    glLinkProgram(engine.activeProgram);

    std::vector<vec<3>> vertices = drawSphere(40, 40);
    
    Camera camera(engine.activeProgram, engine.window, engine.WIDTH, engine.HEIGHT);
    camera.use();
    glfwSetInputMode(engine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(engine.window, mouseCallback);


    std::vector<Body> bodies;
    bodies.reserve(10);
    Body sun(vertices.size(), 6.0f, 10000.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f});
    Body earth(vertices.size(), 2.0f, 3.0f, {36.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.2f, 1.0f});
    Body moon(vertices.size(), 1.0f, 0.037f, {39.628f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.2f, 1.0f});
    Body mercury(vertices.size(), 1.0f, 0.166f, {19.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.2f, 1.0f});
    Body venus(vertices.size(), 2.0f, 2.45f, {27.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.5f, 0.2f, 1.0f});

    moon.calcInitVelocity(earth);
    glm::vec3 moonVel = moon.velocity;

    earth.calcInitVelocity(sun);
    mercury.calcInitVelocity(sun);
    venus.calcInitVelocity(sun);

    bodies.push_back(sun);
    bodies.push_back(earth);
    bodies.push_back(moon);
    bodies.push_back(venus);
    bodies.push_back(mercury);

    //moon.calcInitVelocity(earth);
    //mercury.calcInitVelocity(sun);
    //venus.calcInitVelocity(sun);
    //std::cout << earth.velocity.x << ", " << earth.velocity.y << ", " << earth.velocity.z << std::endl;
    //Body moon(vertices.size(), 1.0f, 50.0f, {20.0f ,0.1f, 1.5f}, {0.0f, 0.0f, 0.0f}, bodies); // Moon is used as a scale quantifier

    engine.setVBO(vertices.size() * sizeof(vec<3>), vertices.data());
    GLuint modelLoc = glGetUniformLocation(engine.activeProgram, "modelMat");
    GLuint colorLoc = glGetUniformLocation(engine.activeProgram, "objectColor");
    
    while (!glfwWindowShouldClose(engine.window))
    {
        float dt = engine.getDeltaTime();
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(engine.activeProgram);

        camera.use();

        processInput(engine.window, camera, dt);

        applyPhysics(bodies, dt);
        bodies[2].velocity = {0.0f, 0.0f, 0.0f};
        bodies[2].velocity = bodies[1].velocity + moonVel;

        std::cout << bodies[1].pos.x << ", " << bodies[1].pos.y << ", " << bodies[1].pos.z << std::endl;

        for (size_t i = 0; i<bodies.size(); i++)
            bodies[i].draw(engine.activeProgram, modelLoc, colorLoc);

        glfwSwapBuffers(engine.window);
    }

    return 0;
}