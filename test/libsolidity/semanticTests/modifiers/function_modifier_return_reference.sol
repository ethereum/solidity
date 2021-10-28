contract C {
    modifier m1(uint value) {
        _;
    }
    modifier m2(uint value) {
        _;
    }

    function f() public m1(x = 2) m2(y = 3) returns (uint x, uint y) {
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 2, 3
