contract C {
    uint[] a;
    function f() public returns (uint, uint) {
        uint[] memory b = new uint[](3);
        b[0] = 1;
        a = b;
        return (a[0], a.length);
    }
}
// ----
// f() -> 1, 3
