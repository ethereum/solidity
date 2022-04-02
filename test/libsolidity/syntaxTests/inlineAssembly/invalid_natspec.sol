contract C {
	function f() public pure {
		/// @test test
		assembly {}
		/// @solidity test
		assembly {}
		/// @param
		assembly {}
	}
}
// ----
// Warning 6269: (60-71='assembly {}'): Unexpected NatSpec tag "test" with value "test" in inline assembly.
// Warning 8787: (95-106='assembly {}'): Unexpected value for @solidity tag in inline assembly: test
// Warning 7828: (122-133='assembly {}'): Inline assembly has invalid NatSpec documentation.
