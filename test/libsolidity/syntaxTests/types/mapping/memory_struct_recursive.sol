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
// TypeError: (143-153): Type struct Test.S is only valid in storage.
