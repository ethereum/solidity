using {add as +} for * global;

function add(int, int) returns (int) {}
// ----
// SyntaxError 8118: (0-30): The type has to be specified explicitly at file level (cannot use '*').
// SyntaxError 2854: (0-30): Can only globally attach functions to specific types.
