contract C {
	function empty_tuple() public pure {
		true ? () : (); // bug fix test, proper handling of empty tuples
	}

	function non_empty_tuple() public pure {
		true ? (1, 2) : (3, 4);
	}

	function return_empty_tuple() public pure {
		return true ? () : ();
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6133: (53-67): Statement has no effect.
// Warning 6133: (166-188): Statement has no effect.
