library L {
    function f(uint256) private {}
}
contract C {
    function f() public pure returns (bytes4) {
        return L.f.selector;
    }
}
// ----
// TypeError: (125-128): Member "f" not found or not visible after argument-dependent lookup in type(library L).
