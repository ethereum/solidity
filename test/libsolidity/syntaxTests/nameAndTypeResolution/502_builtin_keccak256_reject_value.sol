contract C {
    function f() public {
        keccak256.value();
    }
}
// ----
// TypeError: (47-62): Member "value" not found or not visible after argument-dependent lookup in function (bytes memory) pure returns (bytes32) - did you forget the "payable" modifier?
