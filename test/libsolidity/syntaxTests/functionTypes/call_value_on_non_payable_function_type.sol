contract C {
    function (uint) external returns (uint) x;
    function f() public {
        x{value: 2}(1);
    }
}
// ----
// TypeError: (94-105): Cannot set option "value" on a non-payable function type.
