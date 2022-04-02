contract C {
    function (uint) external returns (uint) x;
    function g() public {
        x{value: 2}(1);
    }
}
// ----
// TypeError 7006: (94-105='x{value: 2}'): Cannot set option "value" on a non-payable function type.
