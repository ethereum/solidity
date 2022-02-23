function f(uint) returns (bytes memory) {}
function f(uint[] memory x) returns (bytes memory) { return f(x[0]); }
function g(uint8) {}
function g(uint16) {}
function t() {
    g(2);
}
// ----
// TypeError 4487: (176-177): No unique declaration found after argument-dependent lookup.
