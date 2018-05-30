contract C {
    function f() public {
        ecrecover.value();
    }
}
// ----
// TypeError: (47-62): Member "value" not found or not visible after argument-dependent lookup in function (bytes32,uint8,bytes32,bytes32) pure returns (address) - did you forget the "payable" modifier?
