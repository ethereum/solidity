
		contract C {
			enum X { A, B }

			function tested (X x) public returns (uint) {
				return 1;
			}

			function test() public returns (uint) {
				X garbled;

				assembly {
					garbled := 5
				}

				return this.tested(garbled);
			}
		}
	
// ----
// test() ->  # should throw #

