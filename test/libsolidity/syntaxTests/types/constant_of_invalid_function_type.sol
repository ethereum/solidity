contract C {
	// Used to cause internal compiler error.
	function() returns (x) constant x = x;

}
// ----
// TypeError 5172: (77-78): Name has to refer to a user-defined value type, struct, enum or contract.
