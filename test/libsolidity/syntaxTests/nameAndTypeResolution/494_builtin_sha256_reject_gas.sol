contract C {
    function f() public {
        sha256.gas();
    }
}
// ----
// TypeError: (47-57): Member "gas" not found or not visible after argument-dependent lookup in function (bytes memory) pure returns (bytes32).
