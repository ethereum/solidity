
		contract C {
			uint public x;
			modifier m1 {
				address a1 = msg.sender;
				x++;
				_;
			}
			function f1() m1() public {
				x += 7;
			}
			function f2() m1() public {
				x += 3;
			}
		}
	
// ----
// f1() -> 
// x() -> 0x08
// f2() -> 
// x() -> 0x0c

