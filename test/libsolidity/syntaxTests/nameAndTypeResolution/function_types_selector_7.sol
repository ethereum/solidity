contract C {
    function h() pure external {
    }
    function f() view external returns (bytes4) {
        function () pure external g = this.h;
        var i = g;
        return i.selector;
    }
}
// ----
// Warning: (156-161): Use of the "var" keyword is deprecated.
