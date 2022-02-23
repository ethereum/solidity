library L {
    function value(function()internal a, uint256 b) internal {}
}
contract C {
    using L for function()internal;
    function f() public {
        function()internal x;
        x.value(42);
    }
}
// ----
