contract B {
    uint immutable private x;

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
// TypeError 2658: (0-202): Construction control flow ends without initializing all immutable state variables.
// TypeError 2658: (204-361): Construction control flow ends without initializing all immutable state variables.
