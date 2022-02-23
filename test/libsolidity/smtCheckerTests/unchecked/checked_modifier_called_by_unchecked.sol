contract C {
    modifier add(uint16 a, uint16 b) {
        unchecked { a + b; } // overflow not reported
        _;
    }

    function f(uint16 a, uint16 b, uint16 c) public pure add(a, b) returns (uint16) {
        return b + c; // can overflow
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (225-230): CHC: Overflow (resulting value larger than 65535) happens here.\nCounterexample:\n\na = 65535\nb = 1\nc = 65535\n = 0\na = 65535\nb = 1\n\nTransaction trace:\nC.constructor()\nC.f(65535, 1, 65535)
