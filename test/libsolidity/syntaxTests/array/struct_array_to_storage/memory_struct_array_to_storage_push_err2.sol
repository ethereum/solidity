contract C {
	struct Child { uint32 i; }
	struct Parent { Child[] children; }

	Parent[] sParents;

	function test() public {
		Child[] memory children;

		sParents.push(Parent(children)); // Not yet supported
	}
}
// ----
// TypeError 1598: (156-169): Source struct cannot be pushed to target struct array struct C.Parent[] storage ref. E.g. beuase push memory struct array to storage struct array not yet implemented.