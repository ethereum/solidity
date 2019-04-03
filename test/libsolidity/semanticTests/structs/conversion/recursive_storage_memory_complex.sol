contract CopyTest {
	struct Tree {
		uint256 data;
		Tree[] children;
	}
	Tree storageTree;

	constructor() public {
		storageTree.data = 0x42;
		storageTree.children.length = 2;
		storageTree.children[0].data = 0x4200;
		storageTree.children[1].data = 0x4201;
		storageTree.children[0].children.length = 3;
		for (uint i = 0; i < 3; i++)
		    storageTree.children[0].children[i].data = 0x420000 + i;
		storageTree.children[1].children.length = 4;
		for (uint i = 0; i < 4; i++)
		    storageTree.children[1].children[i].data = 0x420100 + i;
	}

	function countData(Tree memory tree) internal returns (uint256 c) {
	    c = 1;
	    for (uint i = 0; i < tree.children.length; i++) {
	        c += countData(tree.children[i]);
	    }
	}

	function copyFromTree(Tree memory tree, uint256[] memory data, uint256 offset) internal returns (uint256) {
	    data[offset++] = tree.data;
	    for (uint i = 0; i < tree.children.length; i++) {
	        offset = copyFromTree(tree.children[i], data, offset);
	    }
	    return offset;
	}

	function run() public returns (uint256[] memory) {
		Tree memory memoryTree;
		memoryTree = storageTree;
		uint256 length = countData(memoryTree);
		uint256[] memory result = new uint256[](length);
		copyFromTree(memoryTree, result, 0);
		return result;
	}
}
// ----
// run() -> 0x20, 10, 0x42, 0x4200, 0x420000, 0x420001, 0x420002, 0x4201, 0x420100, 0x420101, 0x420102, 0x420103
