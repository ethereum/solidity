function f(uint[] memory x) pure returns (uint) {
    return x[0];
}
function g(uint[] storage x) view returns (uint) {
    return x[0];
}
function h(uint[] calldata x) pure returns (uint) {
    return x[0];
}
using {f, g, h} for uint[];
// ----
