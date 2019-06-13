contract C {
    function f() external payable {}
	function g(address a) external pure {
		a.call.value(42);
		a.call.gas(42);
		a.staticcall.gas(42);
		a.delegatecall.gas(42);
	}
	function h() external view {
		this.f.value(42);
		this.f.gas(42);
	}
}
