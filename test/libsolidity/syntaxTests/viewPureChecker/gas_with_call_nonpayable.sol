contract C {
	function f(address a) external view returns (bool success) {
		(success,) = a.call{gas: 42}("");
	}
	function h() external payable {}
	function i() external view {
		this.h{gas: 42}();
	}
}
// ----
// TypeError: (90-109): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (180-197): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
