function f(uint x, uint[] y) pure returns (uint) {
    return x;
}
function f(uint x, uint y) pure returns (int) {
    return x;
}
using {f} for uint;
// ----
// DeclarationError 9589: (138-139): Identifier is not a function name or not unique.
