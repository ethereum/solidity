
		contract C {
			function () internal returns (uint) x;
			function set() public {
				C.x = g;
			}
			function g() public pure returns (uint) { return 2; }
			function h() public returns (uint) { return C.x(); }
		}
	
// ----
// g() -> 2
// h() -> FAILURE
// set() ->
// h() -> 2
