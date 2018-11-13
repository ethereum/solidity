contract C {
    function h() pure external {
    }
    function f() view external returns (bytes4) {
        function () payable external g = this.h;
        return g.selector;
    }
}
// ----
// TypeError: (110-149): Type function () pure external is not implicitly convertible to expected type function () payable external.
