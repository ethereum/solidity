contract C {
	struct S {
		uint value;
		address origin;
		uint number;
	}
	function f() public payable {
		S memory msg = S(42, address(0), 666);
		S memory tx = S(42, address(0), 666);
		S memory block = S(42, address(0), 666);
		assert(msg.value == 42); // should hold
		assert(msg.value == 41); // should fail
		assert(tx.origin == address(0)); // should hold
		assert(block.number == 666); // should hold
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 2319: (108-120): This declaration shadows a builtin symbol.
// Warning 2319: (149-160): This declaration shadows a builtin symbol.
// Warning 2319: (189-203): This declaration shadows a builtin symbol.
// Warning 6328: (274-297): CHC: Assertion violation happens here.
