
		contract C {
			function f(int a, int b) public returns (int) {
				a >>= b;
				return a;
			}
		}
	
// ----
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x01 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7ab
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef5
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x01 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7aa
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef5
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int256,int256): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

