#include "resources.h"
#include "platform.h"
#include <string.h>

/* --- Texture cache --- */

typedef struct {
    char path[256];
    SDL_Texture *texture;
} TextureEntry;

static TextureEntry textures[MAX_TEXTURES];
static int texture_count = 0;

SDL_Texture *res_load_texture(const char *path)
{
    /* Return cached */
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
    /* Return cached (same path + same size) */
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

/* --- Sound cache --- */

typedef struct {
    char path[256];
    Mix_Chunk *chunk;
} SoundEntry;

static SoundEntry sounds[MAX_SOUNDS];
static int sound_count = 0;

Mix_Chunk *res_load_sound(const char *path)
{
    for (int i = 0; i < sound_count; i++) {
        if (strcmp(sounds[i].path, path) == 0)
            return sounds[i].chunk;
    }

    if (sound_count >= MAX_SOUNDS) {
        SDL_Log("res_load_sound: cache full (%d)", MAX_SOUNDS);
        return NULL;
    }

    Mix_Chunk *chunk = Mix_LoadWAV(path);
    if (!chunk) {
        SDL_Log("res_load_sound: failed to load '%s': %s", path, SDL_GetError());
        return NULL;
    }

    SoundEntry *e = &sounds[sound_count++];
    strncpy(e->path, path, sizeof(e->path) - 1);
    e->path[sizeof(e->path) - 1] = '\0';
    e->chunk = chunk;
    return chunk;
}

/* --- Music cache --- */

typedef struct {
    char path[256];
    Mix_Music *music;
} MusicEntry;

static MusicEntry musics[MAX_MUSIC];
static int music_count = 0;

Mix_Music *res_load_music(const char *path)
{
    for (int i = 0; i < music_count; i++) {
        if (strcmp(musics[i].path, path) == 0)
            return musics[i].music;
    }

    if (music_count >= MAX_MUSIC) {
        SDL_Log("res_load_music: cache full (%d)", MAX_MUSIC);
        return NULL;
    }

    Mix_Music *music = Mix_LoadMUS(path);
    if (!music) {
        SDL_Log("res_load_music: failed to load '%s': %s", path, SDL_GetError());
        return NULL;
    }

    MusicEntry *e = &musics[music_count++];
    strncpy(e->path, path, sizeof(e->path) - 1);
    e->path[sizeof(e->path) - 1] = '\0';
    e->music = music;
    return music;
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

    for (int i = 0; i < sound_count; i++)
        Mix_FreeChunk(sounds[i].chunk);
    sound_count = 0;

    for (int i = 0; i < music_count; i++)
        Mix_FreeMusic(musics[i].music);
    music_count = 0;
}
