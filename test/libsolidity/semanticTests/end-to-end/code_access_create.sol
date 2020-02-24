
		contract C {
			function test() public returns (uint) {
				bytes memory c = type(D).creationCode;
				D d;
				assembly {
					d := create(0, add(c, 0x20), mload(c))
				}
				return d.f();
			}
		}
		contract D {
			uint x;
			constructor() public { x = 7; }
			function f() public view returns (uint) { return x; }
		}
	
// ----
// test() -> 7

