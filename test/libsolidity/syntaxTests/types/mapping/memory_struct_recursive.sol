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
// TypeError 4061: (143-153='S memory s'): Type struct Test.S is only valid in storage because it contains a (nested) mapping.
