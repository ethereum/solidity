contract B {
    function f() external {}
    function g() public {}
}
contract C is B {
    function h() public returns (bytes4 fs, bytes4 gs) {
        fs = B.f.selector;
        gs = B.g.selector;
        B.g();
    }
}
