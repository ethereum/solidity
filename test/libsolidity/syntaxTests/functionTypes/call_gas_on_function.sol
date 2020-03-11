contract C {
    function (uint) external returns (uint) x;
    function f() public {
        x{gas: 2}(1);
    }
}

