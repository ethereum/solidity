contract test {
	function f() public { }
	function g() public { revert(); }
	function h() public { assert(false); }
}
// ====
// compileViaYul: also
// ----
// f() ->
// g() -> FAILURE
// h() -> FAILURE
