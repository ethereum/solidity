
		contract Base {
			function f() public returns (uint i) { return g(); }
			function g() public virtual returns (uint i) { return 1; }
		}
		contract Derived is Base {
			function g() public override returns (uint i) { return 2; }
		}
	
// ====
// optimize-yul: false
// ----
// g() -> 0x2
// f() -> 0x2

