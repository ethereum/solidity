pragma abicoder               v2;


contract C {
    function f(string[] calldata a)
        external
        returns (uint256, uint256, uint256, string memory)
    {
        string memory s1 = a[0];
        bytes memory m1 = bytes(s1);
        return (a.length, m1.length, uint8(m1[0]), s1);
    }
}
// ====
// compileViaYul: also
// ----
// f(string[]): 0x20, 0x1, 0x20, 0x2, hex"6162000000000000000000000000000000000000000000000000000000000000" -> 1, 2, 97, 0x80, 2, "ab"
