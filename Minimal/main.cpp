/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw {
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp {

protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();
			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr {

	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}

	inline mat4 toGlm(const ovrMatrix4f & om) {
		return glm::transpose(glm::make_mat4(&om.M[0][0]));
	}

	inline mat4 toGlm(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
		return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
	}

	inline vec3 toGlm(const ovrVector3f & ov) {
		return glm::make_vec3(&ov.x);
	}

	inline vec2 toGlm(const ovrVector2f & ov) {
		return glm::make_vec2(&ov.x);
	}

	inline uvec2 toGlm(const ovrSizei & ov) {
		return uvec2(ov.w, ov.h);
	}

	inline quat toGlm(const ovrQuatf & oq) {
		return glm::make_quat(&oq.x);
	}

	inline mat4 toGlm(const ovrPosef & op) {
		mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
		mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
		return translation * orientation;
	}

	inline ovrMatrix4f fromGlm(const mat4 & m) {
		ovrMatrix4f result;
		mat4 transposed(glm::transpose(m));
		memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
		return result;
	}

	inline ovrVector3f fromGlm(const vec3 & v) {
		ovrVector3f result;
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	inline ovrVector2f fromGlm(const vec2 & v) {
		ovrVector2f result;
		result.x = v.x;
		result.y = v.y;
		return result;
	}

	inline ovrSizei fromGlm(const uvec2 & v) {
		ovrSizei result;
		result.w = v.x;
		result.h = v.y;
		return result;
	}

	inline ovrQuatf fromGlm(const quat & q) {
		ovrQuatf result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:

private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

	float defaultHmdToEyeOffset[2]; // 0.0294861

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
			_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;
			defaultHmdToEyeOffset[eye] = _viewScaleDesc.HmdToEyeOffset[eye].x;
			std::cout << _viewScaleDesc.HmdToEyeOffset[eye].x << std::endl;

			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 2;
	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	ovrPosef lastEye[2], renderEye[2];
	void draw() final override {
		ovrPosef eyePoses[2];
		ovrVector3f renderEyeOffset[ovrEye_Count];
		renderEyeOffset[0] = _viewScaleDesc.HmdToEyeOffset[0];
		renderEyeOffset[1] = _viewScaleDesc.HmdToEyeOffset[1];
		//renderEyeOffset[0].x = std::min(std::max(defaultHmdToEyeOffset[0] - getIOD() / 2, -0.3f), 0.0f);
		//renderEyeOffset[1].x = std::min(std::max(defaultHmdToEyeOffset[1] + getIOD() / 2, 0.0f), 0.3f);
		//if (getViewState() == 1) { renderEyeOffset[0].x = renderEyeOffset[1].x = 0.0f; }
		/*
		if (getViewState() == 1) { 
			renderEyeOffset[0].x = (renderEyeOffset[0].x + renderEyeOffset[1].x) / 2.0f; 
			renderEyeOffset[0].y = (renderEyeOffset[0].y + renderEyeOffset[1].y) / 2.0f;
			renderEyeOffset[0].z = (renderEyeOffset[0].z + renderEyeOffset[1].z) / 2.0f;
			renderEyeOffset[1] = renderEyeOffset[0];
		}
		*/
		//std::cout << _viewScaleDesc.HmdToEyeOffset[0].x << " " << _viewScaleDesc.HmdToEyeOffset[1].x << std::endl;
		ovr_GetEyePoses(_session, frame, true, renderEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);

		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ovr::for_each_eye([&](ovrEyeType eye) {
			/*
			renderEye[eye] = lastEye[eye];
			if (getTrackingState() == 0) {
				renderEye[eye] = eyePoses[eye];
			}
			else if (getTrackingState() == 2) {
				renderEye[eye].Position = eyePoses[eye].Position;
			}
			else if (getTrackingState() == 3) {
				renderEye[eye].Orientation = eyePoses[eye].Orientation;
			}
			lastEye[eye] = renderEye[eye];
			*/
			/*
			if (getViewState() == 1) {
				currentEye(ovrEye_Left);
				const auto& vp = _sceneLayer.Viewport[eye];
				glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
				_sceneLayer.RenderPose[eye] = eyePoses[ovrEye_Left];
				renderScene(_eyeProjections[ovrEye_Left], ovr::toGlm(renderEye[ovrEye_Left]));
			}
			else {
				currentEye(eye);
				const auto& vp = _sceneLayer.Viewport[eye];
				glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
				_sceneLayer.RenderPose[eye] = eyePoses[eye];
				if (eye == ovrEye_Left && getViewState() == 3) return;
				if (eye == ovrEye_Right && getViewState() == 2) return;
				renderScene(_eyeProjections[eye], ovr::toGlm(renderEye[eye]));
			}
			*/
			
			currentEye(eye);
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]));
			
		});
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}

	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) = 0;
	virtual void currentEye(ovrEyeType eye) = 0;
	//virtual int getViewState() = 0;
	//virtual int getTrackingState() = 0;
	//virtual float getIOD() = 0;
	//virtual float getCubeSize() = 0;
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use oglplus to render an array of cubes, but your 
// application would perform whatever rendering you want
//


#include <time.h>
#include "Shader.h"
#include "Cube.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

#include "rpc/client.h"
#include "rpc/server.h"

#include <glm/gtc/type_ptr.hpp>

rpc::server srv(8080);

struct SimScene {
	Cube * otherhead;
	Cube * myhand;
	Cube * otherhand;

	Model * table;
	Sphere * sphere;	
	Skybox * skybox;
	GLint cubeShaderProgram, sphereShaderProgram, skyboxShaderProgram, tableShaderProgram;

	//bool buttonAPressed = false, buttonBPressed = false, buttonXPressed = false;
	//int buttonA = 0, buttonB = 0, buttonX = 0;
	//float IOD = 0.0f, cubeSize = 0.03f;

#define CUBE_VERTEX_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/shader.vert"
#define CUBE_FRAGMENT_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/shader.frag"

#define SPHERE_VERTEX_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/sphere.vert"
#define SPHERE_FRAGMENT_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/sphere.frag"

#define SKYBOX_VERTEX_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/skybox.vert"
#define SKYBOX_FRAGMENT_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/skybox.frag"

#define TABLE_VERTEX_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/table.vert"
#define TABLE_FRAGMENT_SHADER_PATH "C:/Users/degu/Desktop/CSE190Project4/Minimal/table.frag"

public:
	static glm::mat4 P; // P for projection
	static glm::mat4 V; // V for view
	int curEyeIdx;

	SimScene() {
		cubeShaderProgram = LoadShaders(CUBE_VERTEX_SHADER_PATH, CUBE_FRAGMENT_SHADER_PATH);
		sphereShaderProgram = LoadShaders(SPHERE_VERTEX_SHADER_PATH, SPHERE_FRAGMENT_SHADER_PATH);
		skyboxShaderProgram = LoadShaders(SKYBOX_VERTEX_SHADER_PATH, SKYBOX_FRAGMENT_SHADER_PATH);
		tableShaderProgram = LoadShaders(TABLE_VERTEX_SHADER_PATH, TABLE_FRAGMENT_SHADER_PATH);

		skybox = new Skybox();
		skybox->toWorld = glm::mat4(1.0f);
		table = new Model("C:/Users/degu/Desktop/CSE190Project4/Minimal/digital_x_free_25_ping_pong/X025_017.obj");
		table->toWorld = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f)), glm::vec3(0.0f, -40.0f, -2.0f));
		sphere = new Sphere();
		sphere->toWorld = glm::translate(glm::scale(mat4(1.0f), vec3(10.0f, 10.0f, 10.0f)), vec3(0.0f, 10.0f, -10.0f));

		myhand = new Cube();
		otherhand = new Cube();
		
		srv.bind("setHandTransf", [this](string str) {
			float tmp[16] = { 0.0f };
			istringstream is(str);
			for (int i = 0; i < 16; i++) is >> tmp[i];
			otherhand->toWorld = glm::make_mat4(tmp);
			std::cout << "set leap hand pos!!!!" << std::endl;
		});
		srv.bind("getHandTransf", [this]() {
			float ret[16] = { 0.0 };
			const float *pSource = (const float *)glm::value_ptr(myhand->toWorld);
			for (int i = 0; i < 16; i++) {
				ret[i] = pSource[i];
			}
			ostringstream os;
			for (int i = 0; i<16; i++)
				os << ret[i] << " ";

			std::cout << "get oculus hand pos!!!!!" << std::endl;
			return os.str();
		});
		srv.async_run(1);
	}

	void update() {
		//cube->toWorld = glm::mat4(1.0f);
	}

	void render(const mat4 & projection, const mat4 & modelview) {

		// Use the shader of programID
		glUseProgram(skyboxShaderProgram);
		//skybox->draw(skyboxShaderProgram, projection, modelview);

		glUseProgram(cubeShaderProgram);
		myhand->draw(cubeShaderProgram, projection, modelview);
		otherhand->draw(cubeShaderProgram, projection, modelview);

		glUseProgram(sphereShaderProgram);
		sphere->draw(cubeShaderProgram, projection, modelview);

		glUseProgram(tableShaderProgram);
		glm::vec3 pointLightPosition;
		pointLightPosition = glm::vec3(-5.0f, 20.0f, -5.0f);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[0].position"), pointLightPosition.x, pointLightPosition.y, pointLightPosition.z);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[0].ambient"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[0].diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(tableShaderProgram, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(tableShaderProgram, "pointLights[0].linear"), 0.009f); // 0.09
		glUniform1f(glGetUniformLocation(tableShaderProgram, "pointLights[0].quadratic"), 0.0032f); // 0.032

		pointLightPosition = glm::vec3(5.0f, 20.0f, -5.0f);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[1].position"), pointLightPosition.x, pointLightPosition.y, pointLightPosition.z);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[1].ambient"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[1].diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(tableShaderProgram, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(tableShaderProgram, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(tableShaderProgram, "pointLights[1].linear"), 0.009f); // 0.09
		glUniform1f(glGetUniformLocation(tableShaderProgram, "pointLights[1].quadratic"), 0.0032f); // 0.032
		table->draw(tableShaderProgram, projection, modelview);
	}

	void currentEye(int eyeIdx) {
		curEyeIdx = eyeIdx;
		skybox->useCubemap(curEyeIdx);
	}

private:

};

class SimApp : public RiftApp {
	std::shared_ptr<SimScene> simScene;

public:
	SimApp() {
	}
protected:

	void initGl() override {
		RiftApp::initGl();
		// Enable depth buffering
		glEnable(GL_DEPTH_TEST);
		// Related to shaders and z value comparisons for the depth buffer
		glDepthFunc(GL_LEQUAL);
		// Set polygon drawing mode to fill front and back of each polygon
		// You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Disable backface culling to render both sides of polygons
		glDisable(GL_CULL_FACE);
		// Set clear color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		ovr_RecenterTrackingOrigin(_session);
		simScene = std::shared_ptr<SimScene>(new SimScene());
	}

	void shutdownGl() override {
	}

	void update() override {
		/*
		ovrInputState inputState;
		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {
			if (inputState.Buttons & ovrButton_A) simScene->buttonAPressed = true;
			else if (simScene->buttonAPressed) {
				simScene->buttonA = (simScene->buttonA + 1) % 4; simScene->buttonAPressed = false;
			}
			if (inputState.Buttons & ovrButton_B) simScene->buttonBPressed = true;
			else if (simScene->buttonBPressed) {
				simScene->buttonB = (simScene->buttonB + 1) % 4; simScene->buttonBPressed = false;
			}
			if (inputState.Buttons & ovrButton_X) simScene->buttonXPressed = true;
			else if (simScene->buttonXPressed) {
				simScene->buttonX = (simScene->buttonX + 1) % 4; simScene->buttonXPressed = false;
			}

			if (inputState.Buttons & ovrButton_RThumb) simScene->IOD = 0;
			else {
				if (inputState.Thumbstick[ovrHand_Right].x > 0.5f) simScene->IOD += 0.001f;
				else if (inputState.Thumbstick[ovrHand_Right].x < -0.5f) simScene->IOD -= 0.001f;
			}
			if (inputState.Buttons & ovrButton_LThumb) simScene->cubeSize = 0.03f;
			else {
				if (inputState.Thumbstick[ovrHand_Left].x > 0.5f) simScene->cubeSize = std::min(simScene->cubeSize + 0.001f, 0.1f);
				else if (inputState.Thumbstick[ovrHand_Left].x < -0.5f) simScene->cubeSize = std::max(simScene->cubeSize - 0.001f, 0.001f);
			}
		}
		*/
		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
		ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
		ovrPosef rightPose = trackState.HandPoses[ovrHand_Right].ThePose;
		simScene->myhand->toWorld = ovr::toGlm(rightPose) * glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
		simScene->update();
	}

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose) override {
		simScene->render(projection, glm::inverse(headPose));
	}

	void currentEye(ovrEyeType eye) {
		if (eye == ovrEye_Left) {
			simScene->currentEye(0);
		}
		else {
			simScene->currentEye(1);
		}
	}
	//int getViewState() { return simScene->buttonA; }
	//int getTrackingState() { return simScene->buttonB; }
	//float getIOD() { return simScene->IOD; }
	//float getCubeSize() { return simScene->cubeSize; }
};

// Execute our example class
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int result = -1;
	try {
		if (!OVR_SUCCESS(ovr_Initialize(nullptr))) {
			FAIL("Failed to initialize the Oculus SDK");
		}
		AllocConsole();
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
		result = SimApp().run();
	}
	catch (std::exception & error) {
		OutputDebugStringA(error.what());
		std::cerr << error.what() << std::endl;
	}
	ovr_Shutdown();
	return result;
}