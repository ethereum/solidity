// For accessors, the dynamic types are already removed in the external signature itself.
contract C {
    struct S {
        uint256 x;
        string a; // this is present in the accessor
        uint256[] b; // this is not present
        uint256 y;
    }
    S public s;

    function g() public returns (uint256, uint256) {
        s.x = 2;
        s.a = "abc";
        s.b = [7, 8, 9];
        s.y = 6;
        (uint256 x, , uint256 y) = this.s();
        return (x, y);
    }
}

// ====
// compileViaYul: also
// ----
// g() -> 2, 6
// gas irOptimized: 178953
// gas legacy: 180890
// gas legacyOptimized: 179609
