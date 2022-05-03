contract C {
    function add(uint16 a, uint16 b) public pure returns (uint16) {
        return a + b; // can overflow
    }

    function f(uint16 a, uint16 b, uint16 c) public pure returns (uint16) {
        unchecked { return add(a, b) + c; } // add can still overflow, `+ c` can't
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (96-101): CHC: Overflow (resulting value larger than 65535) happens here.\nCounterexample:\n\na = 65535\nb = 1\n = 0\n\nTransaction trace:\nC.constructor()\nC.add(65535, 1)
