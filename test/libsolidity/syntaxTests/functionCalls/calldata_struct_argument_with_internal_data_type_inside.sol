contract C {
	struct S {
		function() a;
	}
	function f(S calldata) public {}
}
// ----
// TypeError 4103: (56-66): Internal type is not allowed for public or external functions.