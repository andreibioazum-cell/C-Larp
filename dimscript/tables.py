"""Language tables: types, built-in functions, engine variables, namespaces.

Everything the compiler knows about names that are not declared in a script
lives here, so adding a native function or an engine variable is a one-line
change in this file."""


TYPES = {
    'num': 'double',
    'number': 'double',
    'int': 'double',
    'float': 'double',
    'double': 'double',
    'str': 'const char*',
    'string': 'const char*',
    'bool': 'double',
    'col': 'uint32_t',
    'color': 'uint32_t',
    'arr': 'DSArray*',
    'array': 'DSArray*',
}

_TYPE_ALIAS = {
    'number': 'num',
    'int': 'num',
    'float': 'num',
    'double': 'num',
    'string': 'str',
    'bool': 'num',
    'color': 'col',
    'array': 'arr',
}


def canon_type(t):
    return _TYPE_ALIAS.get(t, t)


BUILTINS = frozenset({
    'rect', 'roundrect', 'rect_rot', 'circle', 'ring', 'line', 'tex',
    'tex_tint', 'text', 'text_scaled', 'text_ink_width', 'text_ink_height',
    'text_ink_top', 'png_load', 'clear_screen', 'text_width', 'text_height',
    'sqrt', 'sin', 'cos', 'atan2', 'floor', 'rand', 'snd_load', 'snd_play',
    'snd_loop', 'snd_stop', 'snd_playing', 'snd_volume', 'snd_stop_all',
    'sound_play', 'keyboard_show', 'keyboard_hide', 'keyboard_get_text',
    'keyboard_get_raw', 'keyboard_clear', 'keyboard_enter_pressed',
    'keyboard_type', 'keyboard_visible', 'str_len', 'str_eq', 'str_contains',
    'str_index_of', 'str_sub', 'str_to_num', 'str_trim', 'str_starts_with',
    'str_ends_with', 'str_lower', 'str_upper', 'ds_log', 'console_count',
    'console_line', 'console_type', 'console_clear', 'arr_new', 'arr_push',
    'arr_get', 'arr_set', 'arr_len', 'arr_clear', 'clamp', 'lerp', 'dist',
    # Maths for scripts. In C those names carry the ds_ prefix (see FUNCTION_MAP
    # and native/runtime/core.inc) so they do not clash with libc, where abs(),
    # round() and the like already exist with other signatures.
    'min', 'max', 'abs', 'round', 'sign', 'mod', 'trunc',
})

# Script names to C names. Empty means the name is the same.
FUNCTION_MAP = {
    'min': 'ds_min',
    'max': 'ds_max',
    'abs': 'ds_abs',
    'round': 'ds_round',
    'sign': 'ds_sign',
    'mod': 'ds_mod',
    'trunc': 'ds_trunc',
}

ENGINE_VARS = {
    'screen_w': 'num',
    'screen_h': 'num',
    'dt': 'num',
    'joy': 'joy',
    'mouse_clicked': 'num',
    'ds_mouse_x': 'num',
    'ds_mouse_y': 'num',
}

STR_BUILTINS = frozenset({
    'console_line',
    'keyboard_get_text',
    'keyboard_get_raw',
    'str_sub',
    'str_trim',
    'str_lower',
    'str_upper',
})

# Built-in namespaces: import Dim.System.Graphics allows Graphics.rect(...),
# while the short rect(...) calls keep working.
STD_NAMESPACES = {
    'Dim.System.Graphics': 'Graphics',
    'Dim.System.Math': 'Math',
}
_MATH_NS = {'min', 'max', 'abs', 'round', 'sign', 'mod', 'trunc', 'clamp',
            'lerp', 'dist', 'sqrt', 'sin', 'cos', 'atan2', 'floor', 'rand'}
# Maths from math.h, which the generated game.c includes.
NATIVE_MATH = frozenset({'fabs'})
_MODS = ('public', 'private')
