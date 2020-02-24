
		contract Lib { struct S {uint a; uint b; } }
		contract Test {
			function f() public returns (uint r) {
				Lib.S memory x = Lib.S({a: 2, b: 3});
				r = x.b;
			}
		}
	
// ----
// f() -> 3

