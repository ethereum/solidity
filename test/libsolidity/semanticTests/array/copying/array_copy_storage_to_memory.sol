contract C {
    uint[] a;
    function f() public returns (uint, uint) {
        a.push(1); a.push(0); a.push(0);
        uint[] memory b = a;
        return (b[0], b.length);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1, 3
