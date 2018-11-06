contract C {
    function h() payable external {
    }
    function f() view external returns (bytes4) {
        function () view external g = this.h;
        return g.selector;
    }
}
// ----
// TypeError: (113-149): Type function () payable external is not implicitly convertible to expected type function () view external.
