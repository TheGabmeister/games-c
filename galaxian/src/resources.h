#ifndef RESOURCES_H
#define RESOURCES_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>

/*
 * Resource Manager
 * ================
 * Centralized asset loading with automatic caching. Each asset is loaded once
 * and looked up by file path on subsequent calls, preventing duplicate loads
 * and simplifying cleanup.
 *
 * --- Usage ---
 *
 * Loading assets (returns cached version if already loaded):
 *
 *     SDL_Texture *tex  = res_load_texture("assets/player.png");
 *     TTF_Font    *font = res_load_font("assets/ui.ttf", 24);
 *     MIX_Audio   *sfx  = res_load_sound("assets/jump.wav");
 *     MIX_Audio   *bgm  = res_load_music("assets/bgm.mp3");
 *
 * All functions return NULL on failure and log the error via SDL_Log.
 *
 * --- Sound vs Music ---
 *
 * Both return MIX_Audio*, but:
 *   - res_load_sound() pre-decodes the entire file into memory (fast playback,
 *     good for short sound effects)
 *   - res_load_music() streams from disk (lower memory, good for long tracks)
 *
 * --- Fonts and sizes ---
 *
 * Fonts are keyed by (path + size), so loading the same .ttf at different
 * point sizes gives you separate cached entries:
 *
 *     TTF_Font *small = res_load_font("assets/ui.ttf", 16);
 *     TTF_Font *large = res_load_font("assets/ui.ttf", 32);  // different entry
 *
 * --- Cleanup ---
 *
 * Call res_free_all() once at shutdown to destroy every cached asset.
 * Do NOT manually free resources returned by res_load_* functions.
 *
 * For partial cleanup (e.g., between levels), call res_free_all() and
 * reload what you need. There is no single-asset unload by design — it
 * keeps the API simple and avoids dangling-pointer bugs.
 *
 * --- Where to put asset files ---
 *
 * Place assets relative to the executable. A typical layout:
 *
 *     build/Release/hello_sdl.exe
 *     build/Release/assets/
 *         player.png
 *         ui.ttf
 *         jump.wav
 *         bgm.mp3
 */

#define MAX_TEXTURES 128
#define MAX_FONTS    32
#define MAX_AUDIO    128

SDL_Texture *res_load_texture(const char *path);
TTF_Font    *res_load_font(const char *path, float pt_size);

/* Pre-decoded audio (sound effects) — requires audio_init() first */
MIX_Audio   *res_load_sound(const char *path);

/* Streamed audio (music) — requires audio_init() first */
MIX_Audio   *res_load_music(const char *path);

/* Destroy all cached assets. Call once at shutdown. */
void res_free_all(void);

#endif /* RESOURCES_H */
