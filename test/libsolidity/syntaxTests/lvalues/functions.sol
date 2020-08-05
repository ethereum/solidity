contract C {
    function f() internal {
    }
    function g() internal {
        g = f;
    }
    function h() external {
    }
    function i() external {
        this.i = this.h;
    }
}
// ----
// TypeError 4247: (83-84): Expression has to be an lvalue.
// TypeError 4247: (166-172): Expression has to be an lvalue.
