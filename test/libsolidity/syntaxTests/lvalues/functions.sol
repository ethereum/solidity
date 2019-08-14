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
// TypeError: (83-84): Expression has to be an lvalue.
// TypeError: (166-172): Expression has to be an lvalue.
