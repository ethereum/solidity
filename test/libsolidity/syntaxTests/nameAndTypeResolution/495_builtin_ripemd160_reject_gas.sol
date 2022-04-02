contract C {
    function f() public {
        ripemd160.gas();
    }
}
// ----
// TypeError 9582: (47-60='ripemd160.gas'): Member "gas" not found or not visible after argument-dependent lookup in function (bytes memory) pure returns (bytes20).
