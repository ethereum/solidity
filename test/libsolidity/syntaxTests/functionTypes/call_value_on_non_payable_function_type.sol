contract C {
    function (uint) external returns (uint) x;
    function f() public {
        x.value(2)();
    }
}
// ----
// TypeError: (94-101): Member "value" is only available for payable functions.
