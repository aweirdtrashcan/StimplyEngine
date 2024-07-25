#pragma once

// These are basically copy and paste from SDL2
// Just wrapping around a new enum so i don't expose
// SDL2 to the whole engine.
enum class Key {
	Key_A = 4,
    Key_B = 5,
    Key_C = 6,
    Key_D = 7,
    Key_E = 8,
    Key_F = 9,
    Key_G = 10,
    Key_H = 11,
    Key_I = 12,
    Key_J = 13,
    Key_K = 14,
    Key_L = 15,
    Key_M = 16,
    Key_N = 17,
    Key_O = 18,
    Key_P = 19,
    Key_Q = 20,
    Key_R = 21,
    Key_S = 22,
    Key_T = 23,
    Key_U = 24,
    Key_V = 25,
    Key_W = 26,
    Key_X = 27,
    Key_Y = 28,
    Key_Z = 29,

    Key_1 = 30,
    Key_2 = 31,
    Key_3 = 32,
    Key_4 = 33,
    Key_5 = 34,
    Key_6 = 35,
    Key_7 = 36,
    Key_8 = 37,
    Key_9 = 38,
    Key_0 = 39,

    Key_RETURN = 40,
    Key_ESCAPE = 41,
    Key_BACKSPACE = 42,
    Key_TAB = 43,
    Key_SPACE = 44,

    Key_MINUS = 45,
    Key_EQUALS = 46,
    Key_LEFTBRACKET = 47,
    Key_RIGHTBRACKET = 48,
    Key_BACKSLASH = 49,
    Key_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate Key_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    Key_SEMICOLON = 51,
    Key_APOSTROPHE = 52,
    Key_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    Key_COMMA = 54,
    Key_PERIOD = 55,
    Key_SLASH = 56,

    Key_CAPSLOCK = 57,

    Key_F1 = 58,
    Key_F2 = 59,
    Key_F3 = 60,
    Key_F4 = 61,
    Key_F5 = 62,
    Key_F6 = 63,
    Key_F7 = 64,
    Key_F8 = 65,
    Key_F9 = 66,
    Key_F10 = 67,
    Key_F11 = 68,
    Key_F12 = 69,

    Key_PRINTSCREEN = 70,
    Key_SCROLLLOCK = 71,
    Key_PAUSE = 72,
    Key_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    Key_HOME = 74,
    Key_PAGEUP = 75,
    Key_DELETE = 76,
    Key_END = 77,
    Key_PAGEDOWN = 78,
    Key_RIGHT = 79,
    Key_LEFT = 80,
    Key_DOWN = 81,
    Key_UP = 82,

    Key_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    Key_KP_DIVIDE = 84,
    Key_KP_MULTIPLY = 85,
    Key_KP_MINUS = 86,
    Key_KP_PLUS = 87,
    Key_KP_ENTER = 88,
    Key_KP_1 = 89,
    Key_KP_2 = 90,
    Key_KP_3 = 91,
    Key_KP_4 = 92,
    Key_KP_5 = 93,
    Key_KP_6 = 94,
    Key_KP_7 = 95,
    Key_KP_8 = 96,
    Key_KP_9 = 97,
    Key_KP_0 = 98,
    Key_KP_PERIOD = 99,

    Key_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    Key_APPLICATION = 101, /**< windows contextual menu, compose */
    Key_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    Key_KP_EQUALS = 103,
    Key_F13 = 104,
    Key_F14 = 105,
    Key_F15 = 106,
    Key_F16 = 107,
    Key_F17 = 108,
    Key_F18 = 109,
    Key_F19 = 110,
    Key_F20 = 111,
    Key_F21 = 112,
    Key_F22 = 113,
    Key_F23 = 114,
    Key_F24 = 115,
    Key_EXECUTE = 116,
    Key_HELP = 117,    /**< AL Integrated Help Center */
    Key_MENU = 118,    /**< Menu (show menu) */
    Key_SELECT = 119,
    Key_STOP = 120,    /**< AC Stop */
    Key_AGAIN = 121,   /**< AC Redo/Repeat */
    Key_UNDO = 122,    /**< AC Undo */
    Key_CUT = 123,     /**< AC Cut */
    Key_COPY = 124,    /**< AC Copy */
    Key_PASTE = 125,   /**< AC Paste */
    Key_FIND = 126,    /**< AC Find */
    Key_MUTE = 127,
    Key_VOLUMEUP = 128,
    Key_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     Key_LOCKINGCAPSLOCK = 130,  */
/*     Key_LOCKINGNUMLOCK = 131, */
/*     Key_LOCKINGSCROLLLOCK = 132, */
    Key_KP_COMMA = 133,
    Key_KP_EQUALSAS400 = 134,

    Key_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    Key_INTERNATIONAL2 = 136,
    Key_INTERNATIONAL3 = 137, /**< Yen */
    Key_INTERNATIONAL4 = 138,
    Key_INTERNATIONAL5 = 139,
    Key_INTERNATIONAL6 = 140,
    Key_INTERNATIONAL7 = 141,
    Key_INTERNATIONAL8 = 142,
    Key_INTERNATIONAL9 = 143,
    Key_LANG1 = 144, /**< Hangul/English toggle */
    Key_LANG2 = 145, /**< Hanja conversion */
    Key_LANG3 = 146, /**< Katakana */
    Key_LANG4 = 147, /**< Hiragana */
    Key_LANG5 = 148, /**< Zenkaku/Hankaku */
    Key_LANG6 = 149, /**< reserved */
    Key_LANG7 = 150, /**< reserved */
    Key_LANG8 = 151, /**< reserved */
    Key_LANG9 = 152, /**< reserved */

    Key_ALTERASE = 153,    /**< Erase-Eaze */
    Key_SYSREQ = 154,
    Key_CANCEL = 155,      /**< AC Cancel */
    Key_CLEAR = 156,
    Key_PRIOR = 157,
    Key_RETURN2 = 158,
    Key_SEPARATOR = 159,
    Key_OUT = 160,
    Key_OPER = 161,
    Key_CLEARAGAIN = 162,
    Key_CRSEL = 163,
    Key_EXSEL = 164,

    Key_KP_00 = 176,
    Key_KP_000 = 177,
    Key_THOUSANDSSEPARATOR = 178,
    Key_DECIMALSEPARATOR = 179,
    Key_CURRENCYUNIT = 180,
    Key_CURRENCYSUBUNIT = 181,
    Key_KP_LEFTPAREN = 182,
    Key_KP_RIGHTPAREN = 183,
    Key_KP_LEFTBRACE = 184,
    Key_KP_RIGHTBRACE = 185,
    Key_KP_TAB = 186,
    Key_KP_BACKSPACE = 187,
    Key_KP_A = 188,
    Key_KP_B = 189,
    Key_KP_C = 190,
    Key_KP_D = 191,
    Key_KP_E = 192,
    Key_KP_F = 193,
    Key_KP_XOR = 194,
    Key_KP_POWER = 195,
    Key_KP_PERCENT = 196,
    Key_KP_LESS = 197,
    Key_KP_GREATER = 198,
    Key_KP_AMPERSAND = 199,
    Key_KP_DBLAMPERSAND = 200,
    Key_KP_VERTICALBAR = 201,
    Key_KP_DBLVERTICALBAR = 202,
    Key_KP_COLON = 203,
    Key_KP_HASH = 204,
    Key_KP_SPACE = 205,
    Key_KP_AT = 206,
    Key_KP_EXCLAM = 207,
    Key_KP_MEMSTORE = 208,
    Key_KP_MEMRECALL = 209,
    Key_KP_MEMCLEAR = 210,
    Key_KP_MEMADD = 211,
    Key_KP_MEMSUBTRACT = 212,
    Key_KP_MEMMULTIPLY = 213,
    Key_KP_MEMDIVIDE = 214,
    Key_KP_PLUSMINUS = 215,
    Key_KP_CLEAR = 216,
    Key_KP_CLEARENTRY = 217,
    Key_KP_BINARY = 218,
    Key_KP_OCTAL = 219,
    Key_KP_DECIMAL = 220,
    Key_KP_HEXADECIMAL = 221,

    Key_LCTRL = 224,
    Key_LSHIFT = 225,
    Key_LALT = 226, /**< alt, option */
    Key_LGUI = 227, /**< windows, command (apple), meta */
    Key_RCTRL = 228,
    Key_RSHIFT = 229,
    Key_RALT = 230, /**< alt gr, option */
    Key_RGUI = 231, /**< windows, command (apple), meta */

    Key_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     *  See https://usb.org/sites/default/files/hut1_2.pdf
     *
     *  There are way more keys in the spec than we can represent in the
     *  current scancode range, so pick the ones that commonly come up in
     *  real world usage.
     */
    /* @{ */

    Key_AUDIONEXT = 258,
    Key_AUDIOPREV = 259,
    Key_AUDIOSTOP = 260,
    Key_AUDIOPLAY = 261,
    Key_AUDIOMUTE = 262,
    Key_MEDIASELECT = 263,
    Key_WWW = 264,             /**< AL Internet Browser */
    Key_MAIL = 265,
    Key_CALCULATOR = 266,      /**< AL Calculator */
    Key_COMPUTER = 267,
    Key_AC_SEARCH = 268,       /**< AC Search */
    Key_AC_HOME = 269,         /**< AC Home */
    Key_AC_BACK = 270,         /**< AC Back */
    Key_AC_FORWARD = 271,      /**< AC Forward */
    Key_AC_STOP = 272,         /**< AC Stop */
    Key_AC_REFRESH = 273,      /**< AC Refresh */
    Key_AC_BOOKMARKS = 274,    /**< AC Bookmarks */

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    Key_BRIGHTNESSDOWN = 275,
    Key_BRIGHTNESSUP = 276,
    Key_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    Key_KBDILLUMTOGGLE = 278,
    Key_KBDILLUMDOWN = 279,
    Key_KBDILLUMUP = 280,
    Key_EJECT = 281,
    Key_SLEEP = 282,           /**< SC System Sleep */

    Key_APP1 = 283,
    Key_APP2 = 284,

    /* @} *//* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    Key_AUDIOREWIND = 285,
    Key_AUDIOFASTFORWARD = 286,

    /* @} *//* Usage page 0x0C (additional media keys) */

    /**
     *  \name Mobile keys
     *
     *  These are values that are often used on mobile phones.
     */
    /* @{ */

    Key_SOFTLEFT = 287, /**< Usually situated below the display on phones and
                                      used as a multi-function feature key for selecting
                                      a software defined function shown on the bottom left
                                      of the display. */
    Key_SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom right
                                       of the display. */
    Key_CALL = 289, /**< Used for accepting phone calls. */
    Key_ENDCALL = 290, /**< Used for rejecting phone calls. */

    /* @} *//* Mobile keys */

    /* Add any other keys here. */

    SDL_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
};