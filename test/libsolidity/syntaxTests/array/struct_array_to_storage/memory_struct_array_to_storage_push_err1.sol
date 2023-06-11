contract C {
	struct Child { uint32 i; }
	struct Parent { Child[] children; }

	Child[] sChildren;

	function test() public {
		Child[] memory children;

		sChildren.push(children); // Not yet supported
	}
}
// ----
// TypeError 1598: (156-170): Source struct cannot be pushed to target struct array struct C.Child[] storage ref. E.g. beuase push memory struct array to storage struct array not yet implemented.