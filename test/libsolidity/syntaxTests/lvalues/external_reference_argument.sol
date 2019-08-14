contract C {
    function f(uint256[] calldata x, uint256[] calldata y) external pure {
        x = y;
    }
}
// ----
// TypeError: (96-97): External function arguments of reference type are read-only.
