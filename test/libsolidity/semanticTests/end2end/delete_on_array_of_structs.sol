
		contract C {
			struct S { uint x; uint[] y; }
			S[] data;
			function f() public returns (bool) {
				S storage s1 = data.push();
				s1.x = 2**200;
				S storage s2 = data.push();
				s2.x = 2**200;
				delete data;
				return true;
			}
		}
	
// ----
// f() -> 0x1

