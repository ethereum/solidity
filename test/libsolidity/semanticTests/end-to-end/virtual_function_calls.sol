
		contract Base {
			function f() public returns (uint i) { return g(); }
			function g() public virtual returns (uint i) { return 1; }
		}
		contract Derived is Base {
			function g() public override returns (uint i) { return 2; }
		}
	
// ====
// compileViaYul: also
// ----
// g() -> 2
// f() -> 2

