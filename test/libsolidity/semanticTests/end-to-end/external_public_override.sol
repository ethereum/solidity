
		contract A {
			function f() external virtual returns (uint) { return 1; }
		}
		contract B is A {
			function f() public override returns (uint) { return 2; }
			function g() public returns (uint) { return f(); }
		}
	
// ====
// compileViaYul: also
// ----
// f() -> 2
// g() -> 2

