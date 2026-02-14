#include <cstdio>
#include <array>
#include <cmath>

#include "utility.h"

#include <GLFW/glfw3.h>
#include <glm/ext.hpp>

// ============================================================================
// CALLBACK FUNCTIONS
// ============================================================================

void MouseMoveCallback(GLFWwindow* wnd, double x, double y)
{
    GLState* state = static_cast<GLState*>(glfwGetWindowUserPointer(wnd));

    if (state->firstMouse) {
        state->lastMouseX = x;
        state->lastMouseY = y;
        state->firstMouse = false;
    }

    double dx = x - state->lastMouseX;
    double dy = state->lastMouseY - y; // Reversed: y goes from bottom to top

    state->lastMouseX = x;
    state->lastMouseY = y;

    // Only rotate camera if left mouse button is pressed
    if (state->leftMousePressed) {
        const float sensitivity = 0.1f;

        state->yaw += static_cast<float>(dx) * sensitivity;
        state->pitch += static_cast<float>(dy) * sensitivity;

        // Constrain pitch
        if (state->pitch > 89.0f) state->pitch = 89.0f;
        if (state->pitch < -89.0f) state->pitch = -89.0f;
    }
}

void MouseButtonCallback(GLFWwindow* wnd, int button, int action, int)
{
    GLState* state = static_cast<GLState*>(glfwGetWindowUserPointer(wnd));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            state->leftMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            state->leftMousePressed = false;
        }
    }
}

void MouseScrollCallback(GLFWwindow* wnd, double, double dy)
{
    GLState* state = static_cast<GLState*>(glfwGetWindowUserPointer(wnd));

    if (state->mode == 3) {
        glm::vec3 front;
        front.x = cos(glm::radians(state->yaw)) * cos(glm::radians(state->pitch));
        front.y = sin(glm::radians(state->pitch));
        front.z = sin(glm::radians(state->yaw)) * cos(glm::radians(state->pitch));
        front = glm::normalize(front);

        state->pos += front * (static_cast<float>(dy) * 0.5f);
        state->gaze = state->pos + front;
    } else {
        state->cameraDistance -= static_cast<float>(dy) * 0.5f;
        state->cameraDistance = glm::clamp(state->cameraDistance, 2.0f, 50.0f);
    }
}

void FramebufferChangeCallback(GLFWwindow* wnd, int w, int h)
{
    GLState* state = static_cast<GLState*>(glfwGetWindowUserPointer(wnd));
    state->width = w;
    state->height = h;
}

void KeyboardCallback(GLFWwindow* wnd, int key, int, int action, int)
{
    GLState* state = static_cast<GLState*>(glfwGetWindowUserPointer(wnd));

    // Handle key press/release
    if (action == GLFW_PRESS) {
        // Camera mode switching
        if (key == GLFW_KEY_P) {
            state->mode = (state->mode + 1) % 4;
            printf("Camera mode: %d\n", state->mode);
        }
        if (key == GLFW_KEY_O) {
            state->mode = (state->mode == 0) ? 3 : (state->mode - 1);
            printf("Camera mode: %d\n", state->mode);
        }

        // Time control
        if (key == GLFW_KEY_L) {
            state->timeSpeed += 0.5f;
            if (state->timeSpeed > 5.0f) {
                state->timeSpeed = -2.0f; // Reverse time!
            }
            printf("Time speed: %.1fx\n", state->timeSpeed);
        }
        if (key == GLFW_KEY_K) {
            state->timeSpeed -= 0.5f;
            if (state->timeSpeed < -2.0f) {
                state->timeSpeed = 0.1f;
            }
            printf("Time speed: %.1fx\n", state->timeSpeed);
        }

        // WASD movement
        if (key == GLFW_KEY_W) state->wPressed = true;
        if (key == GLFW_KEY_A) state->aPressed = true;
        if (key == GLFW_KEY_S) state->sPressed = true;
        if (key == GLFW_KEY_D) state->dPressed = true;
    }
    else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_W) state->wPressed = false;
        if (key == GLFW_KEY_A) state->aPressed = false;
        if (key == GLFW_KEY_S) state->sPressed = false;
        if (key == GLFW_KEY_D) state->dPressed = false;
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Update camera position for FPS mode
void UpdateFPSCamera(GLState& state, float deltaTime)
{
    // Calculate camera direction
    glm::vec3 front;
    front.x = cos(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    front.y = sin(glm::radians(state.pitch));
    front.z = sin(glm::radians(state.yaw)) * cos(glm::radians(state.pitch));
    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, state.up));

    const float cameraSpeed = 5.0f * deltaTime;

    // WASD movement (only in FPS mode)
    if (state.mode == 3) {
        if (state.wPressed) state.pos += cameraSpeed * front;
        if (state.sPressed) state.pos -= cameraSpeed * front;
        if (state.aPressed) state.pos -= cameraSpeed * right;
        if (state.dPressed) state.pos += cameraSpeed * right;
    }

    // Update gaze point
    state.gaze = state.pos + front;
}

// Calculate planet position in world space (for orbit camera)
glm::vec3 GetPlanetPosition(int planetIndex, float time)
{
    if (planetIndex == 0) {
        // Earth at origin
        return glm::vec3(0.0f);
    }
    else if (planetIndex == 1) {
        // Moon orbiting Earth
        float moonAngle = time * 0.5f;
        return glm::vec3(5.0f * cos(moonAngle), 0.0f, 5.0f * sin(moonAngle));
    }
    else if (planetIndex == 2) {
        // Moon's moon
        float moonAngle = time * 0.5f;
        glm::vec3 moonPos = glm::vec3(5.0f * cos(moonAngle), 0.0f, 5.0f * sin(moonAngle));

        float moonMoonAngle = time * 1.0f;
        glm::vec3 offset = glm::vec3(2.0f * cos(moonMoonAngle), 0.0f, 2.0f * sin(moonMoonAngle));

        return moonPos + offset;
    }
    return glm::vec3(0.0f);
}

// Update camera for orbit mode
void UpdateOrbitCamera(GLState& state)
{
    glm::vec3 planetPos = GetPlanetPosition(state.mode, state.currentTime);

    float orbitAngle    = glm::radians(state.yaw);
    float verticalAngle = glm::radians(glm::clamp(state.pitch, -89.0f, 89.0f));

    state.pos.x = planetPos.x + state.cameraDistance * cos(orbitAngle) * cos(verticalAngle);
    state.pos.y = planetPos.y + state.cameraDistance * sin(verticalAngle);
    state.pos.z = planetPos.z + state.cameraDistance * sin(orbitAngle) * cos(verticalAngle);

    state.gaze = planetPos;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(int, const char*[])
{
    // Initialize state
    CallbackPointersGLFW callbacks;
    GLState state("Planet Renderer - Phase 1", 1280, 720, callbacks);

    printf("=== Controls ===\n");
    printf("P/O: Switch camera mode (Orbit Earth/Moon/Moon's Moon/FPS)\n");
    printf("Left Mouse + Drag: Rotate camera\n");
    printf("Mouse Scroll: Zoom in/out\n");
    printf("WASD: Move camera (FPS mode only)\n");
    printf("L/K: Speed up / Slow down time\n");
    printf("================\n\n");

    // Load shaders
    ShaderGL planetVS = ShaderGL(ShaderGL::VERTEX, "shaders/planet.vert");
    ShaderGL planetFS = ShaderGL(ShaderGL::FRAGMENT, "shaders/planet.frag");
    ShaderGL earthFS = ShaderGL(ShaderGL::FRAGMENT, "shaders/earth.frag");
    ShaderGL cloudFS = ShaderGL(ShaderGL::FRAGMENT, "shaders/cloud.frag");
    ShaderGL bgVS = ShaderGL(ShaderGL::VERTEX, "shaders/background.vert");
    ShaderGL bgFS = ShaderGL(ShaderGL::FRAGMENT, "shaders/background.frag");
    ShaderGL sunFS = ShaderGL(ShaderGL::FRAGMENT, "shaders/sun.frag");
    ShaderGL shadowVS = ShaderGL(ShaderGL::VERTEX, "shaders/shadow.vert");
    ShaderGL shadowFS = ShaderGL(ShaderGL::FRAGMENT, "shaders/shadow.frag");

    // Load meshes
    MeshGL sphereMesh = MeshGL("meshes/sphere_5k.obj");

    // Load textures
    TextureGL earthTex = TextureGL("textures/2k_earth_daymap.jpg", TextureGL::LINEAR, TextureGL::REPEAT);
    TextureGL earthSpecular = TextureGL("textures/2k_earth_specular_map.png", TextureGL::LINEAR, TextureGL::REPEAT);
    TextureGL earthNight = TextureGL("textures/2k_earth_nightmap_alpha.png", TextureGL::LINEAR, TextureGL::REPEAT);
    TextureGL earthClouds = TextureGL("textures/2k_earth_clouds_alpha.png", TextureGL::LINEAR, TextureGL::REPEAT);
    TextureGL moonTex = TextureGL("textures/2k_moon.jpg", TextureGL::LINEAR, TextureGL::REPEAT);
    TextureGL jupiterTex = TextureGL("textures/2k_jupiter.jpg", TextureGL::LINEAR, TextureGL::REPEAT);
    TextureGL starsTex = TextureGL("textures/2k_stars_milky_way.jpg", TextureGL::LINEAR, TextureGL::REPEAT);

    // Create shadow framebuffer
    ShadowFBO shadowFBO(2048, 2048);

    // Set OpenGL state
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Uniform locations
    constexpr GLuint U_MODEL = 0;
    constexpr GLuint U_VIEW = 1;
    constexpr GLuint U_PROJ = 2;
    constexpr GLuint U_NORMAL = 3;
    constexpr GLuint U_LIGHT_DIR = 4;
    constexpr GLuint U_CAMERA_POS = 5;
    constexpr GLuint U_LIGHT_COLOR = 6;
    constexpr GLuint U_LIGHT_VP = 7;
    constexpr GLuint T_ALBEDO = 0;
    constexpr GLuint T_SHADOW = 1;
    constexpr GLuint T_SPECULAR = 2;
    constexpr GLuint T_NIGHT = 3;

    float lastFrameTime = static_cast<float>(glfwGetTime());

    // ========================================================================
    // RENDER LOOP
    // ========================================================================
    while (!glfwWindowShouldClose(state.window))
    {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrameTime;
        lastFrameTime = currentFrame;

        // Update time with speed multiplier
        state.currentTime += deltaTime * state.timeSpeed;

        // Poll events
        glfwPollEvents();

        // Update camera based on mode
        if (state.mode == 3) {
            // FPS mode
            UpdateFPSCamera(state, deltaTime);
        } else {
            // Orbit mode (0, 1, 2)
            UpdateOrbitCamera(state);
        }

        // Calculate matrices
        glm::mat4 proj = glm::perspective(glm::radians(50.0f),
                                          static_cast<float>(state.width) / static_cast<float>(state.height),
                                          0.01f, 1000.0f);
        glm::mat4 view = glm::lookAt(state.pos, state.gaze, state.up);
        
        // Orthographic projection for background elements (stars, sun)
        float aspect = float(state.width) / float(state.height);
        float orthoSize = 600.0f; // >= 500 (stars radius) and >= 100 (sun distance)
        glm::mat4 orthoProj = glm::ortho(-orthoSize * aspect, orthoSize * aspect,-orthoSize, orthoSize, 0.1f, 2000.0f);


        // Calculate rotating light direction (sun)
        float sunAngle = state.currentTime * 0.1f;
        glm::vec3 lightDir = glm::normalize(glm::vec3(cos(sunAngle), 0.0f, sin(sunAngle)));
        glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f);

        // ====================================================================
        // SHADOW PASS
        // ====================================================================
        // Create light's view and projection matrices
        glm::vec3 lightUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 lightRight = glm::normalize(glm::cross(lightUp, lightDir));
        glm::vec3 lightActualUp = glm::cross(lightDir, lightRight);
        
        glm::mat4 lightView = glm::lookAt(
            -lightDir * 20.0f,  // Position far away in direction of light
            glm::vec3(0.0f),     // Look at origin
            lightActualUp        // Up vector
        );
        
        // Orthographic projection that covers the scene
        float shadowOrthoSize = 15.0f;
        glm::mat4 lightProj = glm::ortho(-shadowOrthoSize, shadowOrthoSize, -shadowOrthoSize, shadowOrthoSize, 1.0f, 50.0f);
        glm::mat4 lightVP = lightProj * lightView;
        
        // Bind shadow framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO.fboId);
        glViewport(0, 0, shadowFBO.width, shadowFBO.height);
        
        // Clear with large depth value
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // Clear color buffer with large value
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
        // Use shadow shaders
        glUseProgramStages(state.renderPipeline, GL_VERTEX_SHADER_BIT, shadowVS.shaderId);
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT, shadowFS.shaderId);
        
        // Render all planets to shadow map
        auto renderShadow = [&](const glm::mat4& model) {
            glActiveShaderProgram(state.renderPipeline, shadowVS.shaderId);
            glUniformMatrix4fv(U_MODEL, 1, false, glm::value_ptr(model));
            glUniformMatrix4fv(U_VIEW, 1, false, glm::value_ptr(lightView));
            glUniformMatrix4fv(U_PROJ, 1, false, glm::value_ptr(lightProj));
            glBindVertexArray(sphereMesh.vaoId);
            glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr);
        };
        
        // Earth
        float earthRotation = state.currentTime * 0.2f;
        glm::mat4 earthModel = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        earthModel = glm::scale(earthModel, glm::vec3(1.0f));
        renderShadow(earthModel);
        
        // Moon
        float moonOrbitAngle = state.currentTime * 0.5f;
        float moonRotation = state.currentTime * 0.3f;
        glm::mat4 moonOrbit = glm::rotate(glm::mat4(1.0f), moonOrbitAngle, glm::vec3(0, 1, 0));
        glm::mat4 moonTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
        glm::mat4 moonRotate = glm::rotate(glm::mat4(1.0f), moonRotation, glm::vec3(0, 1, 0));
        glm::mat4 moonScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));
        glm::mat4 moonModel = earthModel * moonOrbit * moonTranslate * moonRotate * moonScale;
        renderShadow(moonModel);
        
        // Moon's moon
        float moonMoonOrbitAngle = state.currentTime * 1.0f;
        float moonMoonRotation = state.currentTime * 0.7f;
        glm::mat4 moonMoonOrbit = glm::rotate(glm::mat4(1.0f), moonMoonOrbitAngle, glm::vec3(0, 1, 0));
        glm::mat4 moonMoonTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
        glm::mat4 moonMoonRotate = glm::rotate(glm::mat4(1.0f), moonMoonRotation, glm::vec3(0, 1, 0));
        glm::mat4 moonMoonScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
        glm::mat4 moonMoonModel = moonModel * moonMoonOrbit * moonMoonTranslate * moonMoonRotate * moonMoonScale;
        renderShadow(moonMoonModel);
        
        // Unbind shadow framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ====================================================================
        // MAIN RENDERING PASS
        // ====================================================================

        // Clear (reset clear color to black for main rendering)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glViewport(0, 0, state.width, state.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ====================================================================
        // RENDER BACKGROUND (Stars)
        // ====================================================================
        glDepthMask(GL_FALSE); // Don't write to depth buffer
        glDisable(GL_CULL_FACE);

        glUseProgramStages(state.renderPipeline, GL_VERTEX_SHADER_BIT, bgVS.shaderId);
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT, bgFS.shaderId);

        glActiveShaderProgram(state.renderPipeline, bgVS.shaderId);
        {
            // Very large sphere centered on camera
            glm::mat4 bgModel = glm::translate(glm::mat4(1.0f), state.pos);
            bgModel = glm::scale(bgModel, glm::vec3(1000.0f));
            glm::mat3 normalMat = glm::mat3(1.0f);

            glUniformMatrix4fv(U_MODEL, 1, false, glm::value_ptr(bgModel));
            glUniformMatrix4fv(U_VIEW, 1, false, glm::value_ptr(view));
            glUniformMatrix4fv(U_PROJ, 1, false, glm::value_ptr(proj)); // Use perspective
            glUniformMatrix3fv(U_NORMAL, 1, false, glm::value_ptr(normalMat));
        }

        glActiveShaderProgram(state.renderPipeline, bgFS.shaderId);
        {
            glActiveTexture(GL_TEXTURE0 + T_ALBEDO);
            glBindTexture(GL_TEXTURE_2D, starsTex.textureId);
        }

        glBindVertexArray(sphereMesh.vaoId);
        glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr);

        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);

        // ====================================================================
        // RENDER SUN
        // ====================================================================
        glUseProgramStages(state.renderPipeline, GL_VERTEX_SHADER_BIT, bgVS.shaderId);
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT, sunFS.shaderId);

        glActiveShaderProgram(state.renderPipeline, bgVS.shaderId);
        {
            // Small sphere far away in direction of light
            glm::vec3 sunPos = state.pos - lightDir * 100.0f;
            glm::mat4 sunModel = glm::translate(glm::mat4(1.0f), sunPos);
            sunModel = glm::scale(sunModel, glm::vec3(5.0f));
            glm::mat3 normalMat = glm::mat3(1.0f);

            glUniformMatrix4fv(U_MODEL, 1, false, glm::value_ptr(sunModel));
            glUniformMatrix4fv(U_VIEW, 1, false, glm::value_ptr(view));
            glUniformMatrix4fv(U_PROJ, 1, false, glm::value_ptr(orthoProj)); // Use orthographic
            glUniformMatrix3fv(U_NORMAL, 1, false, glm::value_ptr(normalMat));
        }

        glBindVertexArray(sphereMesh.vaoId);
        glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr);

        // ====================================================================
        // RENDER PLANETS
        // ====================================================================
        glUseProgramStages(state.renderPipeline, GL_VERTEX_SHADER_BIT, planetVS.shaderId);
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT, planetFS.shaderId);

        // Set common uniforms for all planets
        glActiveShaderProgram(state.renderPipeline, planetFS.shaderId);
        {
            glUniform3fv(U_LIGHT_DIR, 1, glm::value_ptr(lightDir));
            glUniform3fv(U_CAMERA_POS, 1, glm::value_ptr(state.pos));
            glUniform3fv(U_LIGHT_COLOR, 1, glm::value_ptr(lightColor));
            glUniformMatrix4fv(U_LIGHT_VP, 1, false, glm::value_ptr(lightVP));
            
            // Bind shadow map
            glActiveTexture(GL_TEXTURE0 + T_SHADOW);
            glBindTexture(GL_TEXTURE_2D, shadowFBO.colorTextureId);
        }

        // --------------------------------------------------------------------
        // EARTH (Planet 0)
        // --------------------------------------------------------------------
        // Use Earth-specific shader
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT, earthFS.shaderId);
        
        glActiveShaderProgram(state.renderPipeline, planetVS.shaderId);
        {
            float earthRotation = state.currentTime * 0.2f;
            glm::mat4 earthModel = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
            earthModel = glm::scale(earthModel, glm::vec3(1.0f));
            glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(earthModel));

            glUniformMatrix4fv(U_MODEL, 1, false, glm::value_ptr(earthModel));
            glUniformMatrix4fv(U_VIEW, 1, false, glm::value_ptr(view));
            glUniformMatrix4fv(U_PROJ, 1, false, glm::value_ptr(proj));
            glUniformMatrix3fv(U_NORMAL, 1, false, glm::value_ptr(normalMatrix));
        }

        glActiveShaderProgram(state.renderPipeline, earthFS.shaderId);
        {
            glActiveTexture(GL_TEXTURE0 + T_ALBEDO);
            glBindTexture(GL_TEXTURE_2D, earthTex.textureId);
            glActiveTexture(GL_TEXTURE0 + T_SPECULAR);
            glBindTexture(GL_TEXTURE_2D, earthSpecular.textureId);
            glActiveTexture(GL_TEXTURE0 + T_NIGHT);
            glBindTexture(GL_TEXTURE_2D, earthNight.textureId);
            
            glUniform3fv(U_LIGHT_DIR, 1, glm::value_ptr(lightDir));
            glUniform3fv(U_CAMERA_POS, 1, glm::value_ptr(state.pos));
            glUniform3fv(U_LIGHT_COLOR, 1, glm::value_ptr(lightColor));
            glUniformMatrix4fv(U_LIGHT_VP, 1, false, glm::value_ptr(lightVP));
            
            glActiveTexture(GL_TEXTURE0 + T_SHADOW);
            glBindTexture(GL_TEXTURE_2D, shadowFBO.colorTextureId);
        }

        glBindVertexArray(sphereMesh.vaoId);
        glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr);
        
        // --------------------------------------------------------------------
        // EARTH CLOUDS
        // --------------------------------------------------------------------
        // Enable alpha blending for clouds
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Don't write to depth buffer
        
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT, cloudFS.shaderId);
        
        glActiveShaderProgram(state.renderPipeline, planetVS.shaderId);
        {
            // Clouds stay still (no rotation) while Earth rotates
            glm::mat4 cloudModel = glm::mat4(1.0f); // Identity matrix - no rotation
            cloudModel = glm::scale(cloudModel, glm::vec3(1.015f)); // Slightly larger than Earth
            glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(cloudModel));

            glUniformMatrix4fv(U_MODEL, 1, false, glm::value_ptr(cloudModel));
            glUniformMatrix4fv(U_VIEW, 1, false, glm::value_ptr(view));
            glUniformMatrix4fv(U_PROJ, 1, false, glm::value_ptr(proj));
            glUniformMatrix3fv(U_NORMAL, 1, false, glm::value_ptr(normalMatrix));
        }

        glActiveShaderProgram(state.renderPipeline, cloudFS.shaderId);
        {
            glActiveTexture(GL_TEXTURE0 + T_ALBEDO);
            glBindTexture(GL_TEXTURE_2D, earthClouds.textureId);
            glUniform3fv(U_LIGHT_DIR, 1, glm::value_ptr(lightDir));
            glUniform3fv(U_LIGHT_COLOR, 1, glm::value_ptr(lightColor));
        }

        glBindVertexArray(sphereMesh.vaoId);
        glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr);
        
        // Restore render state
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        
        // Switch back to regular planet shader for other planets
        glUseProgramStages(state.renderPipeline, GL_FRAGMENT_SHADER_BIT, planetFS.shaderId);

        // --------------------------------------------------------------------
        // MOON (Planet 1) - Orbits Earth
        // --------------------------------------------------------------------
        glActiveShaderProgram(state.renderPipeline, planetVS.shaderId);
        {
            float moonOrbitAngle = state.currentTime * 0.5f;
            float moonRotation = state.currentTime * 0.3f;

            // Hierarchical: Earth transform first
            float earthRotation = state.currentTime * 0.2f;
            glm::mat4 earthModel = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));

            // Then Moon orbit and rotation
            glm::mat4 moonOrbit = glm::rotate(glm::mat4(1.0f), moonOrbitAngle, glm::vec3(0, 1, 0));
            glm::mat4 moonTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
            glm::mat4 moonRotate = glm::rotate(glm::mat4(1.0f), moonRotation, glm::vec3(0, 1, 0));
            glm::mat4 moonScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.27f)); // Moon is ~1/4 size

            glm::mat4 moonModel = earthModel * moonOrbit * moonTranslate * moonRotate * moonScale;
            glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(moonModel));

            glUniformMatrix4fv(U_MODEL, 1, false, glm::value_ptr(moonModel));
            glUniformMatrix4fv(U_VIEW, 1, false, glm::value_ptr(view));
            glUniformMatrix4fv(U_PROJ, 1, false, glm::value_ptr(proj));
            glUniformMatrix3fv(U_NORMAL, 1, false, glm::value_ptr(normalMatrix));
        }

        glActiveShaderProgram(state.renderPipeline, planetFS.shaderId);
        {
            glActiveTexture(GL_TEXTURE0 + T_ALBEDO);
            glBindTexture(GL_TEXTURE_2D, moonTex.textureId);
        }

        glBindVertexArray(sphereMesh.vaoId);
        glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr);

        // --------------------------------------------------------------------
        // MOON'S MOON (Planet 2) - Orbits Moon
        // --------------------------------------------------------------------
        glActiveShaderProgram(state.renderPipeline, planetVS.shaderId);
        {
            float moonOrbitAngle = state.currentTime * 0.5f;
            float moonRotation = state.currentTime * 0.3f;
            float moonMoonOrbitAngle = state.currentTime * 1.0f;
            float moonMoonRotation = state.currentTime * 0.7f;

            // Hierarchical: Earth -> Moon -> Moon's Moon
            float earthRotation = state.currentTime * 0.2f;
            glm::mat4 earthModel = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));

            glm::mat4 moonOrbit = glm::rotate(glm::mat4(1.0f), moonOrbitAngle, glm::vec3(0, 1, 0));
            glm::mat4 moonTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
            glm::mat4 moonRotate = glm::rotate(glm::mat4(1.0f), moonRotation, glm::vec3(0, 1, 0));
            glm::mat4 moonScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.27f));

            glm::mat4 moonModel = earthModel * moonOrbit * moonTranslate * moonRotate * moonScale;

            // Moon's moon orbit
            glm::mat4 moonMoonOrbit = glm::rotate(glm::mat4(1.0f), moonMoonOrbitAngle, glm::vec3(0, 1, 0));
            glm::mat4 moonMoonTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
            glm::mat4 moonMoonRotate = glm::rotate(glm::mat4(1.0f), moonMoonRotation, glm::vec3(0, 1, 0));
            glm::mat4 moonMoonScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)); // Smaller than moon

            glm::mat4 moonMoonModel = moonModel * moonMoonOrbit * moonMoonTranslate * moonMoonRotate * moonMoonScale;
            glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(moonMoonModel));

            glUniformMatrix4fv(U_MODEL, 1, false, glm::value_ptr(moonMoonModel));
            glUniformMatrix4fv(U_VIEW, 1, false, glm::value_ptr(view));
            glUniformMatrix4fv(U_PROJ, 1, false, glm::value_ptr(proj));
            glUniformMatrix3fv(U_NORMAL, 1, false, glm::value_ptr(normalMatrix));
        }

        glActiveShaderProgram(state.renderPipeline, planetFS.shaderId);
        {
            glActiveTexture(GL_TEXTURE0 + T_ALBEDO);
            glBindTexture(GL_TEXTURE_2D, jupiterTex.textureId);
        }

        glBindVertexArray(sphereMesh.vaoId);
        glDrawElements(GL_TRIANGLES, sphereMesh.indexCount, GL_UNSIGNED_INT, nullptr);

        // Swap buffers
        glfwSwapBuffers(state.window);
    }

    return 0;
}