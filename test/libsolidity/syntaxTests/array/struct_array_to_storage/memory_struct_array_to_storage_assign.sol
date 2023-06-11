contract C {
	struct Child { uint32 i; }
	struct Parent { Child[] children; }

	Child sChild;
	Child[] sChildren;
	Child[] sChildren1;
	Parent sParent;
	Parent[] sParents;

	function test() public {
		Child memory child;
		Child memory child1;
		Child[] memory children;
		Child[] memory children1;
		Parent memory parent;
		Parent[] memory parents;

		child1 = child; // OK
		sChild = child; // OK

		children1 = children; // OK
		children = sChildren; // OK
		sChildren = children; // Not yet supported
		sChildren1 = sChildren; // OK

		sParent.children = children; // Not yet supported
		sParent.children = sChildren; // OK


		parent = Parent(children); // OK
		parents[0] = Parent(children); // OK

		sParent = Parent(sChildren); // Not yet supported
		sParent = Parent(children); // Not yet supported
		sParent = Parent(sChildren); // Not yet supported
		sParents[0] = Parent(sChildren); // Not yet supported
	}
}
// ----
// TypeError 7407: (474-482): Type struct C.Child[] memory is not implicitly convertible to expected type struct C.Child[] storage ref. Copy of non-storage struct array to storage array is not yet supported.
// TypeError 7407: (559-567): Type struct C.Child[] memory is not implicitly convertible to expected type struct C.Child[] storage ref. Copy of non-storage struct array to storage array is not yet supported.
// TypeError 7407: (717-734): Type struct C.Parent memory is not implicitly convertible to expected type struct C.Parent storage ref. Copy to storage struct array from non-storage array is not yet supported.
// TypeError 7407: (769-785): Type struct C.Parent memory is not implicitly convertible to expected type struct C.Parent storage ref. Copy to storage struct array from non-storage array is not yet supported.
// TypeError 7407: (820-837): Type struct C.Parent memory is not implicitly convertible to expected type struct C.Parent storage ref. Copy to storage struct array from non-storage array is not yet supported.
// TypeError 7407: (876-893): Type struct C.Parent memory is not implicitly convertible to expected type struct C.Parent storage ref. Copy to storage struct array from non-storage array is not yet supported.