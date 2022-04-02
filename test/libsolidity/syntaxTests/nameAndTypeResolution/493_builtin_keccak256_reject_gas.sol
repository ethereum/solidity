contract C {
    function f() public {
        keccak256.gas();
    }
}
// ----
// TypeError 9582: (47-60='keccak256.gas'): Member "gas" not found or not visible after argument-dependent lookup in function (bytes memory) pure returns (bytes32).
