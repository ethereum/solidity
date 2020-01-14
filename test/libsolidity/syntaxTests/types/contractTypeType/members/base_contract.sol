contract B {
    function f() external {}
    function g() public {}
}
contract C is B {
    function h() public {
        B.f.selector;
        B.g.selector;
        B.g();
    }
}
