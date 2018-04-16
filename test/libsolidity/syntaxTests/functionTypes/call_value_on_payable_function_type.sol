contract C {
    function (uint) external payable returns (uint) x;
    function f() public {
        x.value(2)(1);
    }
}
