#include <SDL3/SDL.h>

#include "../components/settings.h"

#include "settings.h"

//==============================================================================

#define STORAGE_DATA_FILE "storage.data"

int _load_storage_value(unsigned int position)
{
    int value = 0;
    size_t dataSize = 0;
    unsigned char *fileData = SDL_LoadFile(STORAGE_DATA_FILE, &dataSize);

    if (fileData != NULL)
    {
        if (dataSize < ((size_t)((position + 1) * sizeof(int))))
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                        "FILEIO: [%s] Failed to find storage position: %u",
                        STORAGE_DATA_FILE, position);
        }
        else
        {
            int *dataPtr = (int *)fileData;
            value = dataPtr[position];
        }

        SDL_free(fileData);

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "FILEIO: [%s] Loaded storage value: %i",
                    STORAGE_DATA_FILE, value);
    }

    return value;
}

bool _save_storage_value(unsigned int position, int value)
{
    bool success = false;
    size_t dataSize = 0;
    size_t newDataSize = 0;
    unsigned char *fileData = SDL_LoadFile(STORAGE_DATA_FILE, &dataSize);
    unsigned char *newFileData = NULL;

    if (fileData != NULL)
    {
        if (dataSize <= ((size_t)position * sizeof(int)))
        {
            // Increase data size up to position and store value
            newDataSize = (position + 1) * sizeof(int);
            newFileData = (unsigned char *)SDL_realloc(fileData, newDataSize);

            if (newFileData != NULL)
            {
                // SDL_realloc succeeded
                int *dataPtr = (int *)newFileData;
                dataPtr[position] = value;
            }
            else
            {
                // SDL_realloc failed
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "FILEIO: [%s] Failed to realloc data (%zu), position in bytes (%zu) bigger than actual file size",
                            STORAGE_DATA_FILE, dataSize, (size_t)position * sizeof(int));

                // We store the old size of the file
                newFileData = fileData;
                newDataSize = dataSize;
            }
        }
        else
        {
            // Store the old size of the file
            newFileData = fileData;
            newDataSize = dataSize;

            // Replace value on selected position
            int *dataPtr = (int *)newFileData;
            dataPtr[position] = value;
        }

        success = SDL_SaveFile(STORAGE_DATA_FILE, newFileData, newDataSize);
        SDL_free(newFileData);

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "FILEIO: [%s] Saved storage value: %i",
                    STORAGE_DATA_FILE, value);
    }
    else
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "FILEIO: [%s] File created successfully",
                    STORAGE_DATA_FILE);

        dataSize = (position + 1) * sizeof(int);
        fileData = (unsigned char *)SDL_malloc(dataSize);
        int *dataPtr = (int *)fileData;
        dataPtr[position] = value;

        success = SDL_SaveFile(STORAGE_DATA_FILE, fileData, dataSize);
        SDL_free(fileData);

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "FILEIO: [%s] Saved storage value: %i",
                    STORAGE_DATA_FILE, value);
    }

    return success;
}

//------------------------------------------------------------------------------

static bool _load_bool(Settings *settings, unsigned int slot)
{
  return (_load_storage_value(slot) == 1);
}

//------------------------------------------------------------------------------

static void _load_settings(ecs_world_t *world)
{
  Settings *settings = ecs_singleton_get_mut(world, Settings);
  settings->music = _load_bool(settings, 0);
  settings->fullscreen = _load_bool(settings, 1);
  ecs_singleton_modified(world, Settings);
}

//------------------------------------------------------------------------------

static void _save_bool(const Settings *settings, unsigned int slot, bool value)
{
  _save_storage_value(slot, value ? 1 : 0);
}

//------------------------------------------------------------------------------
static void _save_settings(ecs_world_t *world)
{
  const Settings *settings = ecs_singleton_get(world, Settings);
  _save_bool(settings, 0, settings->music);
  _save_bool(settings, 1, settings->fullscreen);
}

//------------------------------------------------------------------------------

static void _fini(ecs_world_t *world, void *context)
{
  _save_settings(world);
}

//------------------------------------------------------------------------------

void settings_manager_init(ecs_world_t *world)
{
  ecs_atfini(world, _fini, NULL);
  ecs_singleton_set(world, Settings, {.music = true, .fullscreen = false, .gamepad = -1});
  _load_settings(world);
}
