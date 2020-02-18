
		contract C {
			struct S { uint a; bool x; }
			S public s;
			constructor() public {
				s = S({a: 1, x: true});
			}
		}
	
// ----
// s() -> 0x01, 0x1

