contract C {
    function (uint) external payable returns (uint) x;
    function f() public {
        x.value(2)(1);
    }
}
// ----
// TypeError 1621: (102-109): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
