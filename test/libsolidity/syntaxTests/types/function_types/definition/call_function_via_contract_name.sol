contract A {
    function f() external {}
    function g() external pure {}
    function h() public pure {}
}

contract B {
    function i() external {
        A.f();
        A.g();
        A.h(); // might be allowed in the future
    }
}
// ----
// TypeError: (160-165): Cannot call function via contract name.
// TypeError: (175-180): Cannot call function via contract name.
// TypeError: (190-195): Cannot call function via contract name.
