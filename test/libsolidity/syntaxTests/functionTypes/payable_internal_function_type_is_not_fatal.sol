contract C {
    function (uint) internal payable returns (uint) x;

    function g() public {
        x = g;
    }
}
// ----
// TypeError 7415: (17-66): Only external function types can be payable.
