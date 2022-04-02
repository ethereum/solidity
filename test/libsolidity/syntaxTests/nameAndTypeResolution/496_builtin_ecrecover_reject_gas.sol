contract C {
    function f() public {
        ecrecover.gas();
    }
}
// ----
// TypeError 9582: (47-60='ecrecover.gas'): Member "gas" not found or not visible after argument-dependent lookup in function (bytes32,uint8,bytes32,bytes32) pure returns (address).
