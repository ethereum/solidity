contract c {
	struct S {
		string s;
	}
	mapping(S => uint) data;
}
// ----
// TypeError 7804: (49-50): Only elementary types, user defined value types, contract types or enums are allowed as mapping keys.
