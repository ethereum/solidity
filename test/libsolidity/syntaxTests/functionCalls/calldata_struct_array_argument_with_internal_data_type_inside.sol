contract C {
	struct S {
		function() a;
	}
	function f(S[2] calldata) public {}
}
// ----
// TypeError 4103: (56-69): Internal type is not allowed for public or external functions.
