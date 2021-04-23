contract C {
    function f() public pure returns (fixed) {
        return 1.33;
    }
    function g(fixed f1, fixed f2) public pure returns (fixed, fixed, fixed) {
        return (f2, 0.0000333, f1);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1.330000000000000000
// g(fixed128x18,fixed128x18): 0.000000000000009871, 0.000000000888888880 -> 0.000000000888888880, 0.000033300000000000, 0.000000000000009871
