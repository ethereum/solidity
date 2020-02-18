
		contract Flow {
			bool public success;

			mapping (address => function () internal) stages;

			function stage0() internal {
				stages[msg.sender] = stage1;
			}

			function stage1() internal {
				stages[msg.sender] = stage2;
			}

			function stage2() internal {
				success = true;
			}

			constructor() public {
				stages[msg.sender] = stage0;
			}

			function f() public returns (uint) {
				stages[msg.sender]();
				return 7;
			}
		}
	
// ----
// success() -> 0x0
// f() -> 0x07
// f() -> 0x07
// success() -> 0x0
// f() -> 0x07
// success() -> 0x1

