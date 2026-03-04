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

// Keyboard keys (US keyboard layout)
typedef enum {
    KEY_NULL            = 0,        
    KEY_APOSTROPHE      = 39,       
    KEY_COMMA           = 44,       
    KEY_MINUS           = 45,       
    KEY_PERIOD          = 46,       
    KEY_SLASH           = 47,       
    KEY_ZERO            = 48,       
    KEY_ONE             = 49,       
    KEY_TWO             = 50,       
    KEY_THREE           = 51,       
    KEY_FOUR            = 52,       
    KEY_FIVE            = 53,       
    KEY_SIX             = 54,       
    KEY_SEVEN           = 55,       
    KEY_EIGHT           = 56,       
    KEY_NINE            = 57,       
    KEY_SEMICOLON       = 59,       
    KEY_EQUAL           = 61,       
    KEY_A               = 65,        
    KEY_B               = 66,        
    KEY_C               = 67,        
    KEY_D               = 68,        
    KEY_E               = 69,        
    KEY_F               = 70,        
    KEY_G               = 71,        
    KEY_H               = 72,        
    KEY_I               = 73,        
    KEY_J               = 74,        
    KEY_K               = 75,        
    KEY_L               = 76,        
    KEY_M               = 77,        
    KEY_N               = 78,        
    KEY_O               = 79,        
    KEY_P               = 80,        
    KEY_Q               = 81,        
    KEY_R               = 82,        
    KEY_S               = 83,        
    KEY_T               = 84,        
    KEY_U               = 85,        
    KEY_V               = 86,        
    KEY_W               = 87,        
    KEY_X               = 88,        
    KEY_Y               = 89,        
    KEY_Z               = 90,        
    KEY_LEFT_BRACKET    = 91,       
    KEY_BACKSLASH       = 92,       
    KEY_RIGHT_BRACKET   = 93,       
    KEY_GRAVE           = 96,       
    // Function keys
    KEY_SPACE           = 32,       
    KEY_ESCAPE          = 256,      
    KEY_ENTER           = 257,      
    KEY_TAB             = 258,      
    KEY_BACKSPACE       = 259,      
    KEY_INSERT          = 260,      
    KEY_DELETE          = 261,      
    KEY_RIGHT           = 262,      
    KEY_LEFT            = 263,      
    KEY_DOWN            = 264,      
    KEY_UP              = 265,      
    KEY_PAGE_UP         = 266,      
    KEY_PAGE_DOWN       = 267,      
    KEY_HOME            = 268,      
    KEY_END             = 269,      
    KEY_CAPS_LOCK       = 280,   
    KEY_SCROLL_LOCK     = 281,    
    KEY_NUM_LOCK        = 282,  
    KEY_PRINT_SCREEN    = 283,
    KEY_PAUSE           = 284,    
    KEY_F1              = 290,      
    KEY_F2              = 291,      
    KEY_F3              = 292,      
    KEY_F4              = 293,      
    KEY_F5              = 294,      
    KEY_F6              = 295,      
    KEY_F7              = 296,      
    KEY_F8              = 297,      
    KEY_F9              = 298,      
    KEY_F10             = 299,      
    KEY_F11             = 300,      
    KEY_F12             = 301,      
    KEY_LEFT_SHIFT      = 340,     
    KEY_LEFT_CONTROL    = 341,     
    KEY_LEFT_ALT        = 342,      
    KEY_LEFT_SUPER      = 343,      
    KEY_RIGHT_SHIFT     = 344,     
    KEY_RIGHT_CONTROL   = 345,     
    KEY_RIGHT_ALT       = 346,      
    KEY_RIGHT_SUPER     = 347,    
    KEY_KB_MENU         = 348,     
    // Keypad keys
    KEY_KP_0            = 320,      
    KEY_KP_1            = 321,      
    KEY_KP_2            = 322,      
    KEY_KP_3            = 323,      
    KEY_KP_4            = 324,      
    KEY_KP_5            = 325,      
    KEY_KP_6            = 326,      
    KEY_KP_7            = 327,      
    KEY_KP_8            = 328,      
    KEY_KP_9            = 329,      
    KEY_KP_DECIMAL      = 330,      
    KEY_KP_DIVIDE       = 331,      
    KEY_KP_MULTIPLY     = 332,      
    KEY_KP_SUBTRACT     = 333,      
    KEY_KP_ADD          = 334,      
    KEY_KP_ENTER        = 335,     
    KEY_KP_EQUAL        = 336,      
    // Android key buttons
    KEY_BACK            = 4,        
    KEY_MENU            = 5,        
    KEY_VOLUME_UP       = 24,      
    KEY_VOLUME_DOWN     = 25 
} KeyboardKey;

// Add backwards compatibility support for deprecated names
#define MOUSE_LEFT_BUTTON   MOUSE_BUTTON_LEFT
#define MOUSE_RIGHT_BUTTON  MOUSE_BUTTON_RIGHT
#define MOUSE_MIDDLE_BUTTON MOUSE_BUTTON_MIDDLE

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
ENGINE_API int get_fps(void);                                          
ENGINE_API float get_deltatime(void);                                   
ENGINE_API SDL_Renderer *get_renderer(void);                            
ENGINE_API bool is_key_down(int key); 
ENGINE_API bool is_key_pressed(int key); 

#endif // ENGINE_H

