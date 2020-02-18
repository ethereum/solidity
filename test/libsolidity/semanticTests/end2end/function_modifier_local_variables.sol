
		contract C {
			modifier mod1 { uint8 a = 1; uint8 b = 2; _; }
			modifier mod2(bool a) { if (a) return; else _; }
			function f(bool a) mod1 mod2(a) public returns (uint r) { return 3; }
		}
	
// ----
// f(bool): 0x1 -> 0x0
// f(bool): 0x0 -> 0x3

