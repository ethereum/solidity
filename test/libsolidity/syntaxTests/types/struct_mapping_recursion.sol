// Used to segfault.
contract C {
	struct S {
		mapping(S => uint) a;
	}
	function g (S calldata) external view {}
 }
// ----
// TypeError 7804: (56-57): Only elementary types, user defined value types, contract types or enums are allowed as mapping keys.
