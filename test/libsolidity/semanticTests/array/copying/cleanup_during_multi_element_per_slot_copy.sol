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
// gas irOptimized: 89349
// gas irOptimized code: 140200
// gas legacy: 100902
// gas legacy code: 271400
// gas legacyOptimized: 83062
// gas legacyOptimized code: 91800
// gas ssaCFGOptimized: 88635
// gas ssaCFGOptimized code: 137600
// f() -> 0
