using {add as +} for *;

function add(int, int) returns (int) {}
// ----
// SyntaxError 8118: (0-23): The type has to be specified explicitly at file level (cannot use '*').
