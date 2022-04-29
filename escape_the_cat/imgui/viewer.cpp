#define GLFW_INCLUDE_NONE
//#define IMGUI_DEBUG_LOG_IO IMGUI_DEBUG_LOG
#define IMGUI_USE_BGRA_PACKED_COLOR

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cmath>
#include <iostream>
#include <fstream>

// Game constants
constexpr float RADIUS = 500.0;
constexpr int N_TURNS_MAX = 350;
constexpr float MIN_DISTANCE_AT_END = 80;
constexpr float MOUSE_SPEED = 10.0;
constexpr float GWIDTH = 1920;
constexpr float GHEIGHT = 1080;
const ImVec2 ORIGIN = ImVec2{0, 0};

inline ImVec2 Rescale(float s, const ImVec2& v) { return {s * v.x, s * v.y }; }
inline ImVec2 Translate(const ImVec2& u, const ImVec2& v) { return { u.x + v.x, u.y + v.y }; }

struct Point {
    double x;
    double y;
};


int main(int argc, char *argv[]) {
    using std::cerr;
    using std::endl;

    // Initialize glfw which manages window creation etc...
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // The main window
    auto window = glfwCreateWindow(GWIDTH, GHEIGHT, "EscapeTheCat", nullptr, nullptr);
    if (!window) {
        cerr << "GLFW failed to create the window" << endl;
        return EXIT_FAILURE;
    }

    // Create the context for the gui
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if (!gladLoaderLoadGL()) {
        cerr << "Failed to load OpenGL" << endl;
        return EXIT_FAILURE;
    }

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450 core");

  auto& style = ImGui::GetStyle();
  ImGui::StyleColorsDark(&style);
  style.FrameBorderSize = 1.0f;
  style.FrameRounding = 5;
  style.AntiAliasedFill = true;
  style.CircleTessellationMaxError = 0.1f;

  // The windows
  bool show_editor_window = false;
  bool show_main_window = true;
  bool show_control_window = true;
  bool show_log_window = true;

  float log_window_ratio = 0.25f;
  float main_window_ratio = 0.5f;
  float control_window_ratio = 0.25f;

  ImVec2 log_window_pos{ 0, 0 };
  ImVec2 log_window_size{ 0.25 * GWIDTH, GHEIGHT };
  ImVec2 control_window_pos{ 0.85 * GWIDTH, 0 };
  ImVec2 control_window_size{ 0.15 * GWIDTH, GHEIGHT };
  ImVec2 main_window_pos{ 0.25 * GWIDTH, 0 };
  ImVec2 main_window_size{ 0.6 * GWIDTH, GHEIGHT };
  ImVec2 editor_window_pos{ 0.15 * GWIDTH, 0 };
  float editor_window_alpha{ 0.4f };
  // Colors for the different objects
  ImU32 water_color = IM_COL32(219, 98, 78, 180);
  int cat_colb = 239;
  int cat_colg = 138;
  int cat_colr = 98;
  int cat_cola = 255;
  ImU32 cat_color = IM_COL32(cat_colr, cat_colg, cat_colb, cat_cola);
  int mouse_colb = 153;
  int mouse_colg = 153;
  int mouse_colr = 153;
  int mouse_cola = 255;
  ImU32 mouse_color = IM_COL32(mouse_colr, mouse_colg, mouse_colb, mouse_cola);

  //auto scale_color = [](float scale, ImVec4& vec) { vec[0]*=scale; vec[1]*=scale; vec[2]*=scale; vec[3]*=scale; };
  // The data
  ImVec2 cat { RADIUS, 0.0 };
  ImVec2 mouse{ 5.0, 15.0 };
  float cat_speed = 10.0f;

  auto cat_colormap = [rc=cat_colr, gc=cat_colg, bc=cat_colb, ac=cat_cola, rm=mouse_colr, gm=mouse_colg, bm=mouse_colb](int mid, int end) {
      ImVector<ImU32> ret;
      const int n = end - mid;
      ret.reserve(end);
      for (int i=0; i<mid; ++i) {
          ret.push_back(IM_COL32((int)ImCeil(rc - i*rc/mid), (int)ImCeil(gc - i*gc/mid), (int)ImCeil(bc - i*bc/mid), ac));
      }
      for (int i=0; i<n; ++i) {
          ret.push_back(IM_COL32((int)ImFloor(i * rm/n), (int)ImFloor(gm/n), (int)ImFloor(gm/n), ac));
      }
      return ret;
  };

  float mouse_angle = ImAtan2(mouse.x, mouse.y);
  float cat_angle = ImAtan2(cat.x, cat.y);
  float mouse_n_turn = ImCeil((RADIUS - ImSqrt(ImLengthSqr(mouse))) / MOUSE_SPEED);
  ImVec2 cat_target { RADIUS * ImCos(mouse_angle), RADIUS * ImSin(mouse_angle) };
  //ImVec2 cat_target = Rescale((MOUSE_SPEED * mouse_n_turn), mouse);
  float cat_n_turn = ImCeil(RADIUS * ImFabs(mouse_angle - cat_angle) / cat_speed);
  int n_turn = ImMin(mouse_n_turn, cat_n_turn);
  float final_cat_angle = ImLinearSweep(cat_angle, mouse_angle, n_turn * cat_speed);
  ImVec2 final_cat {RADIUS * ImCos(final_cat_angle), RADIUS * ImSin(final_cat_angle)};
  ImVec2 final_mouse = ImLerp(mouse, cat_target, MOUSE_SPEED * n_turn);
  float cat_mouse_dist_2 = ImLengthSqr(ImVec2(cat.x - mouse.x, cat.y - mouse.y));
  float final_cat_mouse_dist_2 = ImLengthSqr(ImVec2(final_cat.x - final_mouse.x, final_cat.y - final_mouse.y));

  int turn = 0;

  // The main drawing loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Update data
    float mouse_angle = M_PI / 4.0f;
    float mouse_radius_2 = ImLengthSqr(mouse);
    float mouse_dist = RADIUS - ImSqrt(mouse_radius_2);
    float mouse_n_turn = ImCeil(mouse_dist / MOUSE_SPEED);

    float cat_angle = 0.0f;
    float cat_arcdist = RADIUS * ImFabs(mouse_angle - cat_angle);
    float cat_n_turn = ImCeil(cat_arcdist / cat_speed);

    // Then do Lerp for n_turn
    float n_turn = ImMin(mouse_n_turn, cat_n_turn);

    // The Control window
    if (show_control_window) {
        ImGui::SetNextWindowSize(control_window_size);
        ImGui::SetNextWindowPos(control_window_pos);
        ImGui::Begin("Controls", &show_control_window);

        auto margin_x = ImGui::GetStyle().WindowPadding.x;
        auto btnSize = ImVec2(ImGui::GetWindowWidth()/2, 32.0f);
        ImGui::Button("Reset", btnSize);
        ImGui::SameLine();
        if (ImGui::Button("Exit", btnSize)) {
            ImGui::End();
            break;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        btnSize.x = 128.0f;
        ImGui::Dummy(btnSize);

        ImGui::Checkbox("Edit Style", &show_editor_window);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Dummy(btnSize);
        ImGui::Spacing();

        ImGui::BeginGroup();
        ImGui::Button("Undo", btnSize);
        ImGui::EndGroup();

        ImGui::Dummy(btnSize);
        ImGui::Separator();
        ImGui::Spacing();



        ImGui::End();
    }

    if (show_editor_window) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, editor_window_alpha);
        ImGui::SetNextWindowPos(editor_window_pos, ImGuiCond_Once);
        ImGui::Begin("Style Editor");
        ImGui::ShowStyleEditor();
        ImGui::PopStyleVar();
        ImGui::End();
    }


    // The Main window
    if (show_main_window) {
        ImGui::SetNextWindowSize(main_window_size, ImGuiCond_Once);
        ImGui::SetNextWindowPos(main_window_pos, ImGuiCond_Once);
        ImGui::Begin("Cat And Mouse", &show_main_window);

        const float size = (float)ImMin(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
        const float scale = ImMin(1.0f, 0.75f * size / (2*RADIUS));
        const float local_circle_radius = scale * RADIUS;
        ImVec2 local_origin{ ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth()/2, ImGui::GetWindowHeight()/2 };

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Add the background pool
        draw_list->AddCircleFilled(local_origin, scale * RADIUS, water_color);
        // Add the triangle formed by mouse, cat and cat_target
        ImVec2 rmouse = Rescale(scale, mouse);
        ImVec2 rcat = Rescale(scale, cat);
        ImVec2 rtarget = Rescale(scale, cat_target);

        IM_ASSERT(scale * 2*RADIUS < (0.8 * ImGui::GetWindowHeight()));
        // IM_ASSERT(ImLengthSqr(rmouse) < ImLengthSqr(rcat));
        // IM_ASSERT(ImLengthSqr(rcat) < scale*scale * RADIUS*RADIUS + 100);
        // IM_ASSERT(ImLengthSqr(rtarget) < scale*scale * RADIUS*RADIUS + 100);

        //draw_list->AddTriangle(ImVec2(local_origin.x + rmouse.x, local_origin.y+rmouse.y), ImVec2(local_origin.x + rcat.x, local_origin.y+rcat.y), ImVec2(local_origin.x + rtarget.x, local_origin.y+rtarget.y), cat_color, 2.0f);
        float t = cat_n_turn < mouse_n_turn ? (mouse_n_turn - cat_n_turn) / (cat_n_turn) : (cat_n_turn - mouse_n_turn) / (mouse_n_turn);

        ImVec4 cc{ cat_colb, cat_colg, cat_colr, cat_cola};
        ImVec4 mc{ mouse_colb, mouse_colg, mouse_colr, mouse_cola };
        ImVec4 mw{ 0, 0, 0, 255 };

        ImVec4 colc = ImLerp(mc, cc, t);
        ImVec4 colm = ImLerp(cc, mc, t);
        ImU32 col = ImGui::GetColorU32(colc);

        draw_list->AddTriangle(Translate(local_origin, Rescale(scale, mouse)), Translate(local_origin, Rescale(scale, cat)), Translate(local_origin, Rescale(scale, cat_target)), col, 3.0f);
         // draw_list->AddNgonFilled(scale * cat, scale * RADIUS * 0.042857f, cat_color, 5);  // Let the cat be 3% of the screen size
        // // Add mouse
        // draw_list->AddCircleFilled(scale * mouse, scale * RADIUS * 0.042857f, mouse_color);

        // for (int i=0; i<n_turn; ++i) {
        //     draw_list->AddLine(scale * (mouse + i * mouse_speed), scale * (mouse + (i+1) * mouse_speed), IM_COL32())
        // }
        // int vert_start_idx_mouse = vert_end_idx_cat;
        // draw_list->AddLine(scale * mouse, (local_circle_radius * (mouse.radius / RADIUS)) * local_mouse, mouse_color, 1.2f);
        // int vert_start_idx_mouse = draw_list->VtxCurrentIdx;
        // // Color gradient towards white to visually compare the time taken for the cat
        // // and the mouse to reach the current target
        // // TODO: Scale correctly to incorporate the Distance>80 rule
        // ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
        // ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
        // ImGui::ShadeVertsLinearColorGradientKeepAlpha(draw_list, vtx_start_idx, vtx_end_idx, vert_start->pos, vert_end->pos, cat_color, IM_COL32());



        ImGui::End();
    }

    // The logging window
    if (show_log_window) {
        ImGui::SetNextWindowSize(log_window_size, ImGuiCond_Once);
        ImGui::SetNextWindowPos(log_window_pos, ImGuiCond_Once);
        ImGui::Begin("Log", &show_log_window);
        ImGui::End();
    }

    // Render ImGui data
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glClear(GL_COLOR_BUFFER_BIT);
  }


  // Shutdown viewer
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
