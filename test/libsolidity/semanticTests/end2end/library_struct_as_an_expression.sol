
		library Arst {
			struct Foo {
				int Things;
				int Stuff;
			}
		}

		contract Tsra {
			function f() public returns(uint) {
				Arst.Foo;
				return 1;
			}
		}
	
// ----
// f() -> 0x01

