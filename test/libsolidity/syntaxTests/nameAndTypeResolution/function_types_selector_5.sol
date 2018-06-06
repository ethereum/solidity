contract C {
    function h() pure external {
    }
    function f() view external returns (bytes4) {
        var g = this.h;
        return g.selector;
    }
}
// ----
// Warning: (110-115): Use of the "var" keyword is deprecated.
