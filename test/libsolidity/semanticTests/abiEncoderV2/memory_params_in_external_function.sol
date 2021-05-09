pragma abicoder v2;

contract C {
    function f(bytes memory a, bytes calldata b, uint[] memory c)
        external
        pure
        returns (uint, bytes1, uint, bytes1, uint, uint)
    {
        return (a.length, a[1], b.length, b[2], c.length, c[3]);
    }
    function g() public returns (uint, bytes1, uint, bytes1, uint, uint) {
        uint[] memory x = new uint[](4);
        x[3] = 7;
        return this.f("abc", "def", x);
    }
}
// ====
// compileViaYul: also
// ----
// g() -> 3, 0x6200000000000000000000000000000000000000000000000000000000000000, 3, 0x6600000000000000000000000000000000000000000000000000000000000000, 4, 7
