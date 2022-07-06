contract Test {
	struct S { uint8 a; mapping(uint => uint) b; uint8 c; }
	S s;
	function f() public {
		S memory x;
	}
}
// ----
// TypeError 4061: (104-114): Type struct Test.S memory is only valid in storage because it contains a (nested) mapping.
