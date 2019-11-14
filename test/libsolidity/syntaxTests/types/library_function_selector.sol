library L {
    function f(uint256) external {}
    function g(uint256[] storage) external {}
    function h(uint256[] memory) public {}
}
contract C {
    function f() public pure returns (bytes4 a, bytes4 b, bytes4 c, bytes4 d) {
        a = L.f.selector;
        b = L.g.selector;
        c = L.h.selector;
        d = L.h.selector;
    }
}