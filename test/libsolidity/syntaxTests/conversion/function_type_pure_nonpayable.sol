contract C {
    function h() pure external {
    }
    function f() view external returns (bytes4) {
        function () external g = this.h;
        return g.selector;
    }
}
