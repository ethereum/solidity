contract C {
	struct S {
		function() a;
	}
	function f(S[2] calldata) public {}
}
// ----
// TypeError 4103: (56-69='S[2] calldata'): Internal type is not allowed for public or external functions.
