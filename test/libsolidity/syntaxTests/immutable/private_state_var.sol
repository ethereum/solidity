contract B {
    uint immutable private x = f();

    constructor() {
    }

    function f() internal view virtual returns(uint) { return 1; }
    function readX() internal view returns(uint) { return x; }
}

contract C is B {
    uint immutable y;
    constructor() {
        y = 3;
    }
    function f() internal view override returns(uint) { return readX(); }

}
// ----
// TypeError 7733: (202-203): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
