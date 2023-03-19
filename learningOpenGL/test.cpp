#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "shader.h"
#include "camera.h"
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "model.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(const char* path,bool gamma);
void RenderQuad_Normal();
void RenderQuad();
void RenderCube();

// 窗口大小
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

float mixValue = 0.2f;

//摄像机
Camera camera(glm::vec3(-10.0f, 20.0f, 30.0f));
float lastY = SCR_WIDTH / 2.0F;
float lastX = SCR_HEIGHT / 2.0F;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

glm::vec3 DirlightPos(-3.0f, 18.0f, 18.0f);
glm::vec3 lightPos(-10.0f, 20.0f, 20.0f);
//glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

float exposure = 1.0f;
bool bloom = true;
bool bloomKeyPressed = false;



int main()
{

    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------

    // glfw: initialize and configure 初始化和设定
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);


    // glfw window creation 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); //上下文

    //告诉GLFW我们希望每当窗口调整大小的时候调用这个函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //鼠标活动
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);




    // glad: load all OpenGL function pointers 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    //--------------------------------------------------------------------------------------
    //---------------------------imgui设置--------------------------------------------------

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls 开启键盘控制

    // Setup Dear ImGui style 设置风格
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");



    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------
    //开启MSAA
    //glEnable(GL_MULTISAMPLE);

    //开启深度测试
    glEnable(GL_DEPTH_TEST);

    //构建并编译shader程序
    Shader lightshader("light.vs", "light.fs");
    Shader sampleShader("sample_shader.vs", "sample_shader.fs");
    Shader ourShader("nanosuit_shader.vs", "nanosuit_shader.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    Shader boxshader("boxshader.vs", "boxshader.fs");
    Shader normalshader("nanosuit_normal_shader.vs", "nanosuit_normal_shader.fs", "nanosuit_normal_shader.gs");
    Shader explodeshader("explode_shader.vs", "explode_shader.fs", "explode_shader.gs");
    Shader normalmapshader("normalmap_shader.vs", "normalmap_shader.fs");
    Shader baseshader("base_shader.vs", "base_shader.fs");
    Shader hdrshader("hdr.vs", "hdr.fs");
    Shader blurShader("blur.vs", "blur.fs");
    Shader shadowmapdepth("shadow_mapping_depth.vs", "shadow_mapping_depth.fs");
    Shader shadowmap("shadow_mapping.vs", "shadow_mapping.fs");
    Shader shadowmapdebug("shadowmap_debug.vs", "shadowmap_debug.fs");

    //正方体顶点数据
    float cubeVertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
    //天空盒顶点数据
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    //地板顶点数据
    float planeVertices[] = {
        // positions            // normals         // texcoords
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };
    unsigned int planeVAO;
    unsigned int planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);



    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    //阴影相关，深度缓冲
    unsigned int depthmapFBO;
    glGenFramebuffers(1, &depthmapFBO);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthmap;
    glGenTextures(1, &depthmap);
    glBindTexture(GL_TEXTURE_2D, depthmap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthmapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthmap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);




    // HDR帧缓冲
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    //创建浮点帧缓冲,这里用两个，一个记录正常渲染图像，一个记录需要做泛光的图像
    unsigned int colorBuffer[2];
    glGenTextures(2, colorBuffer);
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //贴上纹理
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffer[i], 0);
    }
    //深度帧缓冲并贴上
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    //告诉GL哪一个颜色贴图我们用来渲染
    unsigned int attachment[2] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachment);
    //最后检测是否完成渲染
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //做高斯模糊
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }





    vector<std::string> faces{
    ("assets/skybox/right.jpg"),
    ("assets/skybox/left.jpg"),
    ("assets/skybox/top.jpg"),
    ("assets/skybox/bottom.jpg"),
    ("assets/skybox/front.jpg"),
    ("assets/skybox/back.jpg")
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    unsigned int box_diffuse = loadTexture("assets/textures/container2.png",false);
    unsigned int box_specular = loadTexture("assets/textures/container2_specular.png",false);
    //unsigned int diffuseMap = loadTexture("assets/textures/brickwall.jpg");
    //unsigned int normalMap = loadTexture("assets/textures/brickwall_normal.jpg");
    unsigned int floorwood = loadTexture("assets/textures/wood.png", false);

    Model ourModel("assets/nanosuit_reflection/nanosuit.obj");
    //Model backpackModel("assets/backpack/backpack.obj");
    Model cyborgModel("assets/cyborg/cyborg.obj");
    //Model cat("assets/cat/cat.obj");
    //Model witch("assets/witch/witch.obj");
    //Model crab("assets/crab/crab.obj");
    //Model diablo3_pose("assets/diablo3_pose/diablo3_pose.obj");




    
    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------

    // render loop

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        //render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//设置清空屏幕所用的颜色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glm::vec3 lightPos2(-10.0f, 20.0f, 20.0f);
        ////做深度贴图，计算相机空间转换矩阵
        //glm::mat4 lightProjection, lightView;
        //glm::mat4 lightSpaceMatrix;
        //float near_plane = 1.0f, far_plane = 30.0f;
        ////lightProjection = glm::perspective(glm::radians(camera.Zoom), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        //lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        //lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        //lightSpaceMatrix = lightProjection * lightView;
        //shadowmapdepth.use();
        //shadowmapdepth.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        //glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        //glBindFramebuffer(GL_FRAMEBUFFER, depthmapFBO);
        //glClear(GL_DEPTH_BUFFER_BIT);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, floorwood);
        //
        ////提前渲染一边场景
        //{


        //    // floor
        //    glm::mat4 model = glm::mat4(1.0f);
        //    //model = glm::scale(model, glm::vec3(2.0f, 0.0f, 2.0f));
        //    shadowmapdepth.setMat4("model", model);
        //    glBindVertexArray(planeVAO);
        //    glDrawArrays(GL_TRIANGLES, 0, 6);
        //    // cubes
        //    glActiveTexture(GL_TEXTURE0);
        //    glBindTexture(GL_TEXTURE_2D, box_diffuse);
        //    model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
        //    model = glm::scale(model, glm::vec3(0.5f));
        //    shadowmapdepth.setMat4("model", model);
        //    RenderCube();
        //    model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
        //    model = glm::scale(model, glm::vec3(0.5f));
        //    shadowmapdepth.setMat4("model", model);
        //    RenderCube();
        //    model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
        //    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        //    model = glm::scale(model, glm::vec3(0.25));
        //    shadowmapdepth.setMat4("model", model);
        //    RenderCube();


        //    ////ourmodel
        //    //model = glm::mat4(1.0f);
        //    //shadowmapdepth.setMat4("model", model);
        //    //ourModel.Draw(shadowmapdepth);

        //    ////render the cybrog
        //    //model = glm::mat4(1.0f);
        //    //model = glm::translate(model, glm::vec3(-15.0f, 0.0f, 10.0f));
        //    //model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        //    //shadowmapdepth.setMat4("model", model);
        //    //cyborgModel.Draw(shadowmapdepth);

        //}
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //// reset viewport
        //glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //view/projection transformations
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();


        //render the lightcube 移动光源
        glm::vec3 lightcolor = glm::vec3(5.0f, 5.0f, 5.0f);
        lightshader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        //model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        lightshader.setMat4("model", model);
        lightshader.setMat4("projection", projection);
        lightshader.setMat4("view", view);
        lightshader.setVec3("lightcolor", lightcolor);
        glBindVertexArray(lightCubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);


        // render the loaded model
        ourShader.use();
        //shadowmap.use();
        {
            ourShader.setFloat("material.shininess", 64.0f);
            ourShader.setVec3("viewPos", camera.Position);
            //ourShader.setFloat("material.shininess", 64.0f);
            ourShader.setVec3("dirLight.direction", DirlightPos);
            ourShader.setVec3("dirLight.ambient", 0.4f, 0.4f, 0.4f);
            ourShader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
            ourShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

            ourShader.setVec3("pointLight.position", lightPos);
            ourShader.setVec3("pointLight.lightcolor", lightcolor);
            ourShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
            ourShader.setVec3("pointLight.diffuse", 0.8f, 0.8f, 0.8f);
            ourShader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
            ourShader.setFloat("pointLight.constant", 1.0f);
            ourShader.setFloat("pointLight.linear", 0.045f);
            ourShader.setFloat("pointLight.quadratic", 0.0075f);
        }

        model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        //shadowmap.setMat4("model", model);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        //ourShader.setVec3("viewPos", camera.Position);
        //ourShader.setVec3("lightPos", lightPos);
        //ourShader.setVec3("cameraPos", camera.Position);
        ourShader.setInt("skybox", 5);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);






        //爆炸模型和法线模型
        //注释方法， 选定块，先ctrl+k,再ctrl+c注释，同理取消先k再u
        {
            ////render the normal model
            //model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 0.0f));
            //ourShader.setMat4("model", model);
            //ourModel.Draw(ourShader);

            //normalshader.use();
            //normalshader.setMat4("projection", projection);
            //normalshader.setMat4("view", view);
            //normalshader.setMat4("model", model);
            //ourModel.Draw(normalshader);


            ////-- render the explode model
            //model = glm::translate(model, glm::vec3(20.0f, 0.0f, 0.0f));
            //explodeshader.use();
            //explodeshader.setMat4("model", model);
            //explodeshader.setMat4("projection", projection);
            //explodeshader.setMat4("view", view);
            //explodeshader.setFloat("time", static_cast<float>(glfwGetTime()));
            //ourModel.Draw(explodeshader);
        }

        //sampleshader光
        {
            sampleShader.use();
            sampleShader.setVec3("viewPos", camera.Position);
            sampleShader.setFloat("material.shininess", 64.0f);

            sampleShader.setVec3("dirLight.direction", DirlightPos);
            sampleShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
            sampleShader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
            sampleShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

            sampleShader.setVec3("pointLight.position", lightPos);
            sampleShader.setVec3("pointLight.lightcolor", lightcolor);
            sampleShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
            sampleShader.setVec3("pointLight.diffuse", 0.8f, 0.8f, 0.8f);
            sampleShader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
            sampleShader.setFloat("pointLight.constant", 1.0f);
            sampleShader.setFloat("pointLight.linear", 0.045f);
            sampleShader.setFloat("pointLight.quadratic", 0.0075f);
        }

        //一些小玩意
        {
            sampleShader.setMat4("projection", projection);
            sampleShader.setMat4("view", view);

            ////render the backpack
            //model = glm::mat4(1.0f);
            //model = glm::translate(model, glm::vec3(10.0f, 20.0f, 0.0f));
            //sampleShader.setMat4("model", model);

            //backpackModel.Draw(sampleShader);

            ////render the cat
            //model = glm::mat4(1.0f);
            //model = glm::translate(model, glm::vec3(-10.0f, 30.0f, 0.0f));
            //model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
            //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            //sampleShader.setMat4("model", model);
            //cat.Draw(sampleShader);

            ////render the container
            //model = glm::mat4(1.0f);
            //model = glm::translate(model, glm::vec3(10.0f, 30.0f, 0.0f));
            //model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
            //sampleShader.setMat4("model", model);
            //sampleShader.setInt("material.diffuse", 0);
            //sampleShader.setInt("material.specular", 1);
            //glBindVertexArray(cubeVAO);
            //glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_2D, box_diffuse);
            //glActiveTexture(GL_TEXTURE1);
            //glBindTexture(GL_TEXTURE_2D, box_specular);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
            //glBindVertexArray(0);

            //render the cybrog
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-15.0f, 0.0f, 10.0f));
            model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
            ////model = glm::rotate(model, glm::radians((float)glfwgettime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
            sampleShader.setMat4("model", model);
            cyborgModel.Draw(sampleShader);

            //render the floor
            model = glm::mat4(1.0f);
            //model = glm::scale(model, glm::vec3(2.0f, 0.0f, 2.0f));
            sampleShader.setMat4("model", model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, floorwood);
            glBindVertexArray(planeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

        }

        //反射天空盒的box
        {
            //boxshader.use();
            //model = glm::mat4(1.0f);
            //model = glm::translate(model, glm::vec3(0.0f, 20.0f, 0.0f));
            //model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
            //boxshader.setMat4("model", model);
            //boxshader.setMat4("view", view);
            //boxshader.setMat4("projection", projection);
            //boxshader.setVec3("cameraPos", camera.Position);
            //// cubes
            //glBindVertexArray(cubeVAO);
            //glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
            //glBindVertexArray(0);
        }

        //法线贴图
        {
            //normalmapshader.use();
            //model = glm::mat4(1.0f);
            //normalmapshader.setMat4("view", view);
            //normalmapshader.setMat4("projection", projection);
            //normalmapshader.setVec3("viewPos", camera.Position);
            //glm::vec3 lightPos_normal(0.0f, 0.0f, -50.0f);
            //normalmapshader.setVec3("lightPos", lightPos_normal);

            //model = glm::translate(model, glm::vec3(0.0f, 0.0f, -30.f));
            //model = glm::rotate(model, (GLfloat)glfwGetTime() * 0.2f, glm::vec3(-30.0, 0.0, 0.0));
            //normalmapshader.setMat4("model", model);
            //glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_2D, diffuseMap);
            //glActiveTexture(GL_TEXTURE1);
            //glBindTexture(GL_TEXTURE_2D, normalMap);
            //RenderQuad();

            //baseshader.use();
            //model = glm::mat4(1.0f);
            //baseshader.setMat4("view", view);
            //baseshader.setMat4("projection", projection);
            //baseshader.setVec3("viewPos", camera.Position);
            //glm::vec3 lightPos_normal2(20.0f, 0.0f, -50.0f);
            //baseshader.setVec3("lightPos", lightPos_normal2);
            //model = glm::translate(model, glm::vec3(20.0f, 0.0f, -30.f));
            //baseshader.setMat4("model", model);
            //glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_2D, diffuseMap);
            //RenderQuad();
        }

        //渲染天空盒
        {
            ////render the skybox
            //glDepthFunc(GL_LEQUAL);
            //skyboxShader.use();
            //skyboxShader.setInt("skybox", 0);
            //skyboxShader.setMat4("projection", projection);
            //view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
            //skyboxShader.setMat4("view", view);
            //glBindVertexArray(skyboxVAO);
            //glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
            //glBindVertexArray(0);
            //glDepthFunc(GL_LESS);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //做泛光
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        blurShader.setInt("image", 0);
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            //glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffer[1] : pingpongColorbuffers[!horizontal]);// bind texture of other framebuffer (or scene if first iteration)


            RenderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);



        //后处理，将颜色缓冲渲染到铺屏四边形上
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        hdrshader.use();
        hdrshader.setInt("hdrBuffer", 0);
        hdrshader.setInt("bloomBulr", 1);
        hdrshader.setFloat("exposure", exposure);
        hdrshader.setBool("bloom", bloom);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffer[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        RenderQuad();


        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_DEPTH_TEST);
        //shadowmapdebug.use();
        //shadowmapdebug.setInt("depthMap", 0);
        //shadowmapdebug.setFloat("near_plane", near_plane);
        //shadowmapdebug.setFloat("far_plane", far_plane);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, depthmap);
        //RenderQuad();
        

        
        //-------------------------------------------------------------------------------
        //-------------------------------------------------------------------------------
        //render imgui
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                ImGui::Begin("Data Window");
                ImGui::Text("FPS    : %.1f", ImGui::GetIO().Framerate);
                ImGui::Text("bloom    : %d", bloom);
                ImGui::Text("CAMERA : (%.2f,%.2f,%.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
                ImGui::End();
            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        }





        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

// RenderQuad() Renders a 1x1 quad in NDC
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad_Normal()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0, 1.0, 0.0);
        glm::vec3 pos2(-1.0, -1.0, 0.0);
        glm::vec3 pos3(1.0, -1.0, 0.0);
        glm::vec3 pos4(1.0, 1.0, 0.0);
        // texture coordinates
        glm::vec2 uv1(0.0, 1.0);
        glm::vec2 uv2(0.0, 0.0);
        glm::vec2 uv3(1.0, 0.0);
        glm::vec2 uv4(1.0, 1.0);
        // normal vector
        glm::vec3 nm(0.0, 0.0, -1.0);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // - triangle 1
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // - triangle 2
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        GLfloat quadVertices[] = {
            // Positions            // normal         // TexCoords  // Tangent                          // Bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO1 = 0;
unsigned int quadVBO1;
void RenderQuad()
{
    if (quadVAO1 == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO1);
        glGenBuffers(1, &quadVBO1);
        glBindVertexArray(quadVAO1);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void RenderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}






//天空盒
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}



unsigned int loadTexture(char const* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}







//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

//来检测特定的键是否被按下，并在每一帧做出处理。这里判断是否按下esc退出窗口
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //UP - DOWN 切换混合比例
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            lightPos.x += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            lightPos.y += 0.1f;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            lightPos.x += -0.1f;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            lightPos.y += -0.1f;
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            lightPos.z += -0.1f;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
            lightPos.z += 0.1f;

        //Z是否使用bloom
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !bloomKeyPressed)
        {
            bloom = !bloom;
            bloomKeyPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
        {
            bloomKeyPressed = false;
        }




        //相机移动
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
        //相机重置
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ResetViewpoint();
    
}


//当用户改变窗口的大小的时候，视口也应该被调整
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

    glViewport(0, 0, width, height);
}

//每当鼠标动，调整
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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

//滚轮动，调整
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}