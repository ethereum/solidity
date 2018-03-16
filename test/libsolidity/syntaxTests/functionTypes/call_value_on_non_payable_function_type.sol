contract C {
    function (uint) external returns (uint) x;
    function f() public {
        x.value(2)();
    }
}
