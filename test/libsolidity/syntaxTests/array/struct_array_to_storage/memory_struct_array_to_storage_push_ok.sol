contract C {
	struct Child { uint32 i; }
	struct Parent { Child[] children; }

	Child sChild;
	Child[] sChildren;

	function test() public {
		Child memory child;

		sChildren.push(child); // OK
		sChildren.push(sChild); // OK
	}
}
// ----