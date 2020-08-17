#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "ospray/ospray_cpp.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "callbacks.h"
#include "ArcballCamera.h"
#include "shader.h"

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
uniform sampler2D img;
out vec4 color;
void main(void){ 
	ivec2 uv = ivec2(gl_FragCoord.xy);
	color = texelFetch(img, uv, 0);
})";

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, const char** argv)
{
	vec2i imgSize;
    imgSize.x = 512; // width
    imgSize.y = 512; // height

	vec2i windowSize;
	windowSize.x = imgSize.x * 2;
	windowSize.y = imgSize.y * 2;

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

	printf("initialize OSPRay...");

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

    window = glfwCreateWindow(windowSize.x, windowSize.y, "Viewer", NULL, NULL);
    
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

	// calculate world bound to initialize camera
    box3f worldBound;
	worldBound.lower = vec3f(-1.0f, -1.0f, 0.0f);
	worldBound.upper = vec3f(1.0f, 1.0f, 3.0f);

	//!! Camera
    ArcballCamera arcballCamera(worldBound, imgSize);
    //! App used for control user interface  
	std::shared_ptr<App> app;
    app = std::make_shared<App>(imgSize, arcballCamera);
	
	Shader display_render(fullscreen_quad_vs, display_texture_fs);

	GLuint render_texture;
	glGenTextures(1, &render_texture);
	glBindTexture(GL_TEXTURE_2D, render_texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, windowSize.x, windowSize.y);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DEPTH_TEST);

	{
        std::cout << "debug0" << std::endl;
		// create and setup camera
		ospray::cpp::Camera camera("perspective");
		camera.setParam("aspect", imgSize.x / (float)imgSize.y);
		camera.setParam("position", cam_pos);
		camera.setParam("direction", cam_view);
		camera.setParam("up", cam_up);
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
		renderer.setParam("pixelSamples", 10);
		renderer.setParam("aoSamples", 1);
		renderer.setParam("backgroundColor", 1.0f); // white, transparent
		renderer.commit();

		// create and setup framebuffer
		ospray::cpp::FrameBuffer framebuffer(imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
		framebuffer.clear();
		
		glfwSetWindowUserPointer(window, app.get());
    	glfwSetCursorPosCallback(window, cursorPosCallback);

		framebuffer.renderFrame(renderer, camera, world);
		uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);

		std::string filename = "/home/mengjiao/Desktop/projects/time-varying-data-viewer/image.jpg";
		
		while (!glfwWindowShouldClose(window)){
			if (app ->isCameraChanged) {
				camera.setParam("position", cam_pos);
				camera.setParam("direction", cam_view);
				camera.setParam("up", cam_up);
				camera.commit();
				framebuffer.clear();
				app ->isCameraChanged = false;
        	}
            // 
            // app->camera.lookDir()
            // app->camera.upDir()
			
			// render one frame
    		framebuffer.renderFrame(renderer, camera, world);
			uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        
			glViewport(0, 0, windowSize.x, windowSize.y);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, windowSize.x, windowSize.y, GL_RGB, GL_UNSIGNED_BYTE, sr_fb);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram(display_render.program);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        	framebuffer.unmap(fb);

	        glfwSwapBuffers(window);
    	    glfwPollEvents();
		}
	}
	glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}