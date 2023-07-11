pragma abicoder               v2;


contract C {
    function f(uint256[][] calldata a)
        external
        returns (uint256, uint256[] memory)
    {
        uint256[] memory m = a[0];
        return (a.length, m);
    }
}
// ----
// f(uint256[][]): 0x20, 0x1, 0x20, 0x2, 0x17, 0x2a -> 0x1, 0x40, 0x2, 0x17, 0x2a
