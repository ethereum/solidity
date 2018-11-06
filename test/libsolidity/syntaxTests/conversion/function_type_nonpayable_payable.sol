contract C {
    function h() external {
    }
    function f() view external returns (bytes4) {
        function () payable external g = this.h;
        return g.selector;
    }
}
// ----
// TypeError: (105-144): Type function () external is not implicitly convertible to expected type function () payable external.
