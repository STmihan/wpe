#include <windows.h>
#include <fstream>
#include <SDL3/SDL.h>

HWND progman = nullptr;
std::ofstream out("result.txt");

HWND findWorker() {
    progman = FindWindow("Progman", nullptr);
    if (progman == nullptr) {
        MessageBox(nullptr, "Could not find Progman window", "Error", MB_OK | MB_ICONERROR);
    }
    SendMessageTimeout(progman, 0x52C, 0, 0, SMTO_ABORTIFHUNG, 1000, nullptr);

    HWND worker = nullptr;
    EnumChildWindows(progman, [](HWND hwnd, LPARAM lparam) {
        HWND shell = FindWindowEx(progman, nullptr, "SHELLDLL_DefView", nullptr);

        if (shell != nullptr) {
            *reinterpret_cast<HWND *>(lparam) = FindWindowEx(progman, hwnd, "WorkerW", nullptr);
            return FALSE;
        }

        return 1;
    }, reinterpret_cast<LPARAM>(&worker));

    return worker;
}


int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
    HWND worker = findWorker();
    if (worker == nullptr) {
        return MessageBox(nullptr, "Could not find WorkerW window", "Error", MB_OK | MB_ICONERROR);
    }

    RECT rc{};
    GetClientRect(worker, &rc);
    const int deskW = rc.right - rc.left;
    const int deskH = rc.bottom - rc.top;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_CreateWindowAndRenderer(
        "wpe",
        deskW,
        deskH,
        SDL_WINDOW_BORDERLESS,
        &window,
        &renderer
    );

    HWND hwnd = (HWND) SDL_GetPointerProperty(
        SDL_GetWindowProperties(window),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER,
        nullptr
    );
    SetParent(hwnd, worker);
    SetWindowPos(
        hwnd, nullptr,
        0, 0,
        deskW, deskH,
        SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}
