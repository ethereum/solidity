contract C {
    function f(address payable) internal pure {}
    function f(address) internal pure {}
    function g() internal pure {
        address payable a = address(0);
        f(a);
    }
}
// ----
// TypeError: (184-185): No unique declaration found after argument-dependent lookup.
