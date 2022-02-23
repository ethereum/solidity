contract A {
    function f() internal virtual returns(uint) { return 3; }
}

contract B {
    uint immutable x;

    constructor() {
        x = xInit();
    }

    function xInit() internal virtual returns(uint) {
        return f();
    }

    function f() internal virtual returns(uint) { return 3; }
}

contract C is A, B {
    function xInit() internal override returns(uint) {
        return super.xInit();
    }

    function f() internal override(A, B) returns(uint) {
        return x;
    }
}
// ----
// TypeError 7733: (493-494): Immutable variables cannot be read before they are initialized.
