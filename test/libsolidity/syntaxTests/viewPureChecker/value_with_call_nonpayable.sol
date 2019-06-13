contract C {
	function f(address a) external view returns (bool success) {
		(success,) = a.call.value(42)("");
	}
	function g(address a) external view returns (bool success) {
		(success,) = a.call.value(42)("");
	}
	function h() external payable {}
	function i() external view {
		this.h.value(42)();
	}
}
// ----
// TypeError: (90-110): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (192-212): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (283-301): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
