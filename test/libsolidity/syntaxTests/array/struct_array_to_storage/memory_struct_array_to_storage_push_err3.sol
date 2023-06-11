contract C {
	struct Child { uint32 i; }
	struct Parent { Child[] children; }

	Child[] sChildren;
	Parent[] sParents;

	function test() public {

		sParents.push(Parent(sChildren)); // Not yet supported
	}
}
// ----
// TypeError 1598: (149-162): Source struct cannot be pushed to target struct array struct C.Parent[] storage ref. E.g. beuase push memory struct array to storage struct array not yet implemented.