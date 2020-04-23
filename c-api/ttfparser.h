/**
 * @file ttfparser.h
 *
 * A C API for the Rust's ttf-parser library.
 */

#ifndef TTFP_H
#define TTFP_H

#include <stdbool.h>
#include <stdint.h>

#define TTFP_MAJOR_VERSION 0
#define TTFP_MINOR_VERSION 5
#define TTFP_PATCH_VERSION 0
#define TTFP_VERSION "0.5.0"

/**
 * @brief A table name.
 */
typedef enum {
    TTFP_TABLE_NAME_AXIS_VARIATIONS = 0,
    TTFP_TABLE_NAME_CHARACTER_TO_GLYPH_INDEX_MAPPING,
    TTFP_TABLE_NAME_COLOR_BITMAP_DATA,
    TTFP_TABLE_NAME_COLOR_BITMAP_LOCATION,
    TTFP_TABLE_NAME_COMPACT_FONT_FORMAT,
    TTFP_TABLE_NAME_COMPACT_FONT_FORMAT2,
    TTFP_TABLE_NAME_FONT_VARIATIONS,
    TTFP_TABLE_NAME_GLYPH_DATA,
    TTFP_TABLE_NAME_GLYPH_DEFINITION,
    TTFP_TABLE_NAME_GLYPH_VARIATIONS,
    TTFP_TABLE_NAME_HEADER,
    TTFP_TABLE_NAME_HORIZONTAL_HEADER,
    TTFP_TABLE_NAME_HORIZONTAL_METRICS,
    TTFP_TABLE_NAME_HORIZONTAL_METRICS_VARIATIONS,
    TTFP_TABLE_NAME_INDEX_TO_LOCATION,
    TTFP_TABLE_NAME_KERNING,
    TTFP_TABLE_NAME_MAXIMUM_PROFILE,
    TTFP_TABLE_NAME_METRICS_VARIATIONS,
    TTFP_TABLE_NAME_NAMING,
    TTFP_TABLE_NAME_POST_SCRIPT,
    TTFP_TABLE_NAME_SCALABLE_VECTOR_GRAPHICS,
    TTFP_TABLE_NAME_STANDARD_BITMAP_GRAPHICS,
    TTFP_TABLE_NAME_VERTICAL_HEADER,
    TTFP_TABLE_NAME_VERTICAL_METRICS,
    TTFP_TABLE_NAME_VERTICAL_METRICS_VARIATIONS,
    TTFP_TABLE_NAME_VERTICAL_ORIGIN,
    TTFP_TABLE_NAME_WINDOWS_METRICS,
} ttfp_table_name;

/**
 * @brief A list of glyph classes.
 */
typedef enum {
    TTFP_GLYPH_CLASS_UNKNOWN = 0,
    TTFP_GLYPH_CLASS_BASE,
    TTFP_GLYPH_CLASS_LIGATURE,
    TTFP_GLYPH_CLASS_MARK,
    TTFP_GLYPH_CLASS_COMPONENT,
} ttfp_glyph_class;

/**
 * @brief A glyph image format.
 */
typedef enum {
    TTFP_IMAGE_FORMAT_PNG = 0,
    TTFP_IMAGE_FORMAT_JPEG,
    TTFP_IMAGE_FORMAT_TIFF,
    TTFP_IMAGE_FORMAT_SVG,
} ttfp_image_format;

/**
 * @brief An opaque pointer to the font structure.
 */
typedef struct ttfp_font ttfp_font;

/**
 * @brief A name record.
 *
 * https://docs.microsoft.com/en-us/typography/opentype/spec/name#name-records
 */
typedef struct {
    uint16_t platform_id;
    uint16_t encoding_id;
    uint16_t language_id;
    uint16_t name_id;
    uint16_t name_size;
} ttfp_name_record;

/**
 * @brief A line metrics.
 *
 * Used for underline and strikeout.
 */
typedef struct {
    int16_t position;
    int16_t thickness;
} ttfp_line_metrics;

/**
 * @brief A script metrics used by subscript and superscript.
 */
typedef struct {
    int16_t x_size;
    int16_t y_size;
    int16_t x_offset;
    int16_t y_offset;
} ttfp_script_metrics;

/**
 * @brief An outline building interface.
 */
typedef struct {
    void (*move_to)(float x, float y, void *data);
    void (*line_to)(float x, float y, void *data);
    void (*quad_to)(float x1, float y1, float x, float y, void *data);
    void (*curve_to)(float x1, float y1, float x2, float y2, float x, float y, void *data);
    void (*close_path)(void *data);
} ttfp_outline_builder;

/**
 * @brief A rectangle.
 */
typedef struct {
    int16_t x_min;
    int16_t y_min;
    int16_t x_max;
    int16_t y_max;
} ttfp_rect;

/**
 * @brief A glyph image.
 *
 * An image offset and size isn't defined in all tables, so `x`, `y`, `width` and `height`
 * can be set to 0.
 */
typedef struct {
    /**
     * Horizontal offset.
     */
    int16_t x;
    /**
     * Vertical offset.
     */
    int16_t y;
    /**
     * Image width.
     *
     * It doesn't guarantee that this value is the same as set in the `data`.
     */
    uint16_t width;
    /**
     * Image height.
     *
     * It doesn't guarantee that this value is the same as set in the `data`.
     */
    uint16_t height;
    /**
     * A pixels per em of the selected strike.
     */
    uint16_t pixels_per_em;
    /**
     * An image format.
     */
    ttfp_image_format format;
    /**
     * A raw image data as is. It's up to the caller to decode PNG, JPEG, etc.
     */
    const char *data;
    /**
     * A raw image data size.
     */
    uint32_t len;
} ttfp_glyph_image;

/**
 * A 4-byte tag.
 */
typedef uint32_t ttfp_tag;

#define TTFP_TAG(c1,c2,c3,c4) \
    ((ttfp_tag)((((uint32_t)(c1)&0xFF)<<24)| \
    (((uint32_t)(c2)&0xFF)<<16)| \
    (((uint32_t)(c3)&0xFF)<<8)| \
    ((uint32_t)(c4)&0xFF)))

/**
 * @brief A variation axis.
 */
typedef struct {
    ttfp_tag tag;
    float min_value;
    float def_value;
    float max_value;
    /**< An axis name in the `name` table. */
    uint16_t name_id;
    bool hidden;
} ttfp_variation_axis;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Returns the number of fonts stored in a TrueType font collection.
 *
 * @param data The font data.
 * @param len The size of the font data.
 * @return Number of fonts or -1 when provided data is not a TrueType font collection
 *         or when number of fonts is larger than INT_MAX.
 */
int32_t ttfp_fonts_in_collection(const char *data, uintptr_t len);

/**
 * @brief Creates a new font parser.
 *
 * This is the only heap allocation in the library.
 *
 * @param data The font data. Must outlive the #ttfp_font.
 * @param len The size of the font data.
 * @param index The font index in a collection (typically *.ttc). 0 should be used for basic fonts.
 * @return Font handle or NULL on error.
 */
ttfp_font *ttfp_create_font(const char *data, uintptr_t len, uint32_t index);

/**
 * @brief Destroys the #ttfp_font.
 */
void ttfp_destroy_font(ttfp_font *font);

/**
 * @brief Checks that font has a specified table.
 *
 * @return `true` only for tables that were successfully parsed.
 */
bool ttfp_has_table(const ttfp_font *font, ttfp_table_name name);

/**
 * @brief Returns the number of name records in the font.
 */
uint16_t ttfp_get_name_records_count(const ttfp_font *font);

/**
 * @brief Returns a name record.
 *
 * @param Record's index. The total amount can be obtained via #ttfp_get_name_records_count.
 * @return `false` when `index` is out of range or `platform_id` is invalid.
 */
bool ttfp_get_name_record(const ttfp_font *font, uint16_t index, ttfp_name_record *record);

/**
 * @brief Returns a name record's string.
 *
 * @param index Record's index.
 * @param name A string buffer that will be filled with the record's name.
 *             Remember that a name will use encoding specified in `ttfp_name_record.encoding_id`
 *             Because of that, the name will not be null-terminated.
 * @param len The size of a string buffer. Must be equal to `ttfp_name_record.name_size`.
 * @return `false` when `index` is out of range or string buffer is not equal
 *         `ttfp_name_record.name_size`.
 */
bool ttfp_get_name_record_string(const ttfp_font *font, uint16_t index, char *name, uintptr_t len);

/**
 * @brief Checks that font is marked as *Regular*.
 *
 * @return `false` when OS/2 table is not present.
 */
bool ttfp_is_regular(const ttfp_font *font);

/**
 * @brief Checks that font is marked as *Italic*.
 *
 * @return `false` when OS/2 table is not present.
 */
bool ttfp_is_italic(const ttfp_font *font);

/**
 * @brief Checks that font is marked as *Bold*.
 *
 * @return `false` when OS/2 table is not present.
 */
bool ttfp_is_bold(const ttfp_font *font);

/**
 * @brief Checks that font is marked as *Oblique*.
 *
 * @return `false` when OS/2 table is not present.
 */
bool ttfp_is_oblique(const ttfp_font *font);

/**
 * @brief Checks that font is variable.
 *
 * Simply checks the presence of a `fvar` table.
 */
bool ttfp_is_variable(const ttfp_font *font);

/**
 * @brief Returns font's weight.
 *
 * @return Font's weight or `400` when OS/2 table is not present.
 */
uint16_t ttfp_get_weight(const ttfp_font *font);

/**
 * @brief Returns font's width.
 *
 * @return Font's width in a 1..9 range or `5` when OS/2 table is not present
 *         or when value is invalid.
 */
uint16_t ttfp_get_width(const ttfp_font *font);

/**
 * @brief Returns a horizontal font ascender.
 *
 * This function is affected by variation axes.
 */
int16_t ttfp_get_ascender(const ttfp_font *font);

/**
 * @brief Returns a horizontal font descender.
 *
 * This function is affected by variation axes.
 */
int16_t ttfp_get_descender(const ttfp_font *font);

/**
 * @brief Returns a horizontal font height.
 *
 * This function is affected by variation axes.
 */
int16_t ttfp_get_height(const ttfp_font *font);

/**
 * @brief Returns a horizontal font line gap.
 *
 * This function is affected by variation axes.
 */
int16_t ttfp_get_line_gap(const ttfp_font *font);

/**
 * @brief Returns a vertical font ascender.
 *
 * This function is affected by variation axes.
 *
 * @return `0` when `vhea` table is not present.
 */
int16_t ttfp_get_vertical_ascender(const ttfp_font *font);

/**
 * @brief Returns a vertical font descender.
 *
 * This function is affected by variation axes.
 *
 * @return `0` when `vhea` table is not present.
 */
int16_t ttfp_get_vertical_descender(const ttfp_font *font);

/**
 * @brief Returns a vertical font height.
 *
 * This function is affected by variation axes.
 *
 * @return `0` when `vhea` table is not present.
 */
int16_t ttfp_get_vertical_height(const ttfp_font *font);

/**
 * @brief Returns a vertical font line gap.
 *
 * This function is affected by variation axes.
 *
 * @return `0` when `vhea` table is not present.
 */
int16_t ttfp_get_vertical_line_gap(const ttfp_font *font);

/**
 * @brief Returns font's units per EM.
 *
 * @return Units in a 16..16384 range or `0` otherwise.
 */
uint16_t ttfp_get_units_per_em(const ttfp_font *font);

/**
 * @brief Returns font's x height.
 *
 * This function is affected by variation axes.
 *
 * @return x height or 0 when OS/2 table is not present or when its version is < 2.
 */
int16_t ttfp_get_x_height(const ttfp_font *font);

/**
 * @brief Returns font's underline metrics.
 *
 * This function is affected by variation axes.
 *
 * @return `false` when `post` table is not present.
 */
bool ttfp_get_underline_metrics(const ttfp_font *font, ttfp_line_metrics *metrics);

/**
 * @brief Returns font's strikeout metrics.
 *
 * This function is affected by variation axes.
 *
 * @return `false` when OS/2 table is not present.
 */
bool ttfp_get_strikeout_metrics(const ttfp_font *font, ttfp_line_metrics *metrics);

/**
 * @brief Returns font's subscript metrics.
 *
 * This function is affected by variation axes.
 *
 * @return `false` when OS/2 table is not present.
 */
bool ttfp_get_subscript_metrics(const ttfp_font *font, ttfp_script_metrics *metrics);

/**
 * @brief Returns font's superscript metrics.
 *
 * This function is affected by variation axes.
 *
 * @return `false` when OS/2 table is not present.
 */
bool ttfp_get_superscript_metrics(const ttfp_font *font, ttfp_script_metrics *metrics);

/**
 * @brief Returns a total number of glyphs in the font.
 *
 * @return The number of glyphs which is never zero.
 */
uint16_t ttfp_get_number_of_glyphs(const ttfp_font *font);

/**
 * @brief Resolves a Glyph ID for a code point.
 *
 * All subtable formats except Mixed Coverage (8) are supported.
 *
 * @param codepoint A valid Unicode codepoint. Otherwise 0 will be returned.
 * @return Returns 0 when glyph is not present or parsing is failed.
 */
uint16_t ttfp_get_glyph_index(const ttfp_font *font, uint32_t codepoint);

/**
 * @brief Resolves a variation of a Glyph ID from two code points.
 *
 * @param codepoint A valid Unicode codepoint. Otherwise 0 will be returned.
 * @param variation A valid Unicode codepoint. Otherwise 0 will be returned.
 * @return Returns 0 when glyph is not present or parsing is failed.
 */
uint16_t ttfp_get_glyph_var_index(const ttfp_font *font, uint32_t codepoint, uint32_t variation);

/**
 * @brief Returns glyph's horizontal advance.
 *
 * @return Glyph's advance or 0 when not set.
 */
uint16_t ttfp_get_glyph_hor_advance(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's vertical advance.
 *
 * This function is affected by variation axes.
 *
 * @return Glyph's advance or 0 when not set.
 */
uint16_t ttfp_get_glyph_ver_advance(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's horizontal side bearing.
 *
 * @return Glyph's side bearing or 0 when not set.
 */
int16_t ttfp_get_glyph_hor_side_bearing(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's vertical side bearing.
 *
 * This function is affected by variation axes.
 *
 * @return Glyph's side bearing or 0 when not set.
 */
int16_t ttfp_get_glyph_ver_side_bearing(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's vertical origin.
 *
 * @return Glyph's vertical origin or 0 when not set.
 */
int16_t ttfp_get_glyph_y_origin(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's name.
 *
 * Uses the `post` table as a source.
 *
 * A glyph name cannot be larger than 255 bytes + 1 byte for '\0'.
 *
 * @param name A char buffer longer than 256 bytes.
 * @return `true` on success.
 */
bool ttfp_get_glyph_name(const ttfp_font *font, uint16_t glyph_id, char *name);

/**
 * @brief Returns glyph's class according to Glyph Class Definition Table.
 *
 * @return A glyph class or TTFP_GLYPH_CLASS_UNKNOWN otherwise.
 */
ttfp_glyph_class ttfp_get_glyph_class(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Returns glyph's mark attachment class according to Mark Attachment Class Definition Table.
 *
 * @return All glyphs not assigned to a class fall into Class 0.
 */
uint16_t ttfp_get_glyph_mark_attachment_class(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Checks that glyph is a mark according to Mark Glyph Sets Table.
 */
bool ttfp_is_mark_glyph(const ttfp_font *font, uint16_t glyph_id);

/**
 * @brief Outlines a glyph and returns its tight bounding box.
 *
 * **Warning**: since `ttf-parser` is a pull parser,
 * `OutlineBuilder` will emit segments even when outline is partially malformed.
 * You must check #ttfp_outline_glyph() result before using
 * #ttfp_outline_builder 's output.
 *
 * `glyf`, `gvar`, `CFF` and `CFF2` tables are supported.
 *
 * This function is affected by variation axes.
 *
 * @return `false` when glyph has no outline or on error.
 */
bool ttfp_outline_glyph(const ttfp_font *font,
                        ttfp_outline_builder builder,
                        void *user_data,
                        uint16_t glyph_id,
                        ttfp_rect *bbox);

/**
 * @brief Returns a tight glyph bounding box.
 *
 * Unless the current font has a `glyf` table, this is just a shorthand for `outline_glyph()`
 * since only the `glyf` table stores a bounding box. In case of CFF and variable fonts
 * we have to actually outline a glyph to find it's bounding box.
 *
 * This function is affected by variation axes.
 */
bool ttfp_get_glyph_bbox(const ttfp_font *font, uint16_t glyph_id, ttfp_rect *bbox);

/**
 * @brief Returns a reference to a glyph image.
 *
 * A font can define a glyph using a raster or a vector image instead of a simple outline.
 * Which is primarily used for emojis. This method should be used to access those images.
 *
 * `pixels_per_em` allows selecting a preferred image size. While the chosen size will
 * be closer to an upper one. So when font has 64px and 96px images and `pixels_per_em`
 * is set to 72, 96px image will be returned.
 * To get the largest image simply use `SHRT_MAX`.
 * This property has no effect in case of SVG.
 *
 * Note that this method will return an encoded image. It should be decoded
 * (in case of PNG, JPEG, etc.), rendered (in case of SVG) or even decompressed
 * (in case of SVGZ) by the caller. We don't validate or preprocess it in any way.
 *
 * Also, a font can contain both: images and outlines. So when this method returns `None`
 * you should also try `ttfp_outline_glyph()` afterwards.
 *
 * There are multiple ways an image can be stored in a TrueType font
 * and we support `sbix`, `CBLC`+`CBDT` and `SVG`.
 */
bool ttfp_get_glyph_image(const ttfp_font *font,
                          uint16_t glyph_id,
                          uint16_t pixels_per_em,
                          ttfp_glyph_image *glyph_image);

/**
 * @brief Returns the amount of variation axes.
 */
uint16_t ttfp_get_variation_axes_count(const ttfp_font *font);

/**
 * @brief Returns a variation axis by index.
 */
bool ttfp_get_variation_axis(const ttfp_font *font, uint16_t index, ttfp_variation_axis *axis);

/**
 * @brief Returns a variation axis by tag.
 */
bool ttfp_get_variation_axis_by_tag(const ttfp_font *font, ttfp_tag tag, ttfp_variation_axis *axis);

/**
 * @brief Sets a variation axis coordinate.
 *
 * This is the only mutable function in the library.
 * We can simplify the API a lot by storing the variable coordinates
 * in the font object itself.
 *
 * This function is reentrant.
 *
 * Since coordinates are stored on the stack, we allow only 32 of them.
 *
 * @return `false` when font is not variable or doesn't have such axis.
 */
bool ttfp_set_variation(ttfp_font *font, ttfp_tag axis, float value);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* TTFP_H */
