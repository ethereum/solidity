contract C {
    function h() view external {
    }
    function f() view external returns (bytes4) {
        function () pure external g = this.h;
        return g.selector;
    }
}
// ----
// TypeError: (110-146): Type function () view external is not implicitly convertible to expected type function () pure external.
