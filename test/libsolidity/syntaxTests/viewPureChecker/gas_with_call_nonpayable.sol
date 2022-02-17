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
// TypeError 8961: (90-109): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (180-197): Function cannot be declared as view because this expression (potentially) modifies the state.
