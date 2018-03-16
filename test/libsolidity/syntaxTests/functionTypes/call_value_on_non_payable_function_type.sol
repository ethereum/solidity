contract C {
    function (uint) external returns (uint) x;
    function f() public {
        x.value(2)();
    }
}
// ----
// TypeError: (94-101): Member "value" not found or not visible after argument-dependent lookup in function (uint256) external returns (uint256) - did you forget the "payable" modifier?
