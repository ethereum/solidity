contract C {
	function f(address a) external view returns (bool success) {
		(success,) = a.call{value: 42}("");
	}
	function h() external payable {}
	function i() external view {
		this.h{value: 42}();
	}
}
// ----
// TypeError 8961: (90-111): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (182-201): Function cannot be declared as view because this expression (potentially) modifies the state.
