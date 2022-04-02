function id(uint x) pure returns (uint) {
    return x;
}

using {id} for *;
// ----
// SyntaxError 8118: (59-76='using {id} for *;'): The type has to be specified explicitly at file level (cannot use '*').
