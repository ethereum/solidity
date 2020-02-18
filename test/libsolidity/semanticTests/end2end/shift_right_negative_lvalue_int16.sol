
			contract C {
				function f(int16 a, int16 b) public returns (int) {
					return a >> b;
				}
			}
		
// ----
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x01 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7ab
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef5
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x01 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7aa
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef5
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int16,int16): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

