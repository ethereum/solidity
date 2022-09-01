contract Test {
	struct S {
		T[] t;
	}

	struct T {
		U[] u;
	}

	struct U {
		S[] s;
		mapping (uint => S) map;
	}

	function f() public {
		S memory s;
	}
}
// ----
// TypeError 4061: (143-153): Type struct Test.S memory is only valid in storage because it contains a (nested) mapping.
