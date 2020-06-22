contract c {
	struct S {
		uint x;
	}
	mapping(S => uint) data;
}
// ----
// TypeError 7804: (47-48): Only elementary types, contract types or enums are allowed as mapping keys.
