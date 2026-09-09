type T is int256;
function f(T a, T b) pure returns (T) {}
function g() pure returns (T) {}
using {f as -, g as -} for T global;
// ----
// TypeError 4731: (107-108): The function "g" does not have any parameters, and therefore cannot be attached to the type "T".
