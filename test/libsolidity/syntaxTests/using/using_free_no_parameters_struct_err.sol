struct S { uint8 x; }
function f() {}
using {f} for S;
// ----
// TypeError 4731: (45-46): The function "f" does not have any parameters, and therefore cannot be attached to the type "struct S".
