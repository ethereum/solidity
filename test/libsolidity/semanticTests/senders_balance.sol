
		contract C {
			function f() public view returns (uint) {
				return msg.sender.balance;
			}
		}
		contract D {
			C c = new C();
			constructor() public payable { }
			function f() public view returns (uint) {
				return c.f();
			}
		}
	
// ----
// f() -> 27

