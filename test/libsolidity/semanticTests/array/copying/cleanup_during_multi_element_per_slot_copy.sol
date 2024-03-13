contract C {
    uint32[] s;
    constructor()
    {
        s.push();
        s.push();
    }
    function f() external returns (uint)
    {
        (s[1], s) = (4, [0]);
        s = [0];
        s.push();
        return s[1];
        // used to return 4 via IR.
    }
}
// ----
// constructor()
// gas irOptimized: 90065
// gas irOptimized code: 149000
// gas legacy: 89553
// gas legacy code: 126200
// gas legacyOptimized: 83556
// gas legacyOptimized code: 98200
// f() -> 0
