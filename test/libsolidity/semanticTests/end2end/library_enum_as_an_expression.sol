
		library Arst {
			enum Foo {
				Things,
				Stuff
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

