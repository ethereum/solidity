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
// Warning: (90-100): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning: (226-236): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// Warning: (351-361): Using ".gas(...)" is deprecated. Use "{gas: ...}" instead.
// TypeError: (90-108): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (125-144): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (226-244): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (261-280): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (351-367): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (404-421): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
