contract C {
    bytes20 x;
    function f(bytes16 b) public view {
        b[uint8(x[2])];
    }
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6368: (76-90): CHC: Out of bounds access might happen here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
