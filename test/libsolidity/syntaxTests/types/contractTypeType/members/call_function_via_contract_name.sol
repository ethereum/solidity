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
// TypeError 3419: (160-165): Cannot call function via contract type name.
// TypeError 3419: (175-180): Cannot call function via contract type name.
// TypeError 3419: (190-195): Cannot call function via contract type name.
