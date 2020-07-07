contract C {
	function f(address a) external view returns (bool success) {
		(success,) = a.call.gas(42)("");
		(success,) = a.call{gas: 42}("");
	}
	function g(address a) external view returns (bool success) {
		(success,) = a.call.gas(42)("");
		(success,) = a.call{gas: 42}("");
	}
	function h() external payable {}
	function i() external view {
		this.h.gas(42)();
	}
	function j() external view {
		this.h{gas: 42}();
	}
}
// ----
// Warning 1621: (90-100): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning 1621: (226-236): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning 1621: (351-361): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// TypeError 8961: (90-108): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError 8961: (125-144): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError 8961: (226-244): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError 8961: (261-280): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError 8961: (351-367): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError 8961: (404-421): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
