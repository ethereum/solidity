contract C {
    function (uint) internal payable returns (uint) x;
}
// ----
// TypeError: (17-66): Only external function types can be payable.
