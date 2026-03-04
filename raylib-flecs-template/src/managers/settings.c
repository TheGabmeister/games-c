#include <raylib.h>

#include "../components/settings.h"

#include "settings.h"

//==============================================================================

#define STORAGE_DATA_FILE "storage.data"

int _load_storage_value(unsigned int position)
{
    int value = 0;
    int dataSize = 0;
    unsigned char *fileData = LoadFileData(STORAGE_DATA_FILE, &dataSize);

    if (fileData != NULL)
    {
        if (dataSize < ((int)(position*4))) TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to find storage position: %i", STORAGE_DATA_FILE, position);
        else
        {
            int *dataPtr = (int *)fileData;
            value = dataPtr[position];
        }

        UnloadFileData(fileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] Loaded storage value: %i", STORAGE_DATA_FILE, value);
    }

    return value;
}

bool _save_storage_value(unsigned int position, int value)
{
    bool success = false;
    int dataSize = 0;
    unsigned int newDataSize = 0;
    unsigned char *fileData = LoadFileData(STORAGE_DATA_FILE, &dataSize);
    unsigned char *newFileData = NULL;

    if (fileData != NULL)
    {
        if (dataSize <= (position*sizeof(int)))
        {
            // Increase data size up to position and store value
            newDataSize = (position + 1)*sizeof(int);
            newFileData = (unsigned char *)RL_REALLOC(fileData, newDataSize);

            if (newFileData != NULL)
            {
                // RL_REALLOC succeded
                int *dataPtr = (int *)newFileData;
                dataPtr[position] = value;
            }
            else
            {
                // RL_REALLOC failed
                TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to realloc data (%u), position in bytes (%u) bigger than actual file size", STORAGE_DATA_FILE, dataSize, position*sizeof(int));

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

        success = SaveFileData(STORAGE_DATA_FILE, newFileData, newDataSize);
        RL_FREE(newFileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] Saved storage value: %i", STORAGE_DATA_FILE, value);
    }
    else
    {
        TraceLog(LOG_INFO, "FILEIO: [%s] File created successfully", STORAGE_DATA_FILE);

        dataSize = (position + 1)*sizeof(int);
        fileData = (unsigned char *)RL_MALLOC(dataSize);
        int *dataPtr = (int *)fileData;
        dataPtr[position] = value;

        success = SaveFileData(STORAGE_DATA_FILE, fileData, dataSize);
        UnloadFileData(fileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] Saved storage value: %i", STORAGE_DATA_FILE, value);
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
