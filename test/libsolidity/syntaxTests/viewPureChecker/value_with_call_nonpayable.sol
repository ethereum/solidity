contract C {
	function f(address a) external view returns (bool success) {
		(success,) = a.call.value(42)("");
		(success,) = a.call{value: 42}("");
	}
	function g(address a) external view returns (bool success) {
		(success,) = a.call.value(42)("");
		(success,) = a.call{value: 42}("");
	}
	function h() external payable {}
	function i() external view {
		this.h.value(42)();
		this.h{value: 42}();
	}
}
// ----
// Warning: (90-102): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// Warning: (230-242): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// Warning: (359-371): Using ".value(...)" is deprecated. Use "{value: ...}" instead.
// TypeError: (90-110): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (127-148): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (230-250): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (267-288): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (359-377): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (381-400): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
