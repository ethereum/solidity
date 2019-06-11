contract test {
	function f() public { }
	function g() public { revert(); }
	function h() public { assert(false); }
}
// ----
// f() ->
// g() -> FAILURE
// h() -> FAILURE
