#include <SDL3/SDL.h>

#include <glm/vec2.hpp>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <cstdio>

int main(int, char**) {
    if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    constexpr SDL_WindowFlags window_flags =
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_HIDDEN |
        SDL_WINDOW_HIGH_PIXEL_DENSITY;

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 + Dear ImGui + GLM",
        1280,
        720,
        window_flags
    );
    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        std::fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    if (not ImGui_ImplSDL3_InitForSDLRenderer(window, renderer)) {
        std::fprintf(stderr, "ImGui SDL3 platform backend initialization failed\n");
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (not ImGui_ImplSDLRenderer3_Init(renderer)) {
        std::fprintf(stderr, "ImGui SDL_Renderer backend initialization failed\n");
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    glm::vec2 value{0.0f, 0.0f};
    bool running = true;

    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (
                event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED and
                event.window.windowID == SDL_GetWindowID(window)
            ) {
                running = false;
            }
        }

        if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) != 0) {
            SDL_Delay(10);
            continue;
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(360.0f, 170.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("glm::vec2 editor");
        ImGui::TextUnformatted("Edit the components of a glm::vec2:");
        ImGui::SliderFloat("X", &value.x, -10.0f, 10.0f);
        ImGui::SliderFloat("Y", &value.y, -10.0f, 10.0f);
        ImGui::Separator();
        ImGui::Text("value = (%.3f, %.3f)", value.x, value.y);
        ImGui::End();

        ImGui::Render();

        SDL_SetRenderScale(
            renderer,
            io.DisplayFramebufferScale.x,
            io.DisplayFramebufferScale.y
        );
        SDL_SetRenderDrawColor(renderer, 26, 29, 33, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
