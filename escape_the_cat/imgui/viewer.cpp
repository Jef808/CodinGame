#include <cstddef>
#define GLFW_INCLUDE_NONE
#define IMGUI_USE_BGRA_PACKED_COLOR
#define IMGUI_DEFINE_MATH_OPERATORS

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <deque>

// Game constants
constexpr float RADIUS = 500.0;
constexpr int N_TURNS_MAX = 350;
constexpr float MIN_DISTANCE_AT_END = 80;
constexpr float MOUSE_SPEED = 10.0;
constexpr float GWIDTH = 1920;
constexpr float GHEIGHT = 1080;
constexpr float MIN_ANGLE =
  0.1601712f; // The angle that gives a euclidean distance of 80 when the mouse is at the boundary
constexpr float FT_MIN = std::numeric_limits<float>::min();
constexpr float FT_MAX = std::numeric_limits<float>::max();

const ImVec2 ORIGIN{ 0.0f, 0.0f };
const ImVec2 E1{ 1.0f, 0.0f };
const ImVec2 E2{ 0.0f, 1.0f };
constexpr ImU32 color_water = IM_COL32(219, 98, 78, 180);
constexpr ImU32 color_cat = IM_COL32(138, 98, 190, 255);
constexpr ImU32 color_mouse = IM_COL32(153, 153, 153, 255);

ImU32 color_lerp(ImU32 current, ImU32 target, float ratio) {
    const int c_r = (int)(current >> IM_COL32_R_SHIFT) & 0xFF;
    const int c_g = (int)(current >> IM_COL32_G_SHIFT) & 0xFF;
    const int c_b = (int)(current >> IM_COL32_B_SHIFT) & 0xFF;

    const int d_r = ((int)(target >> IM_COL32_R_SHIFT) & 0xFF) - c_r;
    const int d_g = ((int)(target >> IM_COL32_G_SHIFT) & 0xFF) - c_g;
    const int d_b = ((int)(target >> IM_COL32_B_SHIFT) & 0xFF) - c_b;

    return ((c_r + (int)ratio*d_r) << IM_COL32_R_SHIFT) | ((c_g + (int)ratio*d_g) << IM_COL32_G_SHIFT) | ((c_b + (int)ratio*d_b) << IM_COL32_B_SHIFT) | (current & IM_COL32_A_MASK);
}

inline float sigmoid(float x) {
    return 1.0f / (1.0f + (float)std::exp(-x));
}

inline ImVec2&
MinRadial(ImVec2& v, const ImVec2 mx)
{
    v.x = mx.x > 0 ? v.x > mx.x ? mx.x : v.x : v.x < mx.x ? mx.x : v.x;
    v.y = mx.y > 0 ? v.y > mx.y ? mx.y : v.y : v.y < mx.y ? mx.y : v.y;
    return v;
}

inline ImVec2&
Clamp(ImVec2& v, const ImVec2 mn, const ImVec2 mx)
{
    v.x = v.x < mn.x ? mn.x : v.x > mx.x ? mx.x : v.x;
    v.y = v.y < mn.y ? mn.y : v.y > mx.y ? mx.y : v.y;
    return v;
}
inline ImVec2&
CeilRadial(ImVec2& v)
{
    v.x = v.x > 0 ? std::ceil(v.x) : std::floor(v.x);
    v.y = v.y > 0 ? std::ceil(v.y) : std::floor(v.y);
    return v;
}
inline ImVec2
CeilRadial(const ImVec2& v)
{
    float x = v.x > 0 ? std::ceil(v.x) : std::floor(v.x);
    float y = v.y > 0 ? std::ceil(v.y) : std::floor(v.y);
    return { x, y };
}
inline ImVec2&
Floor(ImVec2& v)
{
    v.x = std::floor(v.x);
    v.y = std::floor(v.y);
    return v;
}
inline ImVec2&
FloorRadial(ImVec2& v)
{
    v.x = v.x > 0.0f ? std::floor(v.x) : std::ceil(v.x);
    v.y = v.y > 0.0f ? std::floor(v.y) : std::ceil(v.y);
    return v;
}
inline ImVec2
FloorRadial(const ImVec2& v)
{
    float x = v.x > 0.0f ? std::floor(v.x) : std::ceil(v.x);
    float y = v.y > 0.0f ? std::floor(v.y) : std::ceil(v.y);
    return { x, y };
}
inline ImVec2
Clamp(const ImVec2& v, float mn, float mx)
{
    std::clamp(v.x, mn, mx);
    std::clamp(v.y, mn, mx);
    return v;
}
inline ImVec2&
Rescale(ImVec2& v, float s)
{
    v.x *= s;
    v.y *= s;
    return v;
}
inline ImVec2&
Translate(ImVec2& u, const ImVec2& v)
{
    u.x += v.x;
    u.y += v.y;
    return u;
}
inline ImVec2
Translate(const ImVec2& u, const ImVec2& v)
{
    return ImVec2(u.x + v.x, u.y + v.y);
}
inline ImVec2&
Rotate(ImVec2& u, float cos_a, float sin_a) {
    const ImVec2 tmp = u;
    u.x = cos_a * tmp.x - sin_a * tmp.y, sin_a * tmp.x + cos_a * tmp.y;
    return u;
}
/**
 * Use a trick to compute angle between two vectors in one call.
 * This is equivalent to atan2(b.x, b.y) - atan2(a.x, a.y).
 */
inline float
angle_between(const ImVec2& a, const ImVec2& b)
{
    float ax = a.x;
    float ay = a.y;
    float bx = b.x;
    float by = b.y;
    if (ax < 0.0f && bx < 0.0f) {
        ax *= -1;
        bx *= -1;
    }
    return ImAtan2(by * a.x - ay * b.x, b.x * a.x + by * ay);
}
/**
 * @pos is the arrow base, 2*half_sz is the length of the arrow, angle is its angle
 */
void DrawArrow(ImDrawList* draw_list, ImVec2 origin, ImVec2 beg, ImVec2 end, float width, ImU32 col)
{
    const float a = ImAtan2(end.y, end.x);
    const float cos_a = ImCos(a);
    const float sin_a = ImSin(a);
    const ImVec2 d = end - beg;
    const float sz = std::sqrt(d.x*d.x + d.y*d.y);
    ImVec2 c1{ 0.0f, width / 4 };
    ImVec2 c2{ 0.0f, -width / 4 };
    ImVec2 c3{ sz/2, -width / 4 };
    ImVec2 c4{ sz/2, width / 4 };
    draw_list->AddQuadFilled(
        Translate(Rotate(c1, cos_a, sin_a), origin + beg),
        Translate(Rotate(c2, cos_a, sin_a), origin + beg),
        Translate(Rotate(c3, cos_a, sin_a), origin + beg),
        Translate(Rotate(c4, cos_a, sin_a), origin + beg),
        col);
    ImVec2 c5 { sz/2, width/2 };
    ImVec2 c6 { sz/2, -width/2 };
    ImVec2 c7 { sz, 0.f };
    draw_list->AddTriangleFilled(
        Translate(Rotate(c5, cos_a, sin_a), origin + beg),
        Translate(Rotate(c6, cos_a, sin_a), origin + beg),
        Translate(Rotate(c7, cos_a, sin_a), origin + beg),
        col);
}
enum class Status
{
    ongoing,
    won,
    lost
};

inline float
normalize_angle(float angle) {
    return angle > IM_PI ? angle - 2*IM_PI : angle < IM_PI ? angle + 2*IM_PI : angle;
}

ImVec2
RoundUpRadial(const ImVec2& v) {
    std::array<ImVec2, 4> corners { ImVec2(std::floor(v.x), std::floor(v.y)), ImVec2(std::ceil(v.x), std::floor(v.y)), ImVec2(std::ceil(v.x), std::ceil(v.y)), ImVec2(std::floor(v.x), std::ceil(v.y)) };
    double r2 = v.x*v.x+v.y*v.y;
    int min = 0;
    double val = 0.0;
    for (int i=0; i<4; ++i) {
        const ImVec2& c = corners[i];
        const double cr2 = c.x*c.x + c.y*c.y;
        if (cr2 > r2 - 0.0000001 && cr2 < val) {
            val = cr2;
            min = i;
        }
    }
    return corners[min];
}
ImVec2
RoundDownRadial(const ImVec2& v) {
    std::array<ImVec2, 4> corners { ImVec2(std::floor(v.x), std::floor(v.y)), ImVec2(std::ceil(v.x), std::floor(v.y)), ImVec2(std::ceil(v.x), std::ceil(v.y)), ImVec2(std::floor(v.x), std::ceil(v.y)) };
    double r2 = v.x*v.x+v.y*v.y;
    int max = 0;
    double val = 0.0;
    for (int i=0; i<4; ++i) {
        const ImVec2& c = corners[i];
        const double cr2 = c.x*c.x + c.y*c.y;
        if (cr2 < r2 + 0.0000001 && cr2 > val) {
            val = cr2;
            max = i;
        }
    }
    return corners[max];
}

struct State
{

    State(const ImVec2& cat_, const ImVec2& mouse_, float cat_speed_, float cat_speed_radian_)
        : cat{ cat_ }
        , mouse{ mouse_ }
        , cat_speed{ cat_speed_ }
        , cat_speed_radian{ cat_speed_radian_ }
        , mouse_dir{ normalize_angle(ImAtan2(mouse_.y + (mouse_-cat_).y, mouse_.x + (mouse_-cat_).x)) }

    {
        mouse_dir_v = ImRotate(E1, ImCos(mouse_dir), ImSin(mouse_dir));
        const float a = angle_between(cat, mouse);
        cat_target = CeilRadial(ImRotate(cat, ImCos(a), ImSin(a)));
    }

    void reset(const ImVec2& cat_, const ImVec2& mouse_) {
        cat = cat_;
        mouse = mouse_;
        mouse_dir = normalize_angle(ImAtan2(mouse_.y + (mouse_-cat_).y, mouse_.x + (mouse_-cat_).x));
        mouse_dir_v = ImRotate(E1, ImCos(mouse_dir), ImSin(mouse_dir));
        cat_target = mouse_dir_v * RADIUS;

        IM_ASSERT(ImLengthSqr(mouse_dir_v) < 1.00001f);
    }

    /**
     * Number of turns for cat to reach the point of intersection
     * between the boundary and the line through the center and the mouse.
     */
    float n_turn_for_cat(const ImVec2& target) const
    {
        return std::ceil(ImAbs(angle_between(cat, target)) / cat_speed_radian);
    }

    /**
     * Number of turns that mouse would need to get to its target in a straight line.
     */
    float n_turn_for_mouse(const ImVec2& target) const
    {
        return std::ceil(std::sqrt(ImLengthSqr(target - mouse) / (MOUSE_SPEED*MOUSE_SPEED)));
    }

    /**
     * Score from 0(cat wins instantly) to 1(mouse wins instantly), linearly interpolating
     * between the difference they take to get to the boundary, would the mouse keep
     * going straight towards the nearest boundary point.
     * NOTE: If they get there at the same time, return value is 0.5.
     */
    ImU32 color_score() const {
        float n_turns_cat = n_turn_for_cat(mouse); // n_turn_for_cat(ImInvLength(mouse) * RADIUS)
        float n_turns_mouse = n_turn_for_mouse(ImRotate(ImVec2(RADIUS, 0.0f), ImCos(mouse_dir), ImSin(mouse_dir)) - mouse);
        float n_turns_winning = ImMin(n_turns_cat, n_turns_mouse);
        float n_turns_losing = ImMax(n_turns_cat, n_turns_mouse);

        ImU32 color_target = n_turns_cat < n_turns_mouse ? color_cat : color_mouse;

        return color_lerp(IM_COL32_WHITE, color_target, (n_turns_winning - n_turns_losing) / n_turns_losing);
    }

    /**
     * Update the mouse's target
     */
    void update_mouse_dir(float dir)
    {
        mouse_dir_v = ImRotate(E1, ImCos(dir), ImSin(dir));
        mouse_dir = dir;

        IM_ASSERT(ImLengthSqr(mouse_dir_v) < 1.001f);
        IM_ASSERT(ImLengthSqr(mouse_dir_v) < 0.99f);
    }

    Status get_status() const
    {
        if (turn > N_TURNS_MAX) {
            return Status::lost;
        }
        const float norm2 = ImLengthSqr(mouse);
        if (norm2 > RADIUS * RADIUS - 00000.1f) {
            return (ImAbs(angle_between(mouse, cat)) < MIN_ANGLE) ? Status::lost : Status::won;
        }
        return Status::ongoing;
    }

    /**
     * Let the state evolve for one turn.
     */
    void step()
    {
        IM_ASSERT(ImLengthSqr(mouse_dir_v) < 1.01f);
        IM_ASSERT(ImLengthSqr(mouse_dir_v) > 0.99f);
        IM_ASSERT(ImLengthSqr(cat) < RADIUS*RADIUS + 1.0f);
        IM_ASSERT(ImLengthSqr(cat) > RADIUS*RADIUS - 1.0f);

        const float x = mouse.x + mouse_dir_v.x * MOUSE_SPEED;
        const float y = mouse.y + mouse_dir_v.y * MOUSE_SPEED;
        mouse.x = x > 0.0f ? std::floor(x) : std::ceil(x);
        mouse.y = y > 0.0f ? std::floor(y) : std::ceil(y);

        const float a = angle_between(cat, mouse);
        const float cat_angle_delta = ImLinearSweep(0.0f, a, cat_speed_radian);

        cat = Rotate(cat, ImCos(cat_angle_delta), ImSin(cat_angle_delta));
        const ImVec2 new_cat_round { std::round(cat.x), std::round(cat.y) };

        const float n2 = ImLengthSqr(new_cat_round);
        if (n2 > RADIUS*RADIUS - 1.0f) {
            cat = RoundUpRadial(cat);
        }

        IM_ASSERT(ImLengthSqr(cat) > RADIUS*RADIUS - 1.0f);

        // NOTE: is this needed?
        cat_target = ImRotate(cat, ImCos(a), ImSin(a));

        ++turn;
    }

    ImVec2 cat;
    ImVec2 mouse;
    // The amount of lattice points the cat can traverse in one turn
    float cat_speed;
    // The amount of "radians" the cat can traverse in one turn
    float cat_speed_radian;
    // Angle between (1, 0) and the mouse's next translation
    float mouse_dir;
    // Vector based at origin in the direction of next step
    ImVec2 mouse_dir_v;
    // The point on boundary nearest to the mouse
    ImVec2 cat_target;

    int turn = 0;

}; // State

class Grid_colormap {
public:
    Grid_colormap() = default;

    /**
    * Prepare a diverging colormap interpolating col0 and col1.
    * The middle color is taken to be white, and we adjust n_samples
    * to ensure it is odd.
    *
    * Note that @n_samples is simply the number of points on the boundary
    * which we use to populate the colormap, we can sample as many points
    * in the interior of the disk as we want in @sample_disk().
    */
    void init_colormap(ImU32 col0, ImU32 col1, int n_points_, float mn_=-1.0f, float md_=0.0f, float mx_=1.0f) {
        n_points = n_points_ & 1 ? n_points_ : n_points_ + 1;
        mn = mn_;
        mx = mx_;
        extent = mx - mn;
        float d0 = (md_ - mn) / ((n_points - 1) / 2.0f);
        float d1 = (mx - md_) / ((n_points - 1) / 2.0f);

        for (int i=0; i < (n_points - 1) / 2; ++i) {
            ImU32 col = color_lerp(col0, IM_COL32_WHITE, i * d0);
            colormap.push_back(col);
        }
        colormap.push_back(IM_COL32_WHITE);
        for (int i=0; i < (n_points - 1) / 2; ++i) {
            ImU32 col = color_lerp(IM_COL32_WHITE, col1, i * d1);
            colormap.push_back(col);
        }
    }

    /**
     * Retrieve the value of the color associated with @m.
     */
    ImU32 get_color(float m) {
        float idx_ = (m - mn) * n_points / extent;
        float idx = std::clamp(idx_, 0.0f, (float)n_points - 1.0f);
        return colormap[static_cast<ptrdiff_t>(idx)];
    }

    /**
     * Note that only the cat's speed matters, its position can always be
     * normalized by rotation. We will assume the cat starts at angle 0.
     *
     * We simply draw @n_points straight lines from the given point different
     * points in the disk to the boundary, and compare the number of turns
     * needed to reach that boundary point for both the cat and the mouse.
     *
     * The i'th result corresponds to an angle increment of i * n_points.
     */
    void sample_point(const ImVec2& mouse, float cat_speed, ImU32 results[]) {
        const double max_mouse_n_turns = 50.0;
        const double max_cat_n_turns = 2.0 * M_PI * RADIUS / cat_speed;

        const double range = max_mouse_n_turns + max_cat_n_turns;
        const double dtheta = 2.0 * M_PI / n_points;

        for (int i = 0; i < n_points; ++i) {
            const double angle = i * dtheta;
            const int n_turns_cat = angle < M_PI ? std::ceil(angle * RADIUS / cat_speed) : std::ceil((2 * M_PI - angle) * RADIUS) / cat_speed;
            const double p[2] = { RADIUS * std::cos(angle), RADIUS * std::sin(angle) };
            const int n_turns_mouse = std::ceil((RADIUS - std::sqrt(p[0] * p[0] + p[1] * p[1])) / MOUSE_SPEED);
            results[i] = colormap[static_cast<ptrdiff_t>(std::round((n_turns_mouse - n_turns_cat) / range))];
        }
    }

    /**
     * Color the disk according to the results of a sampling.
     */
    void render_colored_disk(ImDrawList* draw_list, ImU32 results[], const ImVec2& origin, const float scale) {
        int first_white = 0;
        int mouse_threshold = 0;
        int second_white = 0;
        // We split the angles at the equal (white color) threshold. Note that it starts
        // with cat_color since the first angle is where the cat is.
        // Identify the threshold by it being a local maximum for the unsigned 32 bit integers
        ImU32 current = results[0];
        for (int i = 0; i < n_points - 2; ++i) {
            ImU32 next = results[i + 1];
            if (next < current) {
                first_white = i;
                break;
            }
            current = next;
        }
        current = results[first_white];
        for (int i = first_white; i < n_points - 2; ++i) {
            ImU32 next = results[i + 1];
            if (next > current) {
                mouse_threshold = i;
                break;
            }
            current = next;
        }
        current = results[mouse_threshold];
        for (int i = mouse_threshold; i < n_points - 2; ++i) {
            ImU32 next = results[i + 1];
            if (next < current) {
                second_white = i;
                break;
            }
            current = next;
        }

        const int Ns[5] = { 0, first_white, mouse_threshold, second_white, n_points - 1 };

        const float delta = 0.5f / RADIUS;  // Half a pixel arclength.
        const float width = RADIUS * scale / 2.0;
        //const int segment_per_arc = (int)width / 12;  // That's about 25
        for (int n = 0; n < 4; ++n) {
            const float a0 = Ns[n]  /n_points * 2.0f * IM_PI - delta;
            const float a1 = Ns[n+1]/n_points * 2.0f * IM_PI + delta;
            const int vert_start_idx = draw_list->VtxBuffer.Size;
            draw_list->PathArcTo(origin, (RADIUS - width / 2.0f), a0, a1, 0);
            draw_list->PathStroke(color_water, false, width);
            const int vert_end_idx = draw_list->VtxBuffer.Size;

            ImVec2 gradient_p0(origin.x + ImCos(a0) * (RADIUS * scale - width), origin.y + ImSin(a0) * (RADIUS * scale - width));
            ImVec2 gradient_p1(origin.x + ImCos(a1) * (RADIUS * scale - width), origin.y + ImSin(a1) * (RADIUS * scale - width));
            ImU32 col0 = n < 0.5f ? color_cat : n < 1.5f ? IM_COL32_WHITE : n < 2.5f ? color_mouse : IM_COL32_WHITE;
            ImU32 col1 = n < 0.5f ? IM_COL32_WHITE : n < 1.5f ? color_mouse : n < 2.5f ? IM_COL32_WHITE : colormap[n_points - 1];
            ImGui::ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, col0, col1);
        }
    }

private:
    ImVector<ImU32> colormap;
    int n_points;
    double mn;
    double mx;
    double extent;
};

/**
 * Generate an automatic playthrough, accumulating data
 * for the colormapping
 */
Status
sample(const ImVec2& cat_, const ImVec2& mouse_, float cat_speed_)
{
    float cat_speed_rad_ = cat_speed_ / (2.0f * IM_PI * RADIUS);

    auto state = State(cat_, mouse_, cat_speed_, cat_speed_rad_);
    Status result = state.get_status();

    while (result == Status::ongoing) {
        const float a = angle_between(state.cat, state.mouse);
        const float bdry_d = RADIUS - ImSqrt(ImLengthSqr(state.mouse));

        state.step();

        result = state.get_status();
    }

    return result;
}

int
main(int argc, char* argv[])
{
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

    // The data
    static ImVec2 start_cat { RADIUS, 1.0f };
    static ImVec2 start_mouse ( -10.0f, 0.0f );
    static const float cat_speed = 10.0f;
    static const float cat_speed_radian = cat_speed / (2.0f * IM_PI * RADIUS);
    static State state { start_cat, start_mouse, cat_speed, cat_speed_radian };
    float size;
    static float scale;
    static ImVec2 origin;

    static Grid_colormap colormap;
    constexpr int n_samples = 48;
    colormap.init_colormap(color_cat, color_mouse, n_samples);

    std::deque<State> history;

    // The main drawing loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // The Control window
        if (show_control_window) {
            ImGui::SetNextWindowSize(control_window_size);
            ImGui::SetNextWindowPos(control_window_pos);
            ImGui::Begin("Controls", &show_control_window);

            auto margin_x = ImGui::GetStyle().WindowPadding.x;
            auto btnSize = ImVec2(ImGui::GetWindowWidth() / 2, 32.0f);
            if (ImGui::Button("Reset", btnSize)) {
                state = State(start_cat, start_mouse, cat_speed, cat_speed_radian);
            }
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

            static float mousex {};
            static float mousey {};
            static float mousex_range {};// = ImSqrt(RADIUS * RADIUS - mousey * mousey);
            static float mousey_range = ImSqrt(RADIUS * RADIUS - mousex * mousex);
            static float rcat_angle = ImAtan2(state.cat.y, state.cat.x);
            bool changed = false;
            if (ImGui::VSliderFloat("Set Mouse y", ImVec2(24.0f, 180.0f), &mousey, -mousey_range, mousey_range, "%.0f"), ImGuiSliderFlags_AlwaysClamp) {
                mousey_range = ImSqrt(RADIUS * RADIUS - mousey * mousey);
                changed = true;
            }
            if (ImGui::DragFloat("Set Mouse x", &mousex, 5.0f * scale, -RADIUS * scale, RADIUS * scale, "%.0f"), ImGuiSliderFlags_AlwaysClamp) {
                const double norm = std::sqrt(mousex*mousex + mousey*mousey);
                if (mousex*mousex+mousey*mousey > RADIUS*RADIUS) {
                    mousex_range = ImSqrt(RADIUS * RADIUS - mousex * mousex);
                }
                changed = true;
            }

            if (changed) {
                ImVec2 new_mouse { mousex, mousey };

                IM_ASSERT(ImLengthSqr(new_mouse) < RADIUS * RADIUS + 1.0f);

                const double dist = std::sqrt(ImLengthSqr(new_mouse - state.mouse));
                const int n_turn = std::ceil(dist / MOUSE_SPEED);

                state.mouse = new_mouse;
                double a = angle_between(state.cat, state.mouse);
                rcat_angle = ImMin(std::abs(a), (std::abs(a) / state.cat_speed_radian) * n_turn);
                if (a < 0.0) { rcat_angle *= -1; };
                state.cat = ImVec2(ImCos(rcat_angle), ImSin(rcat_angle)) * RADIUS;
            }

            if (ImGui::SliderAngle("Set Cat", &rcat_angle, -180.0f , 180.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat)) {
                state.reset(ImVec2(ImCos(rcat_angle), ImSin(rcat_angle)) * RADIUS, state.mouse);
                history.clear();
            }

            ImGui::Dummy(btnSize);
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginGroup();

            if (ImGui::Button("Undo", btnSize) && !history.empty()) {
                state = history.back();
                history.pop_back();
            }
            ImGui::SameLine();
            if (ImGui::Button("Step", btnSize)) {
                history.push_back(state);
                state.step();
            }
            ImGui::EndGroup();

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

            size = ImMin(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
            scale = ImMin(1.0f, 0.00075f * size); // So that Diameter is 75% of the screen
            origin = ImGui::GetCursorScreenPos() + ImVec2( size / 2.0f, size / 2.0f );// { ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() / 2,
                       // ImGui::GetCursorScreenPos().y + ImGui::GetWindowHeight() / 2 };

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            // Add the background pool
            // TODO: Color this to get an idea

            static ImU32 colors[n_samples]{ IM_COL32_WHITE };
            colormap.sample_point(state.mouse, cat_speed, colors);

            colormap.render_colored_disk(draw_list, colors, origin, scale);

            //draw_list->AddCircleFilled(origin, scale * RADIUS, color_water);

            IM_ASSERT(scale * 2 * RADIUS < (0.8 * ImGui::GetWindowHeight()));

            ImVec2 mouse_closest = state.mouse * (ImInvLength(state.mouse, 1.0f) * RADIUS) * scale;

            // mouse
            draw_list->AddCircleFilled(origin + state.mouse * scale, 10.0f, color_mouse, 12);

            // ojrigin
            draw_list->AddCircleFilled(origin, 10.0f, IM_COL32(0, 0, 0, 255), 12);

            /// cat
            draw_list->AddCircleFilled(origin + state.cat * scale, 10.0f, color_cat, 12);

            // The triangle joining cat/mouse/origin
            //draw_list->AddTriangle(origin, origin + state.mouse * scale, origin + state.cat * scale, state, 3.0f);
            //ImVec2 mt = origin + state.mouse_target_dir * RADIUS * scale;

            // Mouse target
            draw_list->AddCircleFilled(origin + (state.mouse + ImVec2(ImCos(state.mouse_dir), ImSin(state.mouse_dir))) * scale, IM_COL32(0, 0, 255, 125), 12);

            const float sz = (RADIUS / 5.0);
            const float width = (RADIUS / 25.0);
            DrawArrow(draw_list, origin, (state.mouse + state.mouse_dir_v * 0.2f) * scale, (state.mouse + state.mouse_dir_v * (0.2f + sz)) * scale, width * scale, IM_COL32(0, 0, 255, 125));
            // bool done = false;
            // while (not done) {
            //   Status status = state.sample();
            //   float result = status = Status::won ?
            //   get_color()
            //   draw_list->AddTriangle(origin, m * scale, c * scale, , 3.0f);
            // }

            ImGui::End();
        }

        // The logging window
        if (show_log_window) {
            ImGui::SetNextWindowSize(log_window_size, ImGuiCond_Once);
            ImGui::SetNextWindowPos(log_window_pos, ImGuiCond_Once);
            ImGui::Begin("Log", &show_log_window);

            static std::string str;
            str.clear();

            std::stringstream ss { str };

            ss << "GetCursorScreenPos().x " << ImGui::GetCursorScreenPos().x << " GetCursorScreenPos().y " << ImGui::GetCursorScreenPos().y << endl;

            ss << "GetWindowWidth() " << ImGui::GetWindowWidth() << " GetWindowHeight() " << ImGui::GetWindowHeight() << endl;

            ss << "Scale: " << scale << " size: " << size
                << "\norigin: " << origin.x << ' ' << origin.y
                << "\nmouse: " << state.mouse.x << ' ' << state.mouse.y
                << "\n mouse direction: " << 180.0f * state.mouse_dir / IM_PI << " degrees"
                << "\n mouse direction vector " << state.mouse_dir_v.x << ' ' << state.mouse_dir_v.y
                << "\ncat: " << state.cat.x << ' ' << state.cat.y
                << "\ncat target: " << state.cat_target.x << ' ' << state.cat_target.y
               << "\nAngle between cat and mouse: " << 180.0f * angle_between(state.cat, state.mouse) / IM_PI << " degrees" << endl;


            ss << "Translated mouse: " << (origin + state.mouse).x << ' ' << (origin + state.mouse).y
                << " Translated cat: " << (origin + state.cat).x << ' ' << (origin + state.cat).y << endl;

            str = ss.str();
            const char* s = &str[0];
            ImGui::Text(s);

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
