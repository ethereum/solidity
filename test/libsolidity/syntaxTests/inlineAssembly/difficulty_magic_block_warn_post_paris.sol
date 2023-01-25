contract C {
    function f() public view returns (uint256) {
        return block.difficulty;
    }
}
// ====
// EVMVersion: >=paris
// ----
// Warning 8417: (77-93): Since the VM version paris, "difficulty" was replaced by "prevrandao", which now returns a random number based on the beacon chain.
