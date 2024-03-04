contract C {
    bytes32 public genesisHash;
    bytes32 public currentHash;
    constructor() {
        require(block.number == 1);
        genesisHash = blockhash(0);
        currentHash = blockhash(1);
    }
    function f(uint blockNumber) public returns (bytes32) {
        return blockhash(blockNumber);
    }
}
// ----
// constructor()
// gas irOptimized: 80338
// gas irOptimized code: 27800
// gas legacy: 83571
// gas legacy code: 68600
// gas legacyOptimized: 80338
// gas legacyOptimized code: 26400
// genesisHash() -> 0x3737373737373737373737373737373737373737373737373737373737373737
// currentHash() -> 0
// f(uint256): 0 -> 0x3737373737373737373737373737373737373737373737373737373737373737
// f(uint256): 1 -> 0x3737373737373737373737373737373737373737373737373737373737373738
// f(uint256): 255 -> 0x00
// f(uint256): 256 -> 0x00
// f(uint256): 257 -> 0x00
