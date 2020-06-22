contract C {
    function f() external payable {}
	function g(address a) external pure {
		a.call.value(42);
		a.call{value: 42};
		a.call.gas(42);
		a.call{gas: 42};
		a.staticcall.gas(42);
		a.staticcall{gas: 42};
		a.delegatecall.gas(42);
		a.delegatecall{gas: 42};
	}
	function h() external view {
		this.f.value(42);
		this.f{value: 42};
		this.f.gas(42);
		this.f{gas: 42};
	}
}
// ----
// Warning 1621: (91-103): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// Warning 1621: (132-142): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning 1621: (169-185): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning 1621: (218-236): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning 1621: (304-316): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// Warning 1621: (345-355): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
