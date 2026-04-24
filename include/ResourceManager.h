#pragma once

#include "raylib-cpp.hpp"

constexpr const char* BACKGROUND_PATH = "assets/background.png";
constexpr const char* CURSOR_ATLAS_PATH = "assets/cursor_atlas.png";
constexpr const char* BGM_PATH = "assets/background.mp3";
constexpr const char* GRAB_SFX_PATH = "assets/grab.mp3";
constexpr const char* HIT_SFX_PATH = "assets/hit.mp3";
constexpr const char* FAIL_SFX_PATH = "assets/fail.mp3";
constexpr const char* SUCCESS_SFX_PATH = "assets/success.mp3";
constexpr const char* ENTER_SFX_PATH = "assets/enter.mp3";
constexpr const char* INVERT_SHADER_PATH = "assets/invert.frag";
constexpr const char* SPECTRUM_SHADER_PATH = "assets/spectrum.frag";
constexpr const char* FONT_PATH = "assets/aesi.ttf";
constexpr float MUSIC_VOLUME = 0.3f;
constexpr float SFX_VOLUME = 2.0f;

struct ResourceManager {
    // Shared textures
    raylib::Texture background;
    raylib::Texture cursorAtlas;

    // Audio
    raylib::Music bgm;
    raylib::Sound grabSfx;
    raylib::Sound hitSfx;
    raylib::Sound failSfx;
    raylib::Sound successSfx;
    raylib::Sound enterSfx;

    // Shaders
    raylib::Shader invertShader;
    raylib::Shader spectrumShader;
    int spectrumTimeLoc;

    // Fonts
    raylib::Font font;

    ResourceManager() :
        background(BACKGROUND_PATH), cursorAtlas(CURSOR_ATLAS_PATH), bgm(BGM_PATH), grabSfx(GRAB_SFX_PATH),
        hitSfx(HIT_SFX_PATH), failSfx(FAIL_SFX_PATH), successSfx(SUCCESS_SFX_PATH), enterSfx(ENTER_SFX_PATH),
        invertShader(nullptr, INVERT_SHADER_PATH), spectrumShader(nullptr, SPECTRUM_SHADER_PATH), font(FONT_PATH) {
        bgm.SetVolume(MUSIC_VOLUME);
        bgm.Play();
        grabSfx.SetVolume(SFX_VOLUME);
        hitSfx.SetVolume(SFX_VOLUME);
        failSfx.SetVolume(SFX_VOLUME);
        successSfx.SetVolume(SFX_VOLUME);
        enterSfx.SetVolume(SFX_VOLUME);
        spectrumTimeLoc = GetShaderLocation(spectrumShader, "time");
    }

    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
};
