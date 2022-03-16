function f(uint8 x) pure returns (uint) {
    return x;
}
function f(int8 storage x) pure returns (int) {
    return x[0];
}
using {f} for uint8;
using {f} for int;
// ----
// DeclarationError 7920: (132-133): Identifier not found or not unique.
