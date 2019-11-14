library L {
    function f(uint256) internal {}
}
contract C {
    function f() public pure returns (bytes4) {
        return L.f.selector;
    }
}
// ----
// TypeError: (126-138): Member "selector" not found or not visible after argument-dependent lookup in function (uint256).
