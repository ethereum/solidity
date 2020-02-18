
			contract C {
				function f(int32 a, int32 b) public returns (int) {
					return a >> b;
				}
			}
		
// ----
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x01 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7ab
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef5
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef56, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x01 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7aa
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef5
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int32,int32): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffef55, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

