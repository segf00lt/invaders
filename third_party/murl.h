#ifndef MURL_H
#define MURL_H

#include <raylib.h>
#include "microui.h"

// --- Configuration ---

/** @brief Configuration structure for murl initialization (optional). */
typedef struct murl_Config {
    float scroll_multiplier;       // Multiplier for mouse wheel scrolling. Default: 30.0f
    bool enable_gamepad_navigation; // Enable basic UI navigation with gamepad. Default: false
    int gamepad_index;             // Gamepad index to use for navigation. Default: 0
    // Add other config options here...
} murl_Config;

// --- Initialization & Shutdown ---

/**
 * @brief Initializes the murl binding library with default settings.
 * Must be called after raylib's InitWindow and before other murl functions.
 * @param ctx Pointer to the microui context.
 * @return True on success, false on failure.
 */
bool murl_init(mu_Context *ctx);

/**
 * @brief Initializes the murl binding library with custom configuration.
 * Must be called after raylib's InitWindow and before other murl functions.
 * @param ctx Pointer to the microui context.
 * @param config Pointer to configuration struct. If NULL, uses defaults.
 * @return True on success, false on failure.
 */
bool murl_init_ex(mu_Context *ctx, const murl_Config *config);

/**
 * @brief Shuts down the murl library and releases associated resources (if any managed by murl).
 * Call before raylib's CloseWindow.
 * @param ctx Pointer to the microui context.
 */
void murl_shutdown(mu_Context *ctx);

// --- Font & Style Setup ---

/**
 * @brief Sets up microui to use a specific raylib font.
 * Requires murl to be initialized first.
 * @param ctx Pointer to the microui context.
 * @param font Pointer to the loaded raylib Font. Pass NULL to potentially reset to a default?
 */
void murl_setup_font(mu_Context *ctx, const Font *font);

/**
 * @brief Sets a specific color in the microui style.
 * Requires murl to be initialized first.
 * @param ctx Pointer to the microui context.
 * @param color_id The microui color ID (e.g., MU_COLOR_TEXT, MU_COLOR_BG).
 * @param color The raylib Color to set.
 */
void murl_set_color(mu_Context *ctx, int color_id, Color color);

/**
 * @brief Gets a specific color from the microui style as a raylib Color.
 * Requires murl to be initialized first.
 * @param ctx Pointer to the microui context.
 * @param color_id The microui color ID.
 * @return The color as a raylib Color.
 */
Color murl_get_color(mu_Context *ctx, int color_id);

// Add more style setters as needed (murl_set_style_size, murl_set_style_padding, etc.)
// void murl_set_style_size(mu_Context *ctx, int size_id, int size);
// ...

// --- Icon Setup ---

/** @brief Structure to define a rectangle within an icon atlas texture. */
typedef struct murl_AtlasRect {
    int id;        // The microui MU_ICON_ ID
    Rectangle rect; // The source rectangle within the atlas texture
} murl_AtlasRect;

/**
 * @brief Configures murl to use a texture atlas for rendering icons.
 * Requires murl to be initialized first.
 * @param atlas_texture The raylib Texture2D containing the icon atlas.
 * @param mapping Array of murl_AtlasRect defining the mapping from MU_ICON_ IDs to texture rectangles.
 * @param mapping_count Number of elements in the mapping array.
 */
void murl_setup_icon_atlas(Texture2D atlas_texture, const murl_AtlasRect *mapping, int mapping_count);

/** @brief Structure to define a codepoint mapping for an icon font. */
typedef struct murl_IconFontMapping {
    int id;        // The microui MU_ICON_ ID
    int codepoint; // The Unicode codepoint in the icon font
} murl_IconFontMapping;

/**
 * @brief Configures murl to use a specific font and codepoint mapping for rendering icons.
 * Requires murl to be initialized first.
 * @param icon_font Pointer to the loaded raylib Font containing the icons (e.g., Font Awesome).
 * @param mapping Array of murl_IconFontMapping defining the mapping from MU_ICON_ IDs to codepoints.
 * @param mapping_count Number of elements in the mapping array.
 * @param base_size Optional: Specify a base size override for drawing icons. If 0, uses icon_font->baseSize.
 * @param offset Optional: Specify an offset for drawing icons.
 */
void murl_setup_icon_font(const Font *icon_font, const murl_IconFontMapping *mapping, int mapping_count, int base_size, Vector2 offset);


// --- Input Processing ---

/**
 * @brief Processes all relevant raylib input events (mouse, keyboard, text, gamepad if enabled)
 * and sends them to the microui context. Call this once per frame before mu_begin.
 * Requires murl to be initialized first.
 * @param ctx Pointer to the microui context.
 */
void murl_process_input(mu_Context *ctx);

/**
 * @brief Enables or disables basic gamepad navigation mapped to keyboard events.
 * Requires murl to be initialized first.
 * @param enabled True to enable, false to disable.
 */
void murl_enable_gamepad_navigation(bool enabled);

/**
 * @brief Sets the multiplier applied to mouse wheel scrolling delta.
 * Requires murl to be initialized first. Default is 30.0f. Negative values invert direction.
 * @param multiplier The scroll multiplier.
 */
void murl_set_scroll_multiplier(float multiplier);

// --- Rendering ---

/**
 * @brief Renders the microui interface using raylib draw calls.
 * Call this once per frame after mu_end.
 * Requires murl to be initialized first.
 * @param ctx Pointer to the microui context.
 */
void murl_render_frame(mu_Context *ctx);


// --- Callback Function Declarations (Internal Use) ---
// These might not need to be in the public header unless customizable
int murl__text_width_callback(mu_Font font, const char *str, int len);
int murl__text_height_callback(mu_Font font);


#endif // MURL_H
