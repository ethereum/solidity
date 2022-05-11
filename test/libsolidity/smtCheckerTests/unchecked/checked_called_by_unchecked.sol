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
// SMTIgnoreCex: yes
// ----
// Warning 4984: (96-101): CHC: Overflow (resulting value larger than 65535) happens here.
