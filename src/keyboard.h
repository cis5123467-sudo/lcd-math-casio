#ifndef LCD_KEYBOARD_H
#define LCD_KEYBOARD_H
#include <stdbool.h>
#include <stddef.h>
#include <gint/keycodes.h>
typedef enum {
    KA_NONE,KA_TEXT,KA_FRACTION,KA_MIXED,KA_ROOT,KA_LOGBASE,KA_HOME,KA_ON,KA_OFF,
    KA_SETTINGS,KA_BACK,KA_OK,KA_EXE,KA_DECIMAL_EXE,KA_LEFT,KA_RIGHT,KA_UP,KA_DOWN,
    KA_PREVTAB,KA_NEXTTAB,KA_PAGEUP,KA_PAGEDOWN,KA_VARIABLE,KA_CATALOG,KA_TOOLS,
    KA_FORMAT,KA_DELETE,KA_CLEAR,KA_INSERT,KA_SELECT,KA_PASTE,KA_CAPTURE,KA_MODIFIER
} KeyActionKind;
typedef struct {KeyActionKind kind;const char *text;} KeyAction;
typedef struct {int code;const char *label,*shift_label,*alpha_label;KeyAction primary,shift;const char *alpha;} KeyBinding;
typedef struct {bool shift,alpha,alpha_lock;} KeyboardState;
extern const KeyBinding graph_math_keys[];
extern const size_t graph_math_key_count;
KeyAction keyboard_decode(KeyboardState *state,int key);
const KeyBinding *keyboard_binding(int key);
int keyboard_event_code(int key, int row, int col);
int keyboard_menu_option(int key,bool shift,bool alpha);
void keyboard_option_label(int index,char label[3]);
#endif
