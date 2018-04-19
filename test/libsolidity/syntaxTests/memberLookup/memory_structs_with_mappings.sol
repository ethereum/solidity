contract Test {
	struct S { uint8 a; mapping(uint => uint) b; uint8 c; }
	S s;
	function f() public {
		S memory x;
		x.b[1];
	}
}
// ----
// TypeError: (118-121): Member "b" is not available in struct Test.S memory outside of storage.
