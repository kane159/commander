#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#define HAS_MENU_VOLUME
#define HAS_MENU_BRIGHTNESS
//#define HAS_MENU_SAVE
//#define HAS_MENU_LOAD
//#define HAS_MENU_ASPECT_RATIO
//#define HAS_MENU_USB
//#define HAS_MENU_THEME
//#define HAS_MENU_LAUNCHER
#define HAS_MENU_EXIT
//#define HAS_MENU_POWERDOWN
#define HAS_MENU_RO_RW

typedef enum{
    MENU_TYPE_VOLUME,
    MENU_TYPE_BRIGHTNESS,
    MENU_TYPE_SAVE,
    MENU_TYPE_LOAD,
    MENU_TYPE_ASPECT_RATIO,
    MENU_TYPE_USB,
    MENU_TYPE_THEME,
    MENU_TYPE_LAUNCHER,
    MENU_TYPE_EXIT,
    MENU_TYPE_POWERDOWN,
    MENU_TYPE_RO_RW,
    NB_MENU_TYPES,
} ENUM_MENU_TYPE;

typedef enum{
    MENU_RETURN_OK,
    MENU_RETURN_EXIT,
    MENU_RETURN_ERROR,
    NB_MENU_RETURN_CODES,
} ENUM_MENU_RETURN_CODES;

#ifdef HAS_MENU_ASPECT_RATIO
///------ Definition of the different aspect ratios
#define ASPECT_RATIOS \
    X(ASPECT_RATIOS_TYPE_MANUAL, "MANUAL ZOOM") \
    X(ASPECT_RATIOS_TYPE_STRECHED, "STRECHED") \
    X(ASPECT_RATIOS_TYPE_CROPPED, "CROPPED") \
    X(ASPECT_RATIOS_TYPE_SCALED, "SCALED") \
    X(NB_ASPECT_RATIOS_TYPES, "")

////------ Enumeration of the different aspect ratios ------
#undef X
#define X(a, b) a,
typedef enum {ASPECT_RATIOS} ENUM_ASPECT_RATIOS_TYPES;
#endif

////------ Defines to be shared -------
#ifdef HAS_MENU_VOLUME
#define STEP_CHANGE_VOLUME          10
#endif
#ifdef HAS_MENU_BRIGHTNESS
#define STEP_CHANGE_BRIGHTNESS      10
#endif

////------ Menu commands -------
#ifdef HAS_MENU_VOLUME
#define SHELL_CMD_VOLUME_GET                "volume_get"
#define SHELL_CMD_VOLUME_SET                "volume_set"
#endif
#ifdef HAS_MENU_BRIGHTNESS
#define SHELL_CMD_BRIGHTNESS_GET            "brightness_get"
#define SHELL_CMD_BRIGHTNESS_SET            "brightness_set"
#endif
#ifdef HAS_MENU_USB
#define SHELL_CMD_USB_DATA_CONNECTED        "is_usb_data_connected"
#define SHELL_CMD_USB_MOUNT                 "share start"
#define SHELL_CMD_USB_UNMOUNT               "share stop"
#define SHELL_CMD_USB_CHECK_IS_SHARING      "share is_sharing"
#endif
#ifdef HAS_MENU_POWERDOWN
#define SHELL_CMD_POWERDOWN                 "shutdown_funkey"
#define SHELL_CMD_SCHEDULE_POWERDOWN        "sched_shutdown"
#define SHELL_CMD_CANCEL_SCHED_POWERDOWN    "cancel_sched_powerdown"
#endif
#ifdef HAS_MENU_LAUNCHER
#define SHELL_CMD_SET_LAUNCHER_GMENU2X      "set_launcher gmenu2x"
#define SHELL_CMD_SET_LAUNCHER_RETROFE      "set_launcher retrofe"
#endif
#ifdef HAS_MENU_RO_RW
#define SHELL_CMD_RO                        "ro"
#define SHELL_CMD_RW                        "rw"
#endif

class Menu
{

public:
#ifdef HAS_MENU_THEME
    static void     init(Configuration &c);
#else
  static void     init(void);
#endif
    static void     end(void);
    static int      run(SDL_Surface *screen);
    static void     stop(void);

private:
#if defined(HAS_MENU_VOLUME) || defined(HAS_MENU_BRIGHTNESS)
    static void draw_progress_bar(SDL_Surface * surface, uint16_t x, uint16_t y, uint16_t width,
            uint16_t height, uint8_t percentage, uint16_t nb_bars);
#endif
    static void add_menu_zone(ENUM_MENU_TYPE menu_type);
    static void init_menu_zones(void);
    static void init_menu_system_values(void);
    static void menu_screen_refresh(SDL_Surface *screen, int menuItem, int prevItem, int scroll, uint8_t menu_confirmation, uint8_t menu_action);

    static SDL_Surface * draw_screen;

    static int backup_key_repeat_delay;
	static int backup_key_repeat_interval;
    static SDL_Surface *background_screen;

    static TTF_Font *menu_title_font;
    static TTF_Font *menu_info_font;
    static TTF_Font *menu_small_info_font;
    static SDL_Surface ** menu_zone_surfaces;
    static int * idx_menus;
    static int nb_menu_zones;
    static int menuItem;

    static int stop_menu_loop;

    static SDL_Color text_color;
    static int padding_y_from_center_menu_zone;
    static uint16_t width_progress_bar;
    static uint16_t height_progress_bar;
#ifdef HAS_MENU_VOLUME
    static uint16_t x_volume_bar;
    static uint16_t y_volume_bar;
    static int volume_percentage;
#endif
#ifdef HAS_MENU_BRIGHTNESS
    static uint16_t x_brightness_bar;
    static uint16_t y_brightness_bar;
    static int brightness_percentage;
#endif

#ifdef HAS_MENU_ASPECT_RATIO
    static const char *aspect_ratio_name[];
    static int aspect_ratio;
    static int aspect_ratio_factor_percent;
    static int aspect_ratio_factor_step;
#endif

#if defined(HAS_MENU_SAVE) || defined (HAS_MENU_LOAD)
    static int savestate_slot;
#endif

#ifdef HAS_MENU_THEME
    static Configuration *config;
    static int indexChooseLayout;
#endif
};
