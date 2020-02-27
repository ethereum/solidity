interface A {
    function f() external;
}
contract B {
    function g() public {}
}

contract C is B {
    function h() external {}
    bytes4 constant s1 = A.f.selector;
    bytes4 constant s2 = B.g.selector;
    bytes4 constant s3 = this.h.selector;
    bytes4 constant s4 = super.g.selector;
}
