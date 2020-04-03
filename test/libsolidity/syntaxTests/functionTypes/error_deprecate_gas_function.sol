contract C {
    function (uint) external payable returns (uint) x;
    function f() public {
        x.gas(2)(1);
    }
}
// ----
// TypeError: (102-107): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
