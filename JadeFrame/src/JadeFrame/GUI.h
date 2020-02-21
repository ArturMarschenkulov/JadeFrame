#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

void GUI_init(GLFWwindow* window);
void GUI_new_frame();
void GUI_render();
void GUI_destroy();