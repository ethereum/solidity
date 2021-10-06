contract Test {
	uint constant x = 2;
	function f() public pure {
		assembly {
			let y := x.length
		}
	}
}
// ----
// TypeError 3622: (91-99): The suffix ".length" is not supported by this variable or type.
