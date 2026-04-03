#include "resources.h"
#include "platform.h"
#include "audio.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>
#include <string.h>

static bool resources_ready = false;
static char default_font_path[512];

static bool file_exists(const char *path)
{
    SDL_IOStream *io;

    if (!path || !path[0]) {
        return false;
    }

    io = SDL_IOFromFile(path, "rb");
    if (!io) {
        return false;
    }

    SDL_CloseIO(io);
    return true;
}

static bool set_default_font_path(const char *path)
{
    if (!file_exists(path)) {
        return false;
    }

    SDL_strlcpy(default_font_path, path, sizeof(default_font_path));
    return true;
}

static bool resolve_default_font_path(void)
{
    static const char *relative_candidates[] = {
        "assets/fonts/galaxian.ttf",
        "assets/fonts/PressStart2P-Regular.ttf",
        "assets/galaxian.ttf",
    };
    static const char *system_candidates[] = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/lucon.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        "/System/Library/Fonts/SFNSMono.ttf",
    };
    const char *base_path;
    size_t i;

    if (default_font_path[0]) {
        return true;
    }

    base_path = SDL_GetBasePath();
    if (base_path) {
        char candidate[512];

        for (i = 0; i < SDL_arraysize(relative_candidates); i++) {
            SDL_snprintf(candidate, sizeof(candidate), "%s%s",
                         base_path, relative_candidates[i]);
            if (set_default_font_path(candidate)) {
                SDL_free((void *)base_path);
                return true;
            }
        }

        SDL_free((void *)base_path);
    }

    for (i = 0; i < SDL_arraysize(relative_candidates); i++) {
        if (set_default_font_path(relative_candidates[i])) {
            return true;
        }
    }

    for (i = 0; i < SDL_arraysize(system_candidates); i++) {
        if (set_default_font_path(system_candidates[i])) {
            return true;
        }
    }

    default_font_path[0] = '\0';
    SDL_Log("res_default_font_path: no bundled or fallback font was found");
    return false;
}

bool res_init(void)
{
    if (resources_ready) {
        return true;
    }

    if (!TTF_Init()) {
        SDL_Log("res_init: TTF_Init failed: %s", SDL_GetError());
        return false;
    }

    resources_ready = true;
    resolve_default_font_path();
    return true;
}

void res_shutdown(void)
{
    if (!resources_ready) {
        return;
    }

    resources_ready = false;
    default_font_path[0] = '\0';
}

const char *res_default_font_path(void)
{
    if (!resources_ready) {
        SDL_Log("res_default_font_path: resources not initialized (call res_init first)");
        return NULL;
    }

    if (!resolve_default_font_path()) {
        return NULL;
    }

    return default_font_path;
}

/* --- Texture cache --- */

typedef struct {
    char path[256];
    SDL_Texture *texture;
} TextureEntry;

static TextureEntry textures[MAX_TEXTURES];
static int texture_count = 0;

SDL_Texture *res_load_texture(const char *path)
{
    for (int i = 0; i < texture_count; i++) {
        if (strcmp(textures[i].path, path) == 0)
            return textures[i].texture;
    }

    if (texture_count >= MAX_TEXTURES) {
        SDL_Log("res_load_texture: cache full (%d)", MAX_TEXTURES);
        return NULL;
    }

    SDL_Texture *tex = IMG_LoadTexture(get_renderer(), path);
    if (!tex) {
        SDL_Log("res_load_texture: failed to load '%s': %s", path, SDL_GetError());
        return NULL;
    }

    TextureEntry *e = &textures[texture_count++];
    strncpy(e->path, path, sizeof(e->path) - 1);
    e->path[sizeof(e->path) - 1] = '\0';
    e->texture = tex;
    return tex;
}

/* --- Font cache --- */

typedef struct {
    char path[256];
    float pt_size;
    TTF_Font *font;
} FontEntry;

static FontEntry fonts[MAX_FONTS];
static int font_count = 0;

TTF_Font *res_load_font(const char *path, float pt_size)
{
    if (!resources_ready) {
        SDL_Log("res_load_font: resources not initialized (call res_init first)");
        return NULL;
    }

    for (int i = 0; i < font_count; i++) {
        if (fonts[i].pt_size == pt_size && strcmp(fonts[i].path, path) == 0)
            return fonts[i].font;
    }

    if (font_count >= MAX_FONTS) {
        SDL_Log("res_load_font: cache full (%d)", MAX_FONTS);
        return NULL;
    }

    TTF_Font *font = TTF_OpenFont(path, pt_size);
    if (!font) {
        SDL_Log("res_load_font: failed to load '%s' at %.1fpt: %s", path, pt_size, SDL_GetError());
        return NULL;
    }

    FontEntry *e = &fonts[font_count++];
    strncpy(e->path, path, sizeof(e->path) - 1);
    e->path[sizeof(e->path) - 1] = '\0';
    e->pt_size = pt_size;
    e->font = font;
    return font;
}

/* --- Audio cache (shared for sounds and music) --- */

typedef struct {
    char path[256];
    bool predecoded;
    MIX_Audio *audio;
} AudioEntry;

static AudioEntry audio_entries[MAX_AUDIO];
static int audio_count = 0;

static MIX_Audio *load_audio_cached(const char *path, bool predecode)
{
    for (int i = 0; i < audio_count; i++) {
        if (audio_entries[i].predecoded == predecode &&
            strcmp(audio_entries[i].path, path) == 0) {
            return audio_entries[i].audio;
        }
    }

    if (audio_count >= MAX_AUDIO) {
        SDL_Log("res_load_audio: cache full (%d)", MAX_AUDIO);
        return NULL;
    }

    MIX_Mixer *mixer = get_mixer();
    if (!mixer) {
        SDL_Log("res_load_audio: audio not initialized (call audio_init first)");
        return NULL;
    }

    MIX_Audio *audio = MIX_LoadAudio(mixer, path, predecode);
    if (!audio) {
        SDL_Log("res_load_audio: failed to load '%s': %s", path, SDL_GetError());
        return NULL;
    }

    AudioEntry *e = &audio_entries[audio_count++];
    strncpy(e->path, path, sizeof(e->path) - 1);
    e->path[sizeof(e->path) - 1] = '\0';
    e->predecoded = predecode;
    e->audio = audio;
    return audio;
}

MIX_Audio *res_load_sound(const char *path)
{
    return load_audio_cached(path, true);
}

MIX_Audio *res_load_music(const char *path)
{
    return load_audio_cached(path, false);
}

/* --- Cleanup --- */

void res_free_all(void)
{
    for (int i = 0; i < texture_count; i++)
        SDL_DestroyTexture(textures[i].texture);
    texture_count = 0;

    for (int i = 0; i < font_count; i++)
        TTF_CloseFont(fonts[i].font);
    font_count = 0;

    for (int i = 0; i < audio_count; i++)
        MIX_DestroyAudio(audio_entries[i].audio);
    audio_count = 0;
}
