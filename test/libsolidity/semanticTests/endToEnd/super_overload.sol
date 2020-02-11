contract A {
    function f(uint a) public returns(uint) {
        return 2 * a;
    }
}
contract B {
    function f(bool b) public returns(uint) {
        return 10;
    }
}
contract C is A, B {
    function g() public returns(uint) {
        return super.f(true);
    }

    function h() public returns(uint) {
        return super.f(1);
    }
}

// ----
// g() -> 10
// h() -> 2
