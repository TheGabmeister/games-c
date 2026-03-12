#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <SDL3/SDL.h>

// Function specifiers in case library is build/used as a shared library
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
// NOTE: visibility("default") attribute makes symbols "visible" when compiled with -fvisibility=hidden
#if defined(_WIN32)
    #if defined(__TINYC__)
        #define __declspec(x) __attribute__((x))
    #endif
    #if defined(ENGINE_BUILD_SHARED)
        #define ENGINE_API __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(ENGINE_USE_SHARED)
        #define ENGINE_API __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#else
    #if defined(ENGINE_BUILD_SHARED)
        #define ENGINE_API __attribute__((visibility("default"))) // We are building as a Unix shared library (.so/.dylib)
    #endif
#endif

#ifndef ENGINE_API
    #define ENGINE_API       // Functions defined as 'extern' by default (implicit specifiers)
#endif

typedef SDL_FRect rectangle;
typedef SDL_Color color;
typedef struct { float x, y; } vector2;
typedef struct { float x, y, z; } vector3;
typedef struct { float x, y, z, w; } vector4;

// 2D camera
typedef struct camera {
    vector2 offset;         
    vector2 target;         
    float rotation;         
    float zoom;             
} camera;

// Keyboard keys — values are SDL_Scancode
typedef enum {
    KEY_NULL            = SDL_SCANCODE_UNKNOWN,
    KEY_APOSTROPHE      = SDL_SCANCODE_APOSTROPHE,
    KEY_COMMA           = SDL_SCANCODE_COMMA,
    KEY_MINUS           = SDL_SCANCODE_MINUS,
    KEY_PERIOD          = SDL_SCANCODE_PERIOD,
    KEY_SLASH           = SDL_SCANCODE_SLASH,
    KEY_ZERO            = SDL_SCANCODE_0,
    KEY_ONE             = SDL_SCANCODE_1,
    KEY_TWO             = SDL_SCANCODE_2,
    KEY_THREE           = SDL_SCANCODE_3,
    KEY_FOUR            = SDL_SCANCODE_4,
    KEY_FIVE            = SDL_SCANCODE_5,
    KEY_SIX             = SDL_SCANCODE_6,
    KEY_SEVEN           = SDL_SCANCODE_7,
    KEY_EIGHT           = SDL_SCANCODE_8,
    KEY_NINE            = SDL_SCANCODE_9,
    KEY_SEMICOLON       = SDL_SCANCODE_SEMICOLON,
    KEY_EQUAL           = SDL_SCANCODE_EQUALS,
    KEY_A               = SDL_SCANCODE_A,
    KEY_B               = SDL_SCANCODE_B,
    KEY_C               = SDL_SCANCODE_C,
    KEY_D               = SDL_SCANCODE_D,
    KEY_E               = SDL_SCANCODE_E,
    KEY_F               = SDL_SCANCODE_F,
    KEY_G               = SDL_SCANCODE_G,
    KEY_H               = SDL_SCANCODE_H,
    KEY_I               = SDL_SCANCODE_I,
    KEY_J               = SDL_SCANCODE_J,
    KEY_K               = SDL_SCANCODE_K,
    KEY_L               = SDL_SCANCODE_L,
    KEY_M               = SDL_SCANCODE_M,
    KEY_N               = SDL_SCANCODE_N,
    KEY_O               = SDL_SCANCODE_O,
    KEY_P               = SDL_SCANCODE_P,
    KEY_Q               = SDL_SCANCODE_Q,
    KEY_R               = SDL_SCANCODE_R,
    KEY_S               = SDL_SCANCODE_S,
    KEY_T               = SDL_SCANCODE_T,
    KEY_U               = SDL_SCANCODE_U,
    KEY_V               = SDL_SCANCODE_V,
    KEY_W               = SDL_SCANCODE_W,
    KEY_X               = SDL_SCANCODE_X,
    KEY_Y               = SDL_SCANCODE_Y,
    KEY_Z               = SDL_SCANCODE_Z,
    KEY_LEFT_BRACKET    = SDL_SCANCODE_LEFTBRACKET,
    KEY_BACKSLASH       = SDL_SCANCODE_BACKSLASH,
    KEY_RIGHT_BRACKET   = SDL_SCANCODE_RIGHTBRACKET,
    KEY_GRAVE           = SDL_SCANCODE_GRAVE,
    // Function keys
    KEY_SPACE           = SDL_SCANCODE_SPACE,
    KEY_ESCAPE          = SDL_SCANCODE_ESCAPE,
    KEY_ENTER           = SDL_SCANCODE_RETURN,
    KEY_TAB             = SDL_SCANCODE_TAB,
    KEY_BACKSPACE       = SDL_SCANCODE_BACKSPACE,
    KEY_INSERT          = SDL_SCANCODE_INSERT,
    KEY_DELETE          = SDL_SCANCODE_DELETE,
    KEY_RIGHT           = SDL_SCANCODE_RIGHT,
    KEY_LEFT            = SDL_SCANCODE_LEFT,
    KEY_DOWN            = SDL_SCANCODE_DOWN,
    KEY_UP              = SDL_SCANCODE_UP,
    KEY_PAGE_UP         = SDL_SCANCODE_PAGEUP,
    KEY_PAGE_DOWN       = SDL_SCANCODE_PAGEDOWN,
    KEY_HOME            = SDL_SCANCODE_HOME,
    KEY_END             = SDL_SCANCODE_END,
    KEY_CAPS_LOCK       = SDL_SCANCODE_CAPSLOCK,
    KEY_SCROLL_LOCK     = SDL_SCANCODE_SCROLLLOCK,
    KEY_NUM_LOCK        = SDL_SCANCODE_NUMLOCKCLEAR,
    KEY_PRINT_SCREEN    = SDL_SCANCODE_PRINTSCREEN,
    KEY_PAUSE           = SDL_SCANCODE_PAUSE,
    KEY_F1              = SDL_SCANCODE_F1,
    KEY_F2              = SDL_SCANCODE_F2,
    KEY_F3              = SDL_SCANCODE_F3,
    KEY_F4              = SDL_SCANCODE_F4,
    KEY_F5              = SDL_SCANCODE_F5,
    KEY_F6              = SDL_SCANCODE_F6,
    KEY_F7              = SDL_SCANCODE_F7,
    KEY_F8              = SDL_SCANCODE_F8,
    KEY_F9              = SDL_SCANCODE_F9,
    KEY_F10             = SDL_SCANCODE_F10,
    KEY_F11             = SDL_SCANCODE_F11,
    KEY_F12             = SDL_SCANCODE_F12,
    KEY_LEFT_SHIFT      = SDL_SCANCODE_LSHIFT,
    KEY_LEFT_CONTROL    = SDL_SCANCODE_LCTRL,
    KEY_LEFT_ALT        = SDL_SCANCODE_LALT,
    KEY_LEFT_SUPER      = SDL_SCANCODE_LGUI,
    KEY_RIGHT_SHIFT     = SDL_SCANCODE_RSHIFT,
    KEY_RIGHT_CONTROL   = SDL_SCANCODE_RCTRL,
    KEY_RIGHT_ALT       = SDL_SCANCODE_RALT,
    KEY_RIGHT_SUPER     = SDL_SCANCODE_RGUI,
    KEY_KB_MENU         = SDL_SCANCODE_APPLICATION,
    // Keypad keys
    KEY_KP_0            = SDL_SCANCODE_KP_0,
    KEY_KP_1            = SDL_SCANCODE_KP_1,
    KEY_KP_2            = SDL_SCANCODE_KP_2,
    KEY_KP_3            = SDL_SCANCODE_KP_3,
    KEY_KP_4            = SDL_SCANCODE_KP_4,
    KEY_KP_5            = SDL_SCANCODE_KP_5,
    KEY_KP_6            = SDL_SCANCODE_KP_6,
    KEY_KP_7            = SDL_SCANCODE_KP_7,
    KEY_KP_8            = SDL_SCANCODE_KP_8,
    KEY_KP_9            = SDL_SCANCODE_KP_9,
    KEY_KP_DECIMAL      = SDL_SCANCODE_KP_PERIOD,
    KEY_KP_DIVIDE       = SDL_SCANCODE_KP_DIVIDE,
    KEY_KP_MULTIPLY     = SDL_SCANCODE_KP_MULTIPLY,
    KEY_KP_SUBTRACT     = SDL_SCANCODE_KP_MINUS,
    KEY_KP_ADD          = SDL_SCANCODE_KP_PLUS,
    KEY_KP_ENTER        = SDL_SCANCODE_KP_ENTER,
    KEY_KP_EQUAL        = SDL_SCANCODE_KP_EQUALS,
} KeyboardKey;

// Mouse buttons
typedef enum {
    MOUSE_BUTTON_LEFT    = 0,       
    MOUSE_BUTTON_RIGHT   = 1,       
    MOUSE_BUTTON_MIDDLE  = 2,       
    MOUSE_BUTTON_SIDE    = 3,        
    MOUSE_BUTTON_EXTRA   = 4,      
    MOUSE_BUTTON_FORWARD = 5,       
    MOUSE_BUTTON_BACK    = 6,      
} MouseButton;

// Mouse cursor
typedef enum {
    MOUSE_CURSOR_DEFAULT       = 0,    
    MOUSE_CURSOR_ARROW         = 1,     
    MOUSE_CURSOR_IBEAM         = 2,  
    MOUSE_CURSOR_CROSSHAIR     = 3,     
    MOUSE_CURSOR_POINTING_HAND = 4,  
    MOUSE_CURSOR_RESIZE_EW     = 5,    
    MOUSE_CURSOR_RESIZE_NS     = 6,    
    MOUSE_CURSOR_RESIZE_NWSE   = 7,   
    MOUSE_CURSOR_RESIZE_NESW   = 8,    
    MOUSE_CURSOR_RESIZE_ALL    = 9,    
    MOUSE_CURSOR_NOT_ALLOWED   = 10  
} MouseCursor;

// Gamepad buttons
typedef enum {
    GAMEPAD_BUTTON_UNKNOWN = 0,        
    GAMEPAD_BUTTON_LEFT_FACE_UP,        
    GAMEPAD_BUTTON_LEFT_FACE_RIGHT,    
    GAMEPAD_BUTTON_LEFT_FACE_DOWN,    
    GAMEPAD_BUTTON_LEFT_FACE_LEFT,  
    GAMEPAD_BUTTON_RIGHT_FACE_UP,       
    GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, 
    GAMEPAD_BUTTON_RIGHT_FACE_DOWN,    
    GAMEPAD_BUTTON_RIGHT_FACE_LEFT,    
    GAMEPAD_BUTTON_LEFT_TRIGGER_1,     
    GAMEPAD_BUTTON_LEFT_TRIGGER_2,   
    GAMEPAD_BUTTON_RIGHT_TRIGGER_1,    
    GAMEPAD_BUTTON_RIGHT_TRIGGER_2,   
    GAMEPAD_BUTTON_MIDDLE_LEFT,       
    GAMEPAD_BUTTON_MIDDLE,         
    GAMEPAD_BUTTON_MIDDLE_RIGHT,     
    GAMEPAD_BUTTON_LEFT_THUMB,    
    GAMEPAD_BUTTON_RIGHT_THUMB    
} GamepadButton;

// Gamepad axis
typedef enum {
    GAMEPAD_AXIS_LEFT_X        = 0,     
    GAMEPAD_AXIS_LEFT_Y        = 1,     
    GAMEPAD_AXIS_RIGHT_X       = 2, 
    GAMEPAD_AXIS_RIGHT_Y       = 3,   
    GAMEPAD_AXIS_LEFT_TRIGGER  = 4,
    GAMEPAD_AXIS_RIGHT_TRIGGER = 5     
} GamepadAxis;

ENGINE_API void init_window(int width, int height, const char *title);  
ENGINE_API void close_window(void);                                     
ENGINE_API bool is_window_ready(void);
ENGINE_API bool window_should_close(void);
ENGINE_API int get_fps(void);                                          
ENGINE_API float get_deltatime(void);                                   
ENGINE_API SDL_Renderer *get_renderer(void);                            
ENGINE_API bool is_key_down(int key);
ENGINE_API bool is_key_pressed(int key);
ENGINE_API void engine_begin_frame(void);
ENGINE_API void engine_process_event(SDL_Event *event);
ENGINE_API bool is_mouse_button_down(int button);
ENGINE_API bool is_mouse_button_pressed(int button);
ENGINE_API vector2 get_mouse_position(void);
ENGINE_API float get_mouse_wheel_move(void);

#endif // ENGINE_H

