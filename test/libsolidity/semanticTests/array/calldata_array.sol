pragma abicoder               v2;


contract C {
    function f(uint256[2] calldata s)
        external
        pure
        returns (uint256 a, uint256 b)
    {
        a = s[0];
        b = s[1];
    }
}
// ====
// compileToEwasm: also
// ----
// f(uint256[2]): 42, 23 -> 42, 23
