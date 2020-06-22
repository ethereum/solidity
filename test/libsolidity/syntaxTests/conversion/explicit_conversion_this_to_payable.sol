contract C {
	function f() public pure {
		address payable p = payable(this);
		address payable q = payable(address(this));
	}
}
// ----
// TypeError 9640: (63-76): Explicit type conversion not allowed from "contract C" to "address payable".
