
		interface I { function f(uint[] calldata a) external returns (uint); }
		contract A is I { function f(uint[] calldata a) external override returns (uint) { return 42; } }
		contract B {
			function f(uint[] memory a) public returns (uint) { return a[1]; }
			function g() public returns (uint) {
				I i = I(new A());
				return i.f(new uint[](2));
			}
		}
	
// ----
// g() -> 42

