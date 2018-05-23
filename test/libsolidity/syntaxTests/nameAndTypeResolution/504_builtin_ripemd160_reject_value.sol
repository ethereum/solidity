contract C {
    function f() public {
        ripemd160.value();
    }
}
// ----
// TypeError: (47-62): Member "value" not found or not visible after argument-dependent lookup in function (bytes memory) pure returns (bytes20) - did you forget the "payable" modifier?
