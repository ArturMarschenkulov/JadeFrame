#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

void GUIinit(GLFWwindow* window);
void GUInewFrame();
void GUIrender();
void GUIdestroy();