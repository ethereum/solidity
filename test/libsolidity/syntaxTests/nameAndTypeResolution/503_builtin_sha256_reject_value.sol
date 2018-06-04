contract C {
    function f() public {
        sha256.value();
    }
}
// ----
// TypeError: (47-59): Member "value" not found or not visible after argument-dependent lookup in function () pure returns (bytes32) - did you forget the "payable" modifier?
