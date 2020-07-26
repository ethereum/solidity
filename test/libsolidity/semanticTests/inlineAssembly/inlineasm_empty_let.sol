contract C {
	function f() public pure returns (uint a, uint b) {
		assembly {
			let x
			let y, z
			a := x
			b := z
		}
	}
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 0, 0
