contract CopyTest {
	struct Tree {
		Tree[] children;
	}
	Tree storageTree;

	constructor() public {
		storageTree.children.length = 2;
		storageTree.children[0].children.length = 23;
		storageTree.children[1].children.length = 42;
	}

	function run() public returns (uint256, uint256, uint256) {
		Tree memory memoryTree;
		memoryTree = storageTree;
		return (memoryTree.children.length, memoryTree.children[0].children.length, memoryTree.children[1].children.length);
	}
}
// ----
// run() -> 2, 23, 42
