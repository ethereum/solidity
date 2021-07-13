contract CopyTest {
    struct Tree {
        Tree[] children;
    }

    Tree storageTree;
    Tree[] children;

    constructor() {
        for (uint i = 0; i < 2; i++)
            storageTree.children.push();
        for (uint i = 0; i < 23; i++)
            storageTree.children[0].children.push();
        for (uint i = 0; i < 42; i++)
            storageTree.children[1].children.push();
    }

    function run() public returns (uint256, uint256, uint256) {
        Tree memory memoryTree;
        memoryTree = storageTree;
        return (memoryTree.children.length, memoryTree.children[0].children.length, memoryTree.children[1].children.length);
    }
}
// ====
// compileViaYul: also
// ----
// run() -> 2, 23, 42
// gas irOptimized: 193170
// gas legacy: 186016
// gas legacyOptimized: 184668
