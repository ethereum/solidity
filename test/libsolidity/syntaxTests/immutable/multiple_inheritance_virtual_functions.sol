contract A {
    function f() internal virtual returns(uint) { return 3; }
}

contract B {
    uint immutable x;

    constructor() public {
        x = xInit();
    }

    function xInit() internal virtual returns(uint) {
        return f();
    }

    function f() internal virtual returns(uint) { return 3; }
}

contract C is A, B {
    function xInit() internal override returns(uint) {
        return B.xInit();
    }

    function f() internal override(A, B) returns(uint) {
        return x;
    }
}
// ----
// TypeError: (496-497): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
