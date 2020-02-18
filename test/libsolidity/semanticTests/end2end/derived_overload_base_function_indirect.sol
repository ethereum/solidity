
		contract A { function f(uint a) public returns(uint) { return 2 * a; } }
		contract B { function f() public returns(uint) { return 10; } }
		contract C is A, B {
			function g() public returns(uint) { return f(); }
			function h() public returns(uint) { return f(1); }
		}
	
// ====
// optimize-yul: false
// ----
// g() -> 0xa
// h() -> 0x2

