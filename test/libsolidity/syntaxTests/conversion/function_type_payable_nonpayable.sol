contract C {
    function h() payable external {
    }
    function f() view external returns (bytes4) {
        function () external g = this.h;
        return g.selector;
    }
}
