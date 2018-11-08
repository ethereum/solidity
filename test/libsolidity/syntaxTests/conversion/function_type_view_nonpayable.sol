contract C {
	int dummy;
    function h() view external {
		dummy;
    }
    function f() view external returns (bytes4) {
        function () external g = this.h;
        return g.selector;
    }
}
