
		contract A { function f(uint[] calldata a) virtual external returns (uint) { return a[0]; } }
		contract B is A {
			function f(uint[] memory a) public override returns (uint) { return a[1]; }
			function g() public returns (uint) {
				uint[] memory m = new uint[](2);
				m[0] = 42;
				m[1] = 23;
				return A(this).f(m);
			}
		}
	
// ----
// g() -> 23

