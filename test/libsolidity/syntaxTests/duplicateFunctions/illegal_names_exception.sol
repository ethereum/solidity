// Exception for the rule about illegal names.
contract C {
	function this() public {
	}
	function super() public {
	}
	function _() public {
	}
}
// ----
// Warning 2319: (61-88): This declaration shadows a builtin symbol.
// Warning 2319: (90-118): This declaration shadows a builtin symbol.
