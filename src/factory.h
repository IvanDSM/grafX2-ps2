/* vim:expandtab:ts=2 sw=2:
*/
#ifndef FACTORY_H__
#define FACTORY_H__

void Button_Brush_Factory(int);
void Repeat_script(void);

/// Lua scripts bound to shortcut keys.
extern char * Bound_script[10];

///
/// Run a lua script linked to a shortcut, 0-9.
/// Before: Cursor hidden
/// After: Cursor shown
void Run_numbered_script(byte index);

///
/// Returns a string stating the included Lua engine version,
/// or "Disabled" if Grafx2 is compiled without Lua.
const char * Lua_version(void);

#endif
