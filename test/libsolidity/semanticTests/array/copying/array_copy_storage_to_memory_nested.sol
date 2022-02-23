pragma abicoder v2;
contract C {
    uint[][] a;

    function f() public returns (uint[][] memory) {
        a.push();
        a.push();
        a[0].push(0);
        a[0].push(1);
        a[1].push(2);
        a[1].push(3);
        uint[][] memory m = a;
        return m;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x20, 2, 0x40, 0xa0, 2, 0, 1, 2, 2, 3
// gas irOptimized: 161780
// gas legacy: 162278
// gas legacyOptimized: 159955
