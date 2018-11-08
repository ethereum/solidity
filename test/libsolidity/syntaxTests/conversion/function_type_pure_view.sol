contract C {
    function h() pure external {
    }
    function f() view external returns (bytes4) {
        function () view external g = this.h;
        return g.selector;
    }
}
