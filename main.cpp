// C++
#include <iostream>

// OpenGL
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

// OSPRay
#include "ospray/ospray_cpp.h"
#include "rkcommon/math/vec.h"
#include "rkcommon/math/box.h"

// Imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Other headers
#include "callbacks.h"
#include "ArcballCamera.h"
#include "transfer_function_widget.h"
#include "shader.h"
#include "widget.h"
#include "parseArgs.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#include "dataLoader.h"
#include "ospray_volume.h"

//@ Texture Rendering
const std::string fullscreen_quad_vs = R"(
#version 420 core
const vec4 pos[4] = vec4[4](
	vec4(-1, 1, 0.5, 1),
	vec4(-1, -1, 0.5, 1),
	vec4(1, 1, 0.5, 1),
	vec4(1, -1, 0.5, 1)
);
void main(void){
	gl_Position = pos[gl_VertexID];
}
)";

const std::string display_texture_fs = R"(
#version 420 core
layout(binding=0) uniform sampler2D img;
out vec4 color;
void main(void){ 
	ivec2 uv = ivec2(gl_FragCoord.xy);
	color = texelFetch(img, uv, 0);
})";

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// Global variables
// rkcommon::math::vec3i dims(116, 29, 26);
rkcommon::math::vec3i dims(1024, 1024, 5);
// rkcommon::math::vec3i dims(512, 512, 512);

int main(int argc, const char** argv)
{
    //! Parse Argument
    Args arguments;
    parseArgs(argc, argv, arguments);
    std::cout << "Loading file " << arguments.filename << std::endl;

    //! Load data
    Volume volume(dims);
    bool load = loadRaw(arguments.filename, volume);
    std::cout << "Dimensions = (" << volume.dims.x << ", " << volume.dims.y << ", " << volume.dims.z << ")\n";
    std::cout << "Range = (" << volume.range.x << ", " << volume.range.y << ")\n";
    std::cout << "Data size = " << volume.voxels.size() << std::endl;

    int imgSize_x = 1024; // width
    int imgSize_y = 512; // height
    rkcommon::math::vec2i imgSize = rkcommon::math::vec2i(imgSize_x, imgSize_y);
    
    //!! Initialize OSPRay
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
        return init_error;

    //!! Create GLFW Window
    GLFWwindow* window;
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()){
        std::cout << "Cannot Initialize GLFW!! " << std::endl;
        exit(EXIT_FAILURE);
    }else{
        std::cout << "Initialize GLFW!! " << std::endl;
    }
    // const char* glsl_version = "#version 130";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    window = glfwCreateWindow(imgSize_x, imgSize_y, "OSPRay Viewer", NULL, NULL);

    if (!window)
    {
        std::cout << "Why not opening a window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }else{
        std::cout << "Aha! Window opens successfully!!" << std::endl;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gl3wInit()) {
            fprintf(stderr, "failed to initialize OpenGL\n");
            return -1;
    }
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    rkcommon::math::vec2f range;
    rkcommon::math::box3f worldBound;
    range = volume.range;
    worldBound = rkcommon::math::box3f(rkcommon::math::vec3f(0, 0, 0), rkcommon::math::vec3f(volume.dims.x, volume.dims.y, volume.dims.z));

    //!! Camera
    ArcballCamera arcballCamera(worldBound, imgSize);

    // create ospvolume

    ospray::cpp::Volume ospVolume;
    ospVolume = make_ospray_volume(volume, worldBound);
    

    std::shared_ptr<App> app;
    TransferFunctionWidget transferFcnWidget;
    Widget widget(range.x, range.y);
    int total = (range.y - range.x) / 0.1f;
    app = std::make_shared<App>(imgSize, arcballCamera);

    Shader display_render(fullscreen_quad_vs, display_texture_fs);

	GLuint render_texture;
	glGenTextures(1, &render_texture);
	glBindTexture(GL_TEXTURE_2D, render_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, imgSize_x, imgSize_y);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DEPTH_TEST);


    // OSPCamera camera = ospNewCamera("perspective");
    ospray::cpp::Camera camera("perspective");
    rkcommon::math::vec3f pos = arcballCamera.eyePos();
    rkcommon::math::vec3f dir = arcballCamera.lookDir();
    rkcommon::math::vec3f up = arcballCamera.upDir();
    camera.setParam("aspect", imgSize.x / (float)imgSize.y);
    camera.setParam("position", pos);
    camera.setParam("direction", dir);
    camera.setParam("up", up);
    camera.commit();


    // put the volume into a model
    ospray::cpp::VolumetricModel model(ospVolume);
    ospray::cpp::TransferFunction transferFcn("piecewise_linear");

    auto colormap = transferFcnWidget.get_colormap();
    // set transfer function
    // std::vector<rkcommon::math::vec3f> colors = {rkcommon::math::vec3f(1.0f, 0.0f, 0.0f),
    //                                             rkcommon::math::vec3f(1.0f, 0.0f, 0.0f),
    //                                             rkcommon::math::vec3f(1.0f, 0.0f, 0.0f),
    //                                               rkcommon::math::vec3f(0.0f, 1.0f, 0.0f)};
    // std::vector<float> opacities = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    // transferFcn.setParam("color", ospray::cpp::Data(colors));
    // ospray::cpp::Data opacity = ospray::cpp::Data(opacities);
    // transferFcn.setParam("opacity", opacity);
    // // tfcn.setParam("opacity", ospray::cpp::Data(opacities));
    // transferFcn.setParam("valueRange", range);
    // transferFcn.commit();
    update_transfer_fcn(transferFcn, colormap, range);
    // looping_transfer_fcn(transferFcn, range, 0);
    model.setParam("transferFunction", transferFcn);
    // model.setParam("samplingRate", 30.f);
    model.commit();

    // put the model into a group (collection of models)
    ospray::cpp::Group group;

    ospray::cpp::Geometry isoGeom("isosurfaces");
    float isovalue{0.1f};
    isoGeom.setParam("isovalue", ospray::cpp::Data(isovalue));
    isoGeom.setParam("volume", model);
    isoGeom.commit();

    ospray::cpp::Material mat("scivis", "OBJMaterial");
    mat.setParam("Ks", rkcommon::math::vec3f(0.2f));
    mat.commit();

    ospray::cpp::GeometricModel isoModel(isoGeom);
    isoModel.setParam("material", ospray::cpp::Data(mat));
    isoModel.commit();

    // group.setParam("geometry", ospray::cpp::Data(isoModel));

    group.setParam("volume", ospray::cpp::Data(model));
    group.commit();

    ospray::cpp::Instance instance(group);
    instance.commit();

    // put the instance in the world
    ospray::cpp::World world;
    world.setParam("instance", ospray::cpp::Data(instance));

    ospray::cpp::Light light("ambient");
    light.setParam("intensity", 0.1f);
    light.commit();

    world.setParam("light", ospray::cpp::Data(light));
    world.commit();

    // create renderer
    ospray::cpp::Renderer renderer("scivis");

    // complete setup of renderer
    renderer.setParam("aoSamples", 0);
    renderer.setParam("spp", 10);
    renderer.setParam("bgColor", 0.0f);  // white, transparent
    renderer.commit();

    // create and setup framebuffer
    ospray::cpp::FrameBuffer framebuffer(imgSize, OSP_FB_SRGBA, OSP_FB_COLOR| OSP_FB_ACCUM);

    framebuffer.clear();

    glfwSetWindowUserPointer(window, app.get());
    glfwSetCursorPosCallback(window, cursorPosCallback);

    while (!glfwWindowShouldClose(window))
    {
        app -> isTransferFcnChanged = transferFcnWidget.changed();
        app -> isTimeStepChanged = widget.changed();

        if (app ->isCameraChanged) {
            camera.setParam("position", app->camera.eyePos());
            camera.setParam("direction", app->camera.lookDir());
            camera.setParam("up", app->camera.upDir());
            camera.commit();
            framebuffer.clear();
            app ->isCameraChanged = false;
        }

        if (app -> isTransferFcnChanged) {
            // std::cout << "transfer function changed!" << std::endl;
			auto colormap = transferFcnWidget.get_colormap();
			update_transfer_fcn(transferFcn, colormap, range);
            model.commit();
            framebuffer.clear();
            app ->isTransferFcnChanged = false;
		}

        if(app -> isTimeStepChanged){
            //load other time step
            // std::cout << " time step changed! Need to load other time step" << std::endl;
            // std::string filename = prefix + std::to_string(controlWidget.getTimeStep
            float timestep = widget.getTimeStep();
            int radio = (float)(timestep - range.x) / (range.y - range.x) * total;
            // std::cout << "time step " << timestep << "radio " << radio << std::endl;
            looping_transfer_fcn(transferFcn, range, radio);
            framebuffer.clear();
            app ->isTimeStepChanged = false;
        }
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		if (ImGui::Begin("Transfer Function")) {
			transferFcnWidget.draw_ui();
		}
        
        // if(ImGui::Begin("Control Panel")){
        //     widget.draw();
        // }

		ImGui::End();

        // std::cout << "start rendering " << std::endl;
        auto t1 = std::chrono::high_resolution_clock::now();

        framebuffer.renderFrame(renderer, camera, world);

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        std::cout << "Frame rate: " << 1.f / time_span.count() << "\n";
        uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);

        // std::string filename = "test.jpg";
        // stbi_write_jpg(filename.c_str(), 1024, 1024, 4, fb, 100);


        ImGui::Render();
        
        glViewport(0, 0, imgSize_x, imgSize_y);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imgSize_x, imgSize_y, GL_RGBA, GL_UNSIGNED_BYTE, fb);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(display_render.program);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        framebuffer.unmap(fb);

        // frameId++;
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


// export LD_LIBRARY_PATH=/home/mengjiao/external/rkcommon/install/lib:/home/mengjiao/external/VTK-8.2.0/install/lib:/home/mengjiao/external/ospray/install/lib:/home/mengjiao/external/embree-3.3.0.x86_64.linux/lib:/home/mengjiao/external/openvkl/install/lib
