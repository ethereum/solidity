function f(uint x, uint[] y) pure returns (uint) {
    return x;
}
function f(uint x, uint y) pure returns (int) {
    return x;
}
using {f} for uint;
// ----
// DeclarationError 7920: (138-139): Identifier not found or not unique.
