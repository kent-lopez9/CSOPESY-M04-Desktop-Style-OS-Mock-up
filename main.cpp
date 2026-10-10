#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#pragma comment(lib, "opengl32.lib")
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

enum class Stage { BOOT, SPLASH, DESKTOP };

static const ImGuiWindowFlags kLayerFlags =
ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground;

// begins witha borderless window 
static bool BeginLayer(const char* id, ImVec2 pos, ImVec2 size)
{
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    bool open = ImGui::Begin(id, nullptr, kLayerFlags);
    ImGui::PopStyleVar(2);
    return open;
}

static void FormatClock(char* out, size_t n)
{
    time_t t = time(nullptr);
    tm lt = {};
    localtime_s(&lt, &t);
    strftime(out, n, "%A, %b %d, %Y | %I:%M:%S %p", &lt);
}

//wallpaper using ImGui draw commands
static void DrawWallpaper(ImDrawList* dl, ImVec2 p, ImVec2 s, float t)
{
    ImU32 skyTop = IM_COL32(35, 105, 215, 255);
    ImU32 skyLow = IM_COL32(160, 210, 255, 255);
    dl->AddRectFilledMultiColor(p, p + s, skyTop, skyTop, skyLow, skyLow);

    dl->PushClipRect(p, p + s, true);

    // Drifting clouds
    for (int i = 0; i < 6; i++)
    {
        float span = s.x + 400.0f;
        float x = fmodf(t * (10.0f + i * 3.0f) + i * span / 6.0f, span) - 200.0f;
        float y = s.y * (0.10f + 0.07f * (i % 4));
        float r = s.y * (0.035f + 0.008f * (i % 3));
        ImU32 c = IM_COL32(255, 255, 255, 215);
        dl->AddCircleFilled(ImVec2(p.x + x, p.y + y), r, c, 32);
        dl->AddCircleFilled(ImVec2(p.x + x + r * 1.1f, p.y + y - r * 0.5f), r * 1.2f, c, 32);
        dl->AddCircleFilled(ImVec2(p.x + x + r * 2.3f, p.y + y), r * 0.95f, c, 32);
        dl->AddCircleFilled(ImVec2(p.x + x + r * 1.1f, p.y + y + r * 0.3f), r * 1.1f, c, 32);
    }

    // Hills
    dl->AddCircleFilled(ImVec2(p.x + s.x * 0.95f, p.y + s.y * 1.80f), s.y * 1.05f, IM_COL32(50, 135, 35, 255), 160);
    dl->AddCircleFilled(ImVec2(p.x + s.x * 0.30f, p.y + s.y * 1.95f), s.y * 1.28f, IM_COL32(85, 175, 45, 255), 160);

    dl->PopClipRect();
}

// BIOS
static void DrawBootScreen(ImVec2 vp, double elapsed)
{
    static const char* lines[] = {
        "CSOPESY Megatrends",
        "Released: 12/01/94",
        "DLSU GAME Lab (C)1994 CSOPESY Megatrends Inc.",
        "",
        "Memory Test:",
        "Checking RAM : 64000K OK",
        "",
        "CPU Type: Pentium III",
        "BIOS Version: BCN SIT 1989-1994 Special CSOPESY",
        "Main Processor: Pentium III",
        "Numeric Processor: Built-in",
        "",
        "Primary Master:   420 MB WDCAC2420H",
        "Primary Slave:    None",
        "Secondary Master: CD-ROM LTN-305A",
        "Secondary Slave:  None",
    };
    const int count = (int)(sizeof(lines) / sizeof(lines[0]));
    int visible = (int)(elapsed / 0.18);
    if (visible > count) visible = count;

    if (BeginLayer("##boot", ImVec2(0, 0), vp))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(ImVec2(0, 0), vp, IM_COL32(0, 0, 0, 255));
        float lh = ImGui::GetTextLineHeightWithSpacing();
        for (int i = 0; i < visible; i++)
            dl->AddText(ImVec2(20, 20 + i * lh), IM_COL32(220, 220, 220, 255), lines[i]);
    }
    ImGui::End();
}

static void DrawSplashScreen(ImVec2 vp, double elapsed)
{
    if (BeginLayer("##splash", ImVec2(0, 0), vp))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(ImVec2(0, 0), vp, IM_COL32(0, 0, 0, 255));

        const char* title = "C S O P E S Y";
        const char* lines[] = {
            "CSOPESY Operating System Emulator v1.0",
            "Part of Project Anito:",
            "Support for the PH Gaming Industry Towards Embracing",
            "In-House Game Engine Development",
        };
        float cx = vp.x * 0.5f, y = vp.y * 0.32f;
        float lh = ImGui::GetTextLineHeightWithSpacing();

        ImGui::PushFont(nullptr, ImGui::GetFontSize() * 2.5f);
        ImVec2 ts = ImGui::CalcTextSize(title);
        dl->AddText(ImVec2(cx - ts.x * 0.5f, y), IM_COL32(60, 190, 230, 255), title);
        ImGui::PopFont();
        y += ts.y + lh;

        for (const char* l : lines)
        {
            ImVec2 sz = ImGui::CalcTextSize(l);
            dl->AddText(ImVec2(cx - sz.x * 0.5f, y), IM_COL32(220, 220, 220, 255), l);
            y += lh;
        }

        int dots = 1 + (int)(elapsed * 2.0) % 3;
        char loading[16];
        snprintf(loading, sizeof(loading), "Loading%.*s", dots, "...");
        ImVec2 ls = ImGui::CalcTextSize(loading);
        dl->AddText(ImVec2(cx - ls.x * 0.5f, y + lh * 2), IM_COL32(80, 255, 80, 255), loading);
    }
    ImGui::End();
}

// Desktop layer with the wallpaper, clock, and system label
static void DrawDesktop(ImVec2 vp, float barH)
{
    // Desktop stops where the taskbar starts. ImGui puts NoBringToFrontOnFocus windows
    // at the very back, so a full-screen desktop would sit on top of the taskbar and eat its clicks.
    if (BeginLayer("##desktop", ImVec2(0, 0), ImVec2(vp.x, vp.y - barH)))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        DrawWallpaper(dl, ImVec2(0, 0), ImVec2(vp.x, vp.y - barH), (float)ImGui::GetTime());

        // Real-time clock, fixed top-right corner (updates every frame)
        char clock[64];
        FormatClock(clock, sizeof(clock));
        ImVec2 cs = ImGui::CalcTextSize(clock);
        float pad = ImGui::GetFontSize() * 0.6f;
        ImVec2 cmin(vp.x - cs.x - pad * 3.0f, pad);
        ImVec2 cmax(vp.x - pad, pad + cs.y + pad);
        dl->AddRectFilled(cmin, cmax, IM_COL32(15, 15, 20, 220), 4.0f);
        dl->AddText(ImVec2(cmin.x + pad, cmin.y + pad * 0.5f), IM_COL32(240, 240, 240, 255), clock);

        // System label
        dl->AddText(ImVec2(pad, vp.y - barH - cs.y - pad), IM_COL32(60, 255, 90, 255), "CSOPESY OS v1.0 - System Online");
    }
    ImGui::End();
}

// Task Manager with dummy values that drift once per second
struct Proc { const char* name; int pid; float cpu; float mem; float cpuBase; float memBase; };

static void DrawTaskManager(bool* open, float scale)
{
    static Proc procs[] = {
        { "System",              4,    1.0f,  120.0f,  1.0f, 120.0f },
        { "csopesy.exe",         5120, 12.0f, 310.0f, 12.0f, 310.0f },
        { "explorer.exe",        2208, 2.5f,  180.0f,  2.5f, 180.0f },
        { "Chrome",              7340, 18.0f, 960.0f, 18.0f, 960.0f },
        { "Spotify",             6012, 3.0f,  240.0f,  3.0f, 240.0f },
        { "Discord",             6844, 5.0f,  420.0f,  5.0f, 420.0f },
        { "Visual Studio 2022",  9120, 9.0f,  780.0f,  9.0f, 780.0f },
        { "Task Manager",        3301, 1.5f,  75.0f,   1.5f, 75.0f  },
        { "Desktop Window Mgr",  1180, 2.0f,  95.0f,   2.0f, 95.0f  },
    };
    static const int kCount = (int)(sizeof(procs) / sizeof(procs[0]));
    static float history[60] = {};
    static double lastUpdate = -1.0;
    const float totalRamMB = 16384.0f;

    double now = ImGui::GetTime();
    if (lastUpdate < 0.0 || now - lastUpdate >= 1.0)
    {
        lastUpdate = now;
        float total = 0.0f;
        for (int i = 0; i < kCount; i++)
        {
            Proc& p = procs[i];
            p.cpu = p.cpuBase + ((rand() % 200) - 100) / 100.0f * (p.cpuBase * 0.5f + 0.5f);
            if (p.cpu < 0.0f) p.cpu = 0.0f;
            p.mem = p.memBase + ((rand() % 200) - 100) / 100.0f * (p.memBase * 0.03f);
            total += p.cpu;
        }
        if (total > 100.0f) total = 100.0f;
        memmove(history, history + 1, sizeof(float) * 59);
        history[59] = total;
    }

    ImGui::SetNextWindowPos(ImVec2(160 * scale, 90 * scale), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(560 * scale, 420 * scale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Task Manager", open))
    {
        float totalCpu = history[59];
        float totalMem = 0.0f;
        for (int i = 0; i < kCount; i++) totalMem += procs[i].mem;

        if (ImGui::BeginTabBar("##tmtabs"))
        {
            if (ImGui::BeginTabItem("Processes"))
            {
                ImGui::Text("CPU: %.0f%%    Memory: %.0f / %.0f MB (%.0f%%)", totalCpu, totalMem, totalRamMB, totalMem / totalRamMB * 100.0f);
                ImGui::Separator();

                ImGuiTableFlags tf = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;
                if (ImGui::BeginTable("##procs", 4, tf, ImVec2(0, 0)))
                {
                    ImGui::TableSetupScrollFreeze(0, 1);
                    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 70 * scale);
                    ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthFixed, 80 * scale);
                    ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 100 * scale);
                    ImGui::TableHeadersRow();
                    for (int i = 0; i < kCount; i++)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(procs[i].name);
                        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", procs[i].pid);
                        ImGui::TableSetColumnIndex(2); ImGui::Text("%.1f%%", procs[i].cpu);
                        ImGui::TableSetColumnIndex(3); ImGui::Text("%.0f MB", procs[i].mem);
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Performance"))
            {
                ImGui::Text("CPU Utilization (last 60s)");
                ImGui::PlotLines("##cpu", history, 60, 0, nullptr, 0.0f, 100.0f, ImVec2(-1, 120 * scale));
                ImGui::Spacing();
                ImGui::Text("Memory in use");
                ImGui::ProgressBar(totalMem / totalRamMB, ImVec2(-1, 0));
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

// fake file explorer
static void DrawFilesWindow(bool* open, float scale)
{
    ImGui::SetNextWindowPos(ImVec2(80 * scale, 60 * scale), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620 * scale, 380 * scale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Files", open))
    {
        static int selected = 0;
        static const char* folders[] = { "Desktop", "Documents", "Downloads", "Pictures", "Music", "Games" };
        static const char* contents[][3] = {
            { "wallpaper.png", "shortcut.lnk", "notes.txt" },
            { "csopesy_report.pptx", "readme.txt", "schedule.xlsx" },
            { "setup.exe", "imgui.zip", "glfw.zip" },
            { "cat.jpg", "bliss.png", "screenshot_01.png" },
            { "track01.mp3", "track02.mp3", "playlist.m3u" },
            { "ddr_supernova.iso", "savegame.dat", "config.ini" },
        };
        static const char* sizes[][3] = {
            { "2.4 MB", "1 KB", "3 KB" }, { "5.1 MB", "2 KB", "48 KB" }, { "12 MB", "3.2 MB", "1.1 MB" },
            { "1.8 MB", "2.2 MB", "900 KB" }, { "4.5 MB", "5.0 MB", "1 KB" }, { "620 MB", "14 KB", "2 KB" },
        };

        ImGui::BeginChild("##tree", ImVec2(160 * scale, 0), ImGuiChildFlags_Borders);
        if (ImGui::TreeNodeEx("This PC", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (int i = 0; i < 6; i++)
                if (ImGui::Selectable(folders[i], selected == i)) selected = i;
            ImGui::TreePop();
        }
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("This PC > %s", folders[selected]);
        ImGui::Separator();
        if (ImGui::BeginTable("##files", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 90 * scale);
            ImGui::TableHeadersRow();
            for (int i = 0; i < 3; i++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(contents[selected][i]);
                ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sizes[selected][i]);
            }
            ImGui::EndTable();
        }
        ImGui::EndGroup();
    }
    ImGui::End();
}

// notepad 
static void DrawNotesWindow(bool* open, float scale)
{
    static char text[4096] = "Welcome to CSOPESY OS!\n\nType your notes here...";
    ImGui::SetNextWindowPos(ImVec2(260 * scale, 120 * scale), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(460 * scale, 320 * scale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Notes", open, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) text[0] = '\0';
                ImGui::MenuItem("Save (placeholder)");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::MenuItem("Undo (placeholder)");
                ImGui::MenuItem("Find (placeholder)");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::InputTextMultiline("##notes", text, sizeof(text), ImVec2(-1, -ImGui::GetFrameHeightWithSpacing()));
        ImGui::Text("%d characters", (int)strlen(text));
    }
    ImGui::End();
}

// calculator
struct CalcState
{
    double acc = 0.0; // running result
    char op = 0; // pending operator: + - * / (0 = none)
    char cur[32] = "0"; // text currently shown on the display
    char hist[64] = ""; // small line above the display
    bool fresh = true; // next digit starts a new number
    bool error = false; // divide by zero; only C works until cleared
};

static bool CalcApply(CalcState& c, double rhs)
{
    switch (c.op)
    {
    case '+': c.acc += rhs; break;
    case '-': c.acc -= rhs; break;
    case '*': c.acc *= rhs; break;
    case '/': if (rhs == 0.0) return false; c.acc /= rhs; break;
    default:  c.acc = rhs; break;
    }
    return true;
}

static void CalcPress(CalcState& c, char k)
{
    if (k == 'C') { c = CalcState(); return; }
    if (c.error) return;

    size_t len = strlen(c.cur);
    if (k >= '0' && k <= '9')
    {
        if (c.fresh || strcmp(c.cur, "0") == 0) { c.cur[0] = k; c.cur[1] = '\0'; c.fresh = false; }
        else if (len < 15) { c.cur[len] = k; c.cur[len + 1] = '\0'; }
    }
    else if (k == '.')
    {
        if (c.fresh) { strcpy_s(c.cur, "0."); c.fresh = false; }
        else if (!strchr(c.cur, '.') && len < 15) { c.cur[len] = '.'; c.cur[len + 1] = '\0'; }
    }
    else if (k == 'N') // negate
    {
        if (strcmp(c.cur, "0") != 0)
        {
            if (c.cur[0] == '-') memmove(c.cur, c.cur + 1, len);
            else { memmove(c.cur + 1, c.cur, len + 1); c.cur[0] = '-'; }
        }
    }
    else if (k == 'B') // backspace
    {
        if (!c.fresh)
        {
            if (len > 1) { c.cur[len - 1] = '\0'; if (strcmp(c.cur, "-") == 0) strcpy_s(c.cur, "0"); }
            else strcpy_s(c.cur, "0");
        }
    }
    else // operator or '='
    {
        double v = atof(c.cur);
        if (!c.op) c.acc = v;
        else if (!c.fresh)
        {
            if (!CalcApply(c, v))
            {
                c = CalcState();
                strcpy_s(c.cur, "Error");
                c.error = true;
                return;
            }
        }
        snprintf(c.cur, sizeof(c.cur), "%.10g", c.acc);
        c.fresh = true;
        if (k == '=') { c.op = 0; c.hist[0] = '\0'; }
        else { c.op = k; snprintf(c.hist, sizeof(c.hist), "%.10g %c", c.acc, k); }
    }
}

static void DrawCalculatorWindow(bool* open, float scale)
{
    static CalcState calc;
    ImGui::SetNextWindowPos(ImVec2(340 * scale, 80 * scale), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280 * scale, 390 * scale), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Calculator", open))
    {
        // Display
        ImGui::BeginChild("##calcdisp", ImVec2(0, 72 * scale), ImGuiChildFlags_Borders);
        float w = ImGui::GetContentRegionAvail().x;
        ImVec2 hs = ImGui::CalcTextSize(calc.hist);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (w > hs.x ? w - hs.x : 0.0f));
        ImGui::TextDisabled("%s", calc.hist);
        ImGui::PushFont(nullptr, ImGui::GetFontSize() * 1.8f);
        ImVec2 ts = ImGui::CalcTextSize(calc.cur);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (w > ts.x ? w - ts.x : 0.0f));
        ImGui::TextUnformatted(calc.cur);
        ImGui::PopFont();
        ImGui::EndChild();

        // Keypad (4 columns)
        struct Key { const char* label; char key; int span; };
        static const Key keys[] = {
            { "C", 'C', 1 }, { "+/-", 'N', 1 }, { "<-", 'B', 1 }, { "/", '/', 1 },
            { "7", '7', 1 }, { "8", '8', 1 },   { "9", '9', 1 },  { "*", '*', 1 },
            { "4", '4', 1 }, { "5", '5', 1 },   { "6", '6', 1 },  { "-", '-', 1 },
            { "1", '1', 1 }, { "2", '2', 1 },   { "3", '3', 1 },  { "+", '+', 1 },
            { "0", '0', 2 }, { ".", '.', 1 },   { "=", '=', 1 },
        };
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float unit = (ImGui::GetContentRegionAvail().x - spacing * 3.0f) / 4.0f;
        float h = 44.0f * scale;
        int col = 0;
        for (const Key& k : keys)
        {
            if (col > 0) ImGui::SameLine();
            bool isOp = (k.key == '/' || k.key == '*' || k.key == '-' || k.key == '+' || k.key == '=');
            bool isClear = (k.key == 'C');
            if (isOp)         ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.45f, 0.10f, 1.0f));
            else if (isClear) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.70f, 0.20f, 0.20f, 1.0f));
            if (ImGui::Button(k.label, ImVec2(unit * k.span + spacing * (k.span - 1), h)))
                CalcPress(calc, k.key);
            if (isOp || isClear) ImGui::PopStyleColor();
            col += k.span;
            if (col >= 4) col = 0;
        }
    }
    ImGui::End();
}

// Taskbar
// Returns true if PWR was pressed
static bool TaskbarButton(const char* label, ImVec4 textCol, bool active, ImVec2 size)
{
    ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(0.28f, 0.33f, 0.50f, 1.0f) : ImVec4(0.14f, 0.15f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.36f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.20f, 0.25f, 0.40f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, textCol);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, size.y * 0.5f);
    bool clicked = ImGui::Button(label, size);
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
    return clicked;
}

static bool DrawTaskbar(ImVec2 vp, float barH, bool& showFiles, bool& showNotes, bool& showTask, bool& showCalc)
{
    bool power = false;
    ImVec2 pos(0, vp.y - barH);
    if (BeginLayer("##taskbar", pos, ImVec2(vp.x, barH)))
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, ImVec2(vp.x, vp.y), IM_COL32(18, 20, 30, 245));
        dl->AddLine(pos, ImVec2(vp.x, pos.y), IM_COL32(70, 80, 110, 255), 1.0f);

        float pad = barH * 0.15f;
        ImVec2 bsz(barH * 1.5f, barH - pad * 2.0f);

        ImGui::SetCursorPos(ImVec2(pad, pad));
        if (TaskbarButton("FILES", ImVec4(1.0f, 0.85f, 0.3f, 1.0f), showFiles, bsz)) showFiles = !showFiles;
        ImGui::SameLine();
        if (TaskbarButton("NOTES", ImVec4(0.4f, 1.0f, 0.5f, 1.0f), showNotes, bsz)) showNotes = !showNotes;
        ImGui::SameLine();
        if (TaskbarButton("TASK", ImVec4(0.4f, 0.8f, 1.0f, 1.0f), showTask, bsz)) showTask = !showTask;
        ImGui::SameLine();
        if (TaskbarButton("CALC", ImVec4(1.0f, 0.6f, 0.2f, 1.0f), showCalc, bsz)) showCalc = !showCalc;

        ImGui::SetCursorPos(ImVec2(vp.x - pad - bsz.x * 0.8f, pad));
        if (TaskbarButton("PWR", ImVec4(1.0f, 0.3f, 0.3f, 1.0f), false, ImVec2(bsz.x * 0.8f, bsz.y))) power = true;
    }
    ImGui::End();
    return power;
}

int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(720 * main_scale), "CSOPESY Desktop OS Emulator", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwMaximizeWindow(window); // desktop fills the whole screen

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // windows start fresh every run

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    style.WindowRounding = 6.0f * main_scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // OS state
    Stage stage = Stage::BOOT;
    double stageStart = glfwGetTime();
    bool showFiles = false, showNotes = false, showTask = false, showCalc = false;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImVec2 vp = io.DisplaySize;
        double elapsed = glfwGetTime() - stageStart;

        if (stage == Stage::BOOT)
        {
            DrawBootScreen(vp, elapsed);
            if (elapsed > 4.5) { stage = Stage::SPLASH; stageStart = glfwGetTime(); }
        }
        else if (stage == Stage::SPLASH)
        {
            DrawSplashScreen(vp, elapsed);
            if (elapsed > 3.0) { stage = Stage::DESKTOP; stageStart = glfwGetTime(); }
        }
        else
        {
            float barH = ImGui::GetFontSize() * 3.0f;

            // 1. Desktop first (base layer)
            DrawDesktop(vp, barH);

            // 2. Application windows
            if (showFiles) DrawFilesWindow(&showFiles, main_scale);
            if (showNotes) DrawNotesWindow(&showNotes, main_scale);
            if (showTask)  DrawTaskManager(&showTask, main_scale);
            if (showCalc)  DrawCalculatorWindow(&showCalc, main_scale);

            // 3. Taskbar; PWR is the only way to shut down
            if (DrawTaskbar(vp, barH, showFiles, showNotes, showTask, showCalc))
                glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}