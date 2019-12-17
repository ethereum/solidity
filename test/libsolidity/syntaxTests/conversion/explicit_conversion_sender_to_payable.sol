contract C {
	function f() public view {
		address payable p = payable(msg.sender);
		address payable q = payable(address(msg.sender));
	}
}
// ----
// Warning: (43-60): Unused local variable.
// Warning: (86-103): Unused local variable.
