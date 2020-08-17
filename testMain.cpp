// Copyright 2009-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/* This is a small example tutorial how to use OSPRay in an application.
 *
 * On Linux build it in the build_directory with
 *   g++ ../apps/ospTutorial/ospTutorial.cpp -I ../ospray/include \
 *       -I ../../rkcommon -L . -lospray -Wl,-rpath,. -o ospTutorial
 * On Windows build it in the build_directory\$Configuration with
 *   cl ..\..\apps\ospTutorial\ospTutorial.cpp /EHsc -I ..\..\ospray\include ^
 *      -I ..\.. -I ..\..\..\rkcommon ospray.lib
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#define NOMINMAX
#include <malloc.h>
#else
#include <alloca.h>
#endif

#include <vector>

// OpenGL
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "ospray/ospray_cpp.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "callbacks.h"
#include "shader.h"
#include "ArcballCamera.h"

using namespace rkcommon::math;

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

// helper function to write the rendered image as PPM file
void writePPM(const char *fileName, const vec2i &size, const uint32_t *pixel)
{
  FILE *file = fopen(fileName, "wb");
  if (file == nullptr) {
    fprintf(stderr, "fopen('%s', 'wb') failed: %d", fileName, errno);
    return;
  }
  fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
  unsigned char *out = (unsigned char *)alloca(3 * size.x);
  for (int y = 0; y < size.y; y++) {
    const unsigned char *in =
        (const unsigned char *)&pixel[(size.y - 1 - y) * size.x];
    for (int x = 0; x < size.x; x++) {
      out[3 * x + 0] = in[4 * x + 0];
      out[3 * x + 1] = in[4 * x + 1];
      out[3 * x + 2] = in[4 * x + 2];
    }
    fwrite(out, 3 * size.x, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}

int main(int argc, const char **argv)
{
    // image size
    vec2i imgSize;
    imgSize.x = 1024; // width
    imgSize.y = 768; // height

    // camera
    vec3f cam_pos{0.f, 0.f, 0.f};
    vec3f cam_up{0.f, 1.f, 0.f};
    vec3f cam_view{0.1f, 0.f, 1.f};

    // triangle mesh data
    std::vector<vec3f> vertex = {vec3f(-1.0f, -1.0f, 3.0f),
        vec3f(-1.0f, 1.0f, 3.0f),
        vec3f(1.0f, -1.0f, 3.0f),
        vec3f(0.1f, 0.1f, 0.3f)};

    std::vector<vec4f> color = {vec4f(0.9f, 0.5f, 0.5f, 1.0f),
        vec4f(0.8f, 0.8f, 0.8f, 1.0f),
        vec4f(0.8f, 0.8f, 0.8f, 1.0f),
        vec4f(0.5f, 0.9f, 0.5f, 1.0f)};

    std::vector<vec3ui> index = {vec3ui(0, 1, 2), vec3ui(1, 2, 3)};

    box3f worldBound;
    worldBound.lower = vec3f{-1.f, -1.f, 0.f};
    worldBound.upper = vec3f{1.f, 1.f, 3.0f};

    ArcballCamera arcballCamera(worldBound, imgSize);
    
    std::shared_ptr<App> app;
    app = std::make_shared<App>(imgSize, arcballCamera);

    // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
    // e.g. "--osp:debug"
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	window = glfwCreateWindow(imgSize.x, imgSize.y, "OSPRay Viewer", NULL, NULL);

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

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    Shader display_render(fullscreen_quad_vs, display_texture_fs);

	GLuint render_texture;
	glGenTextures(1, &render_texture);
	glBindTexture(GL_TEXTURE_2D, render_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, imgSize.x, imgSize.y);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DEPTH_TEST);

    // use scoped lifetimes of wrappers to release everything before ospShutdown()
    {
        // create and setup camera
        ospray::cpp::Camera camera("perspective");
        camera.setParam("aspect", imgSize.x / (float)imgSize.y);
        camera.setParam("position", arcballCamera.eyePos());
        camera.setParam("direction", arcballCamera.lookDir());
        camera.setParam("up", arcballCamera.upDir());
        camera.commit(); // commit each object to indicate modifications are done

        // create and setup model and mesh
        ospray::cpp::Geometry mesh("mesh");
        mesh.setParam("vertex.position", ospray::cpp::CopiedData(vertex));
        mesh.setParam("vertex.color", ospray::cpp::CopiedData(color));
        mesh.setParam("index", ospray::cpp::CopiedData(index));
        mesh.commit();

        // put the mesh into a model
        ospray::cpp::GeometricModel model(mesh);
        model.commit();

        // put the model into a group (collection of models)
        ospray::cpp::Group group;
        group.setParam("geometry", ospray::cpp::CopiedData(model));
        group.commit();

        // put the group into an instance (give the group a world transform)
        ospray::cpp::Instance instance(group);
        instance.commit();

        // put the instance in the world
        ospray::cpp::World world;
        world.setParam("instance", ospray::cpp::CopiedData(instance));

        // create and setup light for Ambient Occlusion
        ospray::cpp::Light light("ambient");
        light.commit();

        world.setParam("light", ospray::cpp::CopiedData(light));
        world.commit();

        // create renderer, choose Scientific Visualization renderer
        ospray::cpp::Renderer renderer("scivis");

        // complete setup of renderer
        renderer.setParam("aoSamples", 1);
        renderer.setParam("backgroundColor", 1.0f); // white, transparent
        renderer.commit();

        // create and setup framebuffer
        ospray::cpp::FrameBuffer framebuffer(imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        glfwSetWindowUserPointer(window, app.get());
        glfwSetCursorPosCallback(window, cursorPosCallback);

        while (!glfwWindowShouldClose(window))
        {

            if (app ->isCameraChanged) {
                camera.setParam("position", app->camera.eyePos());
                camera.setParam("direction", app->camera.lookDir());
                camera.setParam("up", app->camera.upDir());
                camera.commit();
                framebuffer.clear();
                app ->isCameraChanged = false;
            }
            // render one frame
            framebuffer.renderFrame(renderer, camera, world);
            framebuffer.clear();

            // access framebuffer and write its content as PPM file
            uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
            glViewport(0, 0, imgSize.x, imgSize.y);
		    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, imgSize.x, imgSize.y, GL_RGBA, GL_UNSIGNED_BYTE, fb);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(display_render.program);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            framebuffer.unmap(fb);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    ospShutdown();

    return 0;
}
