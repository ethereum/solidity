contract C {
    function f(int32 x) external pure returns (int32)
    {
        this.x + 1;
        return x;
    }
}
// ----
// TypeError 9582: (81-87): Member "x" not found or not visible after argument-dependent lookup in contract C.
