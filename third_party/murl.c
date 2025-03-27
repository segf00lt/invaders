
#include "murl.h"       // Include the header first
#include <assert.h>
#include <raylib.h>
#include <rlgl.h>      // For potential advanced rendering/state access
#include <stdio.h>     // For snprintf
#include <stdlib.h>
#include <string.h>    // For strlen if needed

#include "microui.h"

// --- Internal State ---

// Enum for icon rendering mode
typedef enum {
    MURL_ICON_MODE_NONE,
    MURL_ICON_MODE_ATLAS,
    MURL_ICON_MODE_FONT
} murl_IconMode;

// Structure to hold murl's internal state and configuration
typedef struct murl_State {
    bool initialized;
    float scroll_multiplier;
    bool gamepad_navigation_enabled;
    int gamepad_index;

    // Icon rendering resources
    murl_IconMode icon_mode;
    Texture2D icon_atlas_texture;
    const Font* icon_font;
    int icon_font_base_size_override; // Use if > 0
    Vector2 icon_font_offset;         // Offset for drawing font icons

    // Mappings (consider dynamic allocation if mapping size can be very large)
    #define MURL_MAX_ICON_MAPPINGS 32 // Example limit
    murl_AtlasRect icon_atlas_map[MURL_MAX_ICON_MAPPINGS];
    int icon_atlas_map_count;
    murl_IconFontMapping icon_font_map[MURL_MAX_ICON_MAPPINGS];
    int icon_font_map_count;

    // Text input buffer (dynamic growth example)
    #define MURL_TEXT_INPUT_INITIAL_SIZE 64
    char* text_input_buffer;
    size_t text_input_buffer_capacity;

    // Style cache (optional, if getting colors frequently is slow)
    // Color cached_colors[MU_COLOR_MAX];

} murl_State;

// Global internal state variable
static murl_State g_murl_state = {0}; // Initialize to zero

// --- Conversion Macros/Functions ---

// Convert mu_Color to raylib Color
static inline Color murl_color_from_mu(mu_Color c) {
    return (Color){c.r, c.g, c.b, c.a};
}

// Convert raylib Color to mu_Color
static inline mu_Color murl_mu_from_color(Color c) {
    return (mu_Color){c.r, c.g, c.b, c.a};
}

// Convert mu_Rect to raylib Rectangle
static inline Rectangle murl_rectangle_from_mu(mu_Rect r) {
    return (Rectangle){(float)r.x, (float)r.y, (float)r.w, (float)r.h};
}

// Convert mu_Vec2 to raylib Vector2
static inline Vector2 murl_vector2_from_mu(mu_Vec2 v) {
    return (Vector2){(float)v.x, (float)v.y};
}

// Convert raylib Font* (stored in mu_Font handle) back to Font
// IMPORTANT: Assumes mu_Font is directly castable to/from Font*. Change if assumption is wrong.
#define MURL_FONT_FROM_MU(mu_font) (*(Font*)(mu_font))


// --- Text Measurement Callbacks ---

int murl__text_width_callback(mu_Font font, const char *str, int len) {
    if (!font || !str) return 0;
    // Note: microui might pass len == -1 to indicate null-terminated string
    // MeasureTextEx handles null termination correctly if len is not used carefully.
    // If len is non-negative, we might need to measure a substring.
    Font rlfont = MURL_FONT_FROM_MU(font);
    float spacing = rlfont.baseSize / 10.0f; // Default raylib spacing if not stored elsewhere

    // Use len if provided and valid, otherwise measure whole string
    if (len >= 0) {
         // MeasureTextEx doesn't directly support length limit easily.
         // We can copy to a temp buffer or iterate codepoints if precise length needed.
         // Simple approach: Measure whole string (less accurate if len matters)
         // OR create a temporary null-terminated string (safer but less efficient)
         char temp_buffer[1024]; // Adjust size as needed
         int copy_len = (len < (int)sizeof(temp_buffer) - 1) ? len : (int)sizeof(temp_buffer) - 1;
         strncpy(temp_buffer, str, copy_len);
         temp_buffer[copy_len] = '\0';
         Vector2 size = MeasureTextEx(rlfont, temp_buffer, (float)rlfont.baseSize, spacing);
         return (int)size.x;
    } else {
         Vector2 size = MeasureTextEx(rlfont, str, (float)rlfont.baseSize, spacing);
         return (int)size.x;
    }
}

int murl__text_height_callback(mu_Font font) {
    if (!font) return 0;
    Font rlfont = MURL_FONT_FROM_MU(font);
    // Consider adding line spacing? Microui seems to expect character height.
    return rlfont.baseSize;
}

// --- Input Handling ---

static void murl__handle_mouse_scroll(mu_Context *ctx) {
    const Vector2 mouse_wheel_scroll = GetMouseWheelMoveV();
    if (mouse_wheel_scroll.x != 0 || mouse_wheel_scroll.y != 0) {
         mu_input_scroll(ctx, (int)(mouse_wheel_scroll.x * -g_murl_state.scroll_multiplier),
                             (int)(mouse_wheel_scroll.y * -g_murl_state.scroll_multiplier));
    }
}

// Mapping for mouse buttons
static const struct { MouseButton rl; int mu; } murl__mouse_buttons[] = {
    {MOUSE_BUTTON_LEFT, MU_MOUSE_LEFT},
    {MOUSE_BUTTON_RIGHT, MU_MOUSE_RIGHT},
    {MOUSE_BUTTON_MIDDLE, MU_MOUSE_MIDDLE},
    // Add MOUSE_BUTTON_SIDE, EXTRA, FORWARD, BACK if needed and microui supports them
};
#define MURL_MOUSE_BUTTON_COUNT (sizeof(murl__mouse_buttons) / sizeof(murl__mouse_buttons[0]))

static void murl__handle_mouse_buttons_input(mu_Context *ctx) {
    const int mouse_x = GetMouseX();
    const int mouse_y = GetMouseY();
    for (size_t i = 0; i < MURL_MOUSE_BUTTON_COUNT; ++i) {
        if (IsMouseButtonPressed(murl__mouse_buttons[i].rl)) {
            mu_input_mousedown(ctx, mouse_x, mouse_y, murl__mouse_buttons[i].mu);
        } else if (IsMouseButtonReleased(murl__mouse_buttons[i].rl)) {
            mu_input_mouseup(ctx, mouse_x, mouse_y, murl__mouse_buttons[i].mu);
        }
    }
}

// Expanded mapping for keyboard keys
static const struct { KeyboardKey rl; int mu; } murl__keyboard_keys[] = {
    {KEY_LEFT_SHIFT, MU_KEY_SHIFT},     {KEY_RIGHT_SHIFT, MU_KEY_SHIFT},
    {KEY_LEFT_CONTROL, MU_KEY_CTRL},    {KEY_RIGHT_CONTROL, MU_KEY_CTRL},
    {KEY_LEFT_ALT, MU_KEY_ALT},         {KEY_RIGHT_ALT, MU_KEY_ALT},
    {KEY_ENTER, MU_KEY_RETURN},         {KEY_KP_ENTER, MU_KEY_RETURN},
    {KEY_BACKSPACE, MU_KEY_BACKSPACE},
    {KEY_DELETE, MU_KEY_DELETE},
    {KEY_TAB, MU_KEY_TAB},
    {KEY_ESCAPE, MU_KEY_ESCAPE},
    {KEY_UP, MU_KEY_UP},
    {KEY_DOWN, MU_KEY_DOWN},
    {KEY_LEFT, MU_KEY_LEFT},
    {KEY_RIGHT, MU_KEY_RIGHT},
    {KEY_HOME, MU_KEY_HOME},
    {KEY_END, MU_KEY_END},
    {KEY_PAGE_UP, MU_KEY_PAGEUP},
    {KEY_PAGE_DOWN, MU_KEY_PAGEDOWN},
    {KEY_F1, MU_KEY_F1}, {KEY_F2, MU_KEY_F2}, {KEY_F3, MU_KEY_F3}, {KEY_F4, MU_KEY_F4},
    {KEY_F5, MU_KEY_F5}, {KEY_F6, MU_KEY_F6}, {KEY_F7, MU_KEY_F7}, {KEY_F8, MU_KEY_F8},
    {KEY_F9, MU_KEY_F9}, {KEY_F10, MU_KEY_F10}, {KEY_F11, MU_KEY_F11}, {KEY_F12, MU_KEY_F12},
    // Add other keys as needed (e.g., function keys, numpad keys if microui supports them)
};
#define MURL_KEYBOARD_KEY_COUNT (sizeof(murl__keyboard_keys) / sizeof(murl__keyboard_keys[0]))

static void murl__handle_keyboard_input(mu_Context *ctx) {
    // Handle mapped special keys
    for (size_t i = 0; i < MURL_KEYBOARD_KEY_COUNT; ++i) {
        if (IsKeyPressed(murl__keyboard_keys[i].rl)) {
            mu_input_keydown(ctx, murl__keyboard_keys[i].mu);
        } else if (IsKeyReleased(murl__keyboard_keys[i].rl)) {
            mu_input_keyup(ctx, murl__keyboard_keys[i].mu);
        }
    }
    // You could also check IsKeyDown here for modifier states, though microui might handle this internally
    // ctx->keys[MU_KEY_SHIFT] = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    // ctx->keys[MU_KEY_CTRL] = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    // ... but sending keydown/keyup events is usually preferred.
}

static void murl__handle_text_input(mu_Context *ctx) {
    // Dynamically growing buffer example
    if (!g_murl_state.text_input_buffer) return; // Not initialized

    size_t buffer_index = 0;
    int codepoint;
    while ((codepoint = GetCharPressed()) != 0) { // GetCharPressed gets unicode codepoints
        // Check if buffer needs resizing
        if (buffer_index + 4 >= g_murl_state.text_input_buffer_capacity) { // +4 for potential UTF-8 max bytes
             size_t new_capacity = g_murl_state.text_input_buffer_capacity * 2;
             char *new_buffer = (char*)realloc(g_murl_state.text_input_buffer, new_capacity);
             if (!new_buffer) {
                 TraceLog(LOG_ERROR, "MURL: Failed to reallocate text input buffer");
                 return; // Stop processing text if allocation fails
             }
             g_murl_state.text_input_buffer = new_buffer;
             g_murl_state.text_input_buffer_capacity = new_capacity;
        }

        // Convert codepoint to UTF-8 and append to buffer
        // Raylib provides EncodeCodepoint (internally used by DrawText) - let's reimplement basic UTF-8 encoding
        if (codepoint <= 0x7F) {
            g_murl_state.text_input_buffer[buffer_index++] = (char)codepoint;
        } else if (codepoint <= 0x7FF) {
            g_murl_state.text_input_buffer[buffer_index++] = (char)(0xC0 | (codepoint >> 6));
            g_murl_state.text_input_buffer[buffer_index++] = (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0xFFFF) {
            g_murl_state.text_input_buffer[buffer_index++] = (char)(0xE0 | (codepoint >> 12));
            g_murl_state.text_input_buffer[buffer_index++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
            g_murl_state.text_input_buffer[buffer_index++] = (char)(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0x10FFFF) {
             g_murl_state.text_input_buffer[buffer_index++] = (char)(0xF0 | (codepoint >> 18));
             g_murl_state.text_input_buffer[buffer_index++] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
             g_murl_state.text_input_buffer[buffer_index++] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
             g_murl_state.text_input_buffer[buffer_index++] = (char)(0x80 | (codepoint & 0x3F));
        }
    }

    // Null-terminate the buffer if any characters were added
    if (buffer_index > 0) {
        g_murl_state.text_input_buffer[buffer_index] = '\0';
        mu_input_text(ctx, g_murl_state.text_input_buffer);
    }
}

static void murl__handle_gamepad_input(mu_Context *ctx) {
    if (!g_murl_state.gamepad_navigation_enabled || !IsGamepadAvailable(g_murl_state.gamepad_index)) {
        return;
    }
    int gp = g_murl_state.gamepad_index;

    // D-Pad to Arrow Keys (consider axis input too)
    if (IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_LEFT_FACE_UP))    mu_input_keydown(ctx, MU_KEY_UP);
    if (IsGamepadButtonReleased(gp, GAMEPAD_BUTTON_LEFT_FACE_UP))   mu_input_keyup(ctx, MU_KEY_UP);
    if (IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_LEFT_FACE_DOWN))  mu_input_keydown(ctx, MU_KEY_DOWN);
    if (IsGamepadButtonReleased(gp, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) mu_input_keyup(ctx, MU_KEY_DOWN);
    if (IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_LEFT_FACE_LEFT))  mu_input_keydown(ctx, MU_KEY_LEFT);
    if (IsGamepadButtonReleased(gp, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) mu_input_keyup(ctx, MU_KEY_LEFT);
    if (IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) mu_input_keydown(ctx, MU_KEY_RIGHT);
    if (IsGamepadButtonReleased(gp, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))mu_input_keyup(ctx, MU_KEY_RIGHT);

    // Action Buttons (Example mapping)
    if (IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))  mu_input_keydown(ctx, MU_KEY_RETURN); // A/Cross
    if (IsGamepadButtonReleased(gp, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) mu_input_keyup(ctx, MU_KEY_RETURN);
    if (IsGamepadButtonPressed(gp, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) mu_input_keydown(ctx, MU_KEY_BACKSPACE); // B/Circle
    if (IsGamepadButtonReleased(gp, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))mu_input_keyup(ctx, MU_KEY_BACKSPACE);
    // Add more mappings (e.g., Start -> Enter?, Select -> Tab?)
}

static void murl__update_cursor(mu_Context *ctx) {
     // Only change if microui indicates a specific cursor is desired
     // (Microui itself doesn't have direct cursor shape support in base commands,
     // often handled by checking hover states externally or via extensions)
     // This is a placeholder; true cursor updates often need more context.

     if (mu_is_any_item_hovered(ctx)) {
         // Potentially change to a hand cursor, requires raylib support or custom cursor loading
         // Example using raylib's standard cursors:
         // if (ctx->hover == ctx->focus) // Example condition
         //    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
         //} else {
         //    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
         //}
     } else {
          SetMouseCursor(MOUSE_CURSOR_DEFAULT);
     }
}


// --- Rendering Implementation ---

// Helper function to find icon atlas rect by ID
static Rectangle murl__get_icon_atlas_rect(int id) {
    for (int i = 0; i < g_murl_state.icon_atlas_map_count; ++i) {
        if (g_murl_state.icon_atlas_map[i].id == id) {
            return g_murl_state.icon_atlas_map[i].rect;
        }
    }
    return (Rectangle){0, 0, 0, 0}; // Not found
}

// Helper function to find icon font codepoint by ID
static int murl__get_icon_font_codepoint(int id) {
    for (int i = 0; i < g_murl_state.icon_font_map_count; ++i) {
        if (g_murl_state.icon_font_map[i].id == id) {
            return g_murl_state.icon_font_map[i].codepoint;
        }
    }
    return '?'; // Fallback character '?'
}

static void murl__render_command(mu_Context *ctx, mu_Command *cmd) {
    switch (cmd->type) {
        case MU_COMMAND_TEXT: {
            if (!cmd->text.font) break; // Skip if no font set
            Font font = MURL_FONT_FROM_MU(cmd->text.font);
            Vector2 text_position = murl_vector2_from_mu(cmd->text.pos);
            // Use the callback to get height consistently
            int font_size = ctx->text_height(cmd->text.font);
             if (font_size <= 0) font_size = font.baseSize; // Fallback if callback failed
            Color text_color = murl_color_from_mu(cmd->text.color);
            float spacing = ctx->style && ctx->style->spacing > 0 ? ctx->style->spacing : (font.baseSize / 10.0f);

            DrawTextEx(font, cmd->text.str, text_position, (float)font_size, spacing, text_color);
        } break;

        case MU_COMMAND_RECT: {
            Rectangle rect = murl_rectangle_from_mu(cmd->rect.rect);
            Color rect_color = murl_color_from_mu(cmd->rect.color);
            DrawRectangleRec(rect, rect_color);

            // Add border rendering
            if (ctx->style && cmd->rect.color.a > 0) { // Check if style exists and rect isn't fully transparent
                mu_Color border_mu_color = ctx->style->colors[MU_COLOR_BORDER];
                if (border_mu_color.a > 0) { // Draw border only if it's visible
                    Color border_color = murl_color_from_mu(border_mu_color);
                    // Draw lines instead of filled rect for border
                    // DrawRectangleLinesEx is useful but might need adjustment for thickness > 1
                    // Simple 1px border:
                     DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h, border_color);
                    // For thicker borders: DrawRectangleLinesEx(rect, thickness, border_color);
                }
            }
        } break;

        case MU_COMMAND_ICON: {
            Color icon_color = murl_color_from_mu(cmd->icon.color);
            mu_Rect mu_rect = cmd->icon.rect;

            if (g_murl_state.icon_mode == MURL_ICON_MODE_ATLAS && g_murl_state.icon_atlas_texture.id > 0) {
                Rectangle source_rect = murl__get_icon_atlas_rect(cmd->icon.id);
                if (source_rect.width > 0 && source_rect.height > 0) {
                    // Draw texture centered in the target rect
                    Rectangle dest_rect = murl_rectangle_from_mu(mu_rect);
                    Vector2 origin = {0, 0}; // Adjust if icons aren't centered in their source rect
                    // Optional: Scale icon if dest_rect size differs from source_rect size?
                    // Simple draw:
                     DrawTextureRec(g_murl_state.icon_atlas_texture, source_rect, (Vector2){dest_rect.x, dest_rect.y}, icon_color);
                     // Centered draw:
                     // float x = dest_rect.x + (dest_rect.width - source_rect.width) / 2.0f;
                     // float y = dest_rect.y + (dest_rect.height - source_rect.height) / 2.0f;
                     // DrawTextureRec(g_murl_state.icon_atlas_texture, source_rect, (Vector2){x, y}, icon_color);
                } else {
                    TraceLog(LOG_WARNING, "MURL: Icon ID %d not found in atlas map.", cmd->icon.id);
                    // Draw fallback text '?'
                    DrawText("?", mu_rect.x + 2, mu_rect.y + 2, mu_rect.h - 4, icon_color);
                }
            } else if (g_murl_state.icon_mode == MURL_ICON_MODE_FONT && g_murl_state.icon_font) {
                int codepoint = murl__get_icon_font_codepoint(cmd->icon.id);
                int font_size = g_murl_state.icon_font_base_size_override > 0 ? g_murl_state.icon_font_base_size_override : g_murl_state.icon_font->baseSize;
                // Measure text to center it? Or assume fixed width? Font Awesome is often monospace-ish.
                 Vector2 text_size = MeasureTextEx(*g_murl_state.icon_font, TextFormat("%c", codepoint), (float)font_size, 1.0f); // Measure single char
                 float x = mu_rect.x + g_murl_state.icon_font_offset.x + (mu_rect.w - text_size.x) / 2.0f;
                 float y = mu_rect.y + g_murl_state.icon_font_offset.y + (mu_rect.h - text_size.y) / 2.0f; // Assumes baseSize approx height
                 DrawTextCodepoint(*g_murl_state.icon_font, codepoint, (Vector2){x, y}, (float)font_size, icon_color);

            } else {
                // Fallback text rendering (original method)
                char icon_char = '?';
                switch (cmd->icon.id) {
                    case MU_ICON_CLOSE:     icon_char = 'x'; break;
                    case MU_ICON_CHECK:     icon_char = '*'; break; // Or use checkmark codepoint if font supports it
                    case MU_ICON_COLLAPSED: icon_char = '+'; break; // Or use right arrow codepoint
                    case MU_ICON_EXPANDED:  icon_char = '-'; break; // Or use down arrow codepoint
                    // Add more standard icons
                    case MU_ICON_WARNING:   icon_char = '!'; break;
                    case MU_ICON_INFO:      icon_char = 'i'; break;
                    default: TraceLog(LOG_WARNING, "MURL: Unmapped icon ID %d.", cmd->icon.id); break;
                }
                // Simple centered text draw
                char text_buffer[2] = {icon_char, '\0'};
                Font current_font = ctx->style && ctx->style->font ? MURL_FONT_FROM_MU(ctx->style->font) : GetFontDefault();
                int font_size = ctx->style && ctx->style->font ? ctx->text_height(ctx->style->font) : current_font.baseSize;
                Vector2 size = MeasureTextEx(current_font, text_buffer, (float)font_size, 1.0f);
                float x = (float)mu_rect.x + ((float)mu_rect.w - size.x) / 2.0f;
                float y = (float)mu_rect.y + ((float)mu_rect.h - size.y) / 2.0f; // Assumes baseSize approx height
                DrawTextEx(current_font, text_buffer, (Vector2){x,y}, (float)font_size, 1.0f, icon_color);
            }
        } break;

        case MU_COMMAND_CLIP: {
            // End previous scissor mode before starting new one
            // Using rlgl directly for potentially slightly more control? Or stick to raylib API.
            // EndScissorMode(); // raylib API
            rlglDraw(); // Process buffered draws before changing scissor
            rlDisableScissorTest(); // rlgl direct

            if (cmd->clip.rect.w > 0 && cmd->clip.rect.h > 0) {
                // BeginScissorMode(cmd->clip.rect.x, cmd->clip.rect.y, cmd->clip.rect.w, cmd->clip.rect.h); // raylib API
                 rlEnableScissorTest(); // rlgl direct
                 // Need to account for flipped Y coordinate if framebuffer origin differs? Usually not needed with raylib screen space.
                 rlScissor(cmd->clip.rect.x, GetScreenHeight() - (cmd->clip.rect.y + cmd->clip.rect.h), cmd->clip.rect.w, cmd->clip.rect.h);
            }
             // If w/h <= 0, scissor test is effectively disabled by rlDisableScissorTest()
        } break;

        case MU_COMMAND_TRIANGLE: {
             Vector2 v0 = murl_vector2_from_mu(cmd->triangle.p0);
             Vector2 v1 = murl_vector2_from_mu(cmd->triangle.p1);
             Vector2 v2 = murl_vector2_from_mu(cmd->triangle.p2);
             Color color = murl_color_from_mu(cmd->triangle.color);
             DrawTriangle(v0, v1, v2, color);
        } break;

         case MU_COMMAND_LINE: {
              Vector2 p0 = murl_vector2_from_mu(cmd->line.p0);
              Vector2 p1 = murl_vector2_from_mu(cmd->line.p1);
              Color color = murl_color_from_mu(cmd->line.color);
              // DrawLineV might be thin, use DrawLineEx for thickness? Microui doesn't specify thickness.
              DrawLineV(p0, p1, color);
              // DrawLineEx(p0, p1, 1.0f, color); // Example 1px thick line
         } break;

         case MU_COMMAND_JUMP: {
              // Usually corresponds to changing Z-order or layer, which isn't
              // directly applicable in raylib's immediate mode rendering typically.
              // This command can often be ignored, or used for custom layer logic.
              // TraceLog(LOG_DEBUG, "MURL: MU_COMMAND_JUMP encountered (offset: %p)", cmd->jump.dst);
         } break;


        default:
            TraceLog(LOG_WARNING, "MURL: Unhandled microui command type: %d", cmd->type);
            break;
    }
}

// --- Public API Implementation ---

bool murl_init_ex(mu_Context *ctx, const murl_Config *config) {
    if (g_murl_state.initialized) {
        TraceLog(LOG_WARNING, "MURL: Already initialized.");
        return true;
    }

    TraceLog(LOG_INFO, "MURL: Initializing...");

    // Apply config or defaults
    g_murl_state.scroll_multiplier = (config && config->scroll_multiplier != 0) ? config->scroll_multiplier : 30.0f;
    g_murl_state.gamepad_navigation_enabled = (config) ? config->enable_gamepad_navigation : false;
    g_murl_state.gamepad_index = (config) ? config->gamepad_index : 0;

    // Initialize text buffer
    g_murl_state.text_input_buffer_capacity = MURL_TEXT_INPUT_INITIAL_SIZE;
    g_murl_state.text_input_buffer = (char*)malloc(g_murl_state.text_input_buffer_capacity);
    if (!g_murl_state.text_input_buffer) {
        TraceLog(LOG_ERROR, "MURL: Failed to allocate text input buffer.");
        return false;
    }
    g_murl_state.text_input_buffer[0] = '\0'; // Start empty

    // Icon state defaults
    g_murl_state.icon_mode = MURL_ICON_MODE_NONE;
    g_murl_state.icon_atlas_texture.id = 0; // Invalid texture
    g_murl_state.icon_font = NULL;
    g_murl_state.icon_atlas_map_count = 0;
    g_murl_state.icon_font_map_count = 0;
    g_murl_state.icon_font_base_size_override = 0;
    g_murl_state.icon_font_offset = (Vector2){0, 0};


    // Setup microui context basics (if not already done)
    // Often done externally before calling init, but good to ensure.
    if (ctx && !ctx->text_width && !ctx->text_height) {
         TraceLog(LOG_INFO, "MURL: Setting default text callbacks (font setup needed).");
         // Setting callbacks without a font is problematic.
         // Require murl_setup_font *after* init.
         ctx->text_width = murl__text_width_callback;
         ctx->text_height = murl__text_height_callback;
    }


    g_murl_state.initialized = true;
    TraceLog(LOG_INFO, "MURL: Initialization complete.");
    return true;
}

bool murl_init(mu_Context *ctx) {
    return murl_init_ex(ctx, NULL); // Call with default config
}


void murl_shutdown(mu_Context *ctx) {
    if (!g_murl_state.initialized) return;
    TraceLog(LOG_INFO, "MURL: Shutting down...");

    // Free resources managed by murl
    if (g_murl_state.text_input_buffer) {
        free(g_murl_state.text_input_buffer);
        g_murl_state.text_input_buffer = NULL;
        g_murl_state.text_input_buffer_capacity = 0;
    }

    // Reset icon state (don't unload textures/fonts owned externally)
    g_murl_state.icon_mode = MURL_ICON_MODE_NONE;
    g_murl_state.icon_atlas_texture.id = 0;
    g_murl_state.icon_font = NULL;
    g_murl_state.icon_atlas_map_count = 0;
    g_murl_state.icon_font_map_count = 0;

    // Reset microui callbacks? Maybe not necessary if context is destroyed.
    if (ctx) {
        // ctx->text_width = NULL;
        // ctx->text_height = NULL;
        // ctx->style->font = NULL; // Don't null out external font pointer
    }

    g_murl_state.initialized = false;
     TraceLog(LOG_INFO, "MURL: Shutdown complete.");
}


void murl_setup_font(mu_Context *ctx, const Font *font) {
    if (!g_murl_state.initialized || !ctx) return;
    if (!font) {
        TraceLog(LOG_WARNING, "MURL: Attempted to setup NULL font.");
        // Optionally reset to default or clear font settings?
        ctx->style->font = NULL;
        // Keep callbacks? Microui might crash if font is NULL but callbacks exist.
        // ctx->text_width = NULL;
        // ctx->text_height = NULL;
        return;
    }

    ctx->style->font = (mu_Font)font; // Store pointer to raylib font
    // Set callbacks if not already set (or always overwrite?)
    ctx->text_width = murl__text_width_callback;
    ctx->text_height = murl__text_height_callback;

    // Set default spacing based on font size? Or require manual setting?
    // ctx->style->spacing = font->baseSize / 10.0f; // Example default
}

void murl_set_color(mu_Context *ctx, int color_id, Color color) {
     if (!g_murl_state.initialized || !ctx || !ctx->style || color_id < 0 || color_id >= MU_COLOR_MAX) {
         return;
     }
     ctx->style->colors[color_id] = murl_mu_from_color(color);
}

Color murl_get_color(mu_Context *ctx, int color_id) {
     if (!g_murl_state.initialized || !ctx || !ctx->style || color_id < 0 || color_id >= MU_COLOR_MAX) {
         return BLACK; // Return default or error color
     }
     return murl_color_from_mu(ctx->style->colors[color_id]);
}

void murl_setup_icon_atlas(Texture2D atlas_texture, const murl_AtlasRect *mapping, int mapping_count) {
     if (!g_murl_state.initialized) return;
     if (atlas_texture.id <= 0 || !mapping || mapping_count <= 0) {
          TraceLog(LOG_WARNING, "MURL: Invalid arguments for icon atlas setup.");
          g_murl_state.icon_mode = MURL_ICON_MODE_NONE;
          g_murl_state.icon_atlas_texture.id = 0;
          g_murl_state.icon_atlas_map_count = 0;
          return;
     }
     if (mapping_count > MURL_MAX_ICON_MAPPINGS) {
          TraceLog(LOG_WARNING, "MURL: Icon atlas mapping count (%d) exceeds limit (%d). Clamping.", mapping_count, MURL_MAX_ICON_MAPPINGS);
          mapping_count = MURL_MAX_ICON_MAPPINGS;
     }

     g_murl_state.icon_mode = MURL_ICON_MODE_ATLAS;
     g_murl_state.icon_atlas_texture = atlas_texture; // Store texture (struct copy)
     memcpy(g_murl_state.icon_atlas_map, mapping, sizeof(murl_AtlasRect) * mapping_count);
     g_murl_state.icon_atlas_map_count = mapping_count;
     // Invalidate font icon settings
     g_murl_state.icon_font = NULL;
     g_murl_state.icon_font_map_count = 0;
     TraceLog(LOG_INFO, "MURL: Icon atlas setup complete (%d icons).", mapping_count);
}


void murl_setup_icon_font(const Font *icon_font, const murl_IconFontMapping *mapping, int mapping_count, int base_size, Vector2 offset) {
     if (!g_murl_state.initialized) return;
      if (!icon_font || icon_font->texture.id <= 0 || !mapping || mapping_count <= 0) {
           TraceLog(LOG_WARNING, "MURL: Invalid arguments for icon font setup.");
           g_murl_state.icon_mode = MURL_ICON_MODE_NONE;
           g_murl_state.icon_font = NULL;
           g_murl_state.icon_font_map_count = 0;
           return;
      }
      if (mapping_count > MURL_MAX_ICON_MAPPINGS) {
           TraceLog(LOG_WARNING, "MURL: Icon font mapping count (%d) exceeds limit (%d). Clamping.", mapping_count, MURL_MAX_ICON_MAPPINGS);
           mapping_count = MURL_MAX_ICON_MAPPINGS;
      }

      g_murl_state.icon_mode = MURL_ICON_MODE_FONT;
      g_murl_state.icon_font = icon_font; // Store pointer
      memcpy(g_murl_state.icon_font_map, mapping, sizeof(murl_IconFontMapping) * mapping_count);
      g_murl_state.icon_font_map_count = mapping_count;
      g_murl_state.icon_font_base_size_override = base_size;
      g_murl_state.icon_font_offset = offset;
      // Invalidate atlas icon settings
      g_murl_state.icon_atlas_texture.id = 0;
      g_murl_state.icon_atlas_map_count = 0;
      TraceLog(LOG_INFO, "MURL: Icon font setup complete (%d icons).", mapping_count);
}

void murl_enable_gamepad_navigation(bool enabled) {
    if (!g_murl_state.initialized) return;
    g_murl_state.gamepad_navigation_enabled = enabled;
}

void murl_set_scroll_multiplier(float multiplier) {
    if (!g_murl_state.initialized) return;
    g_murl_state.scroll_multiplier = multiplier;
}

void murl_process_input(mu_Context *ctx) {
    if (!g_murl_state.initialized || !ctx) return;

    // Mouse position (always needed)
    const Vector2 mouse_pos = GetMousePosition();
    mu_input_mousemove(ctx, (int)mouse_pos.x, (int)mouse_pos.y);

    // Other inputs
    murl__handle_mouse_scroll(ctx);
    murl__handle_mouse_buttons_input(ctx);
    murl__handle_keyboard_input(ctx);
    murl__handle_text_input(ctx);
    murl__handle_gamepad_input(ctx); // Handles its own enabled check

    // Update cursor state based on UI hover (optional)
    murl__update_cursor(ctx);
}

void murl_render_frame(mu_Context *ctx) {
    if (!g_murl_state.initialized || !ctx) return;

    // Use rlgl directly for scissor management to ensure proper batch breaking
    rlglDraw(); // Draw buffered content before first scissor change
    rlDisableScissorTest(); // Start with scissor disabled globally for the frame

    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
        murl__render_command(ctx, cmd); // Delegate rendering of each command
    }

    // Ensure scissor is disabled at the end of the frame
    rlglDraw(); // Draw remaining buffered content
    rlDisableScissorTest();

    // Old raylib API way:
    // BeginScissorMode(0, 0, GetScreenWidth(), GetScreenHeight()); // Start full screen
    // mu_Command *cmd = NULL;
    // while (mu_next_command(ctx, &cmd)) {
    //      if (cmd->type == MU_COMMAND_CLIP) {
    //          EndScissorMode(); // End previous
    //          BeginScissorMode(cmd->clip.rect.x, cmd->clip.rect.y, cmd->clip.rect.w, cmd->clip.rect.h);
    //      } else {
    //          murl__render_command(ctx, cmd); // Render others
    //      }
    // }
    // EndScissorMode(); // End final scissor mode
}
