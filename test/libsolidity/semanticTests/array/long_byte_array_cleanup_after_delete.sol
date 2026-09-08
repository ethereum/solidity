contract C {
    bytes testArray;

    struct Canary {
        uint256 value;
    }

    function getCanary() internal pure returns (Canary storage canary) {
        assembly {
            mstore(0, testArray.slot)
            let testArrayDataArea := keccak256(0, 0x20)
            // testArray's data area occupies 3 slots when filled. Canary goes right after
            canary.slot := add(testArrayDataArea, 3)
        }
    }

    constructor() {
        Canary storage canary = getCanary();
        canary.value = type(uint256).max;
    }

    function getArrayDataAreaSlot() public pure returns (uint256 slot) {
        assembly {
            mstore(0, testArray.slot)
            slot := keccak256(0, 0x20)
        }
        assert(slot == 0x290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e563);
        return slot;
    }

    function getCanarySlot() public pure returns (uint256) {
        return getArrayDataAreaSlot() + 3;
    }

    function checkSlots() public view returns (uint256, uint256, uint256, uint256, uint256) {
        uint256 dataSlot = getArrayDataAreaSlot();
        uint256 slot0;
        uint256 slot1;
        uint256 slot2;
        uint256 slot3;
        assembly {
            slot0 := sload(dataSlot)
            slot1 := sload(add(dataSlot, 1))
            slot2 := sload(add(dataSlot, 2))
            slot3 := sload(add(dataSlot, 3))
        }
        return (slot0, slot1, slot2, slot3, testArray.length);
    }

    function fillArray() public {
        // Fill testArray to exactly 96 bytes (3 slots in its data area)
        for (uint i = 0; i < 96; i++) {
            testArray.push(bytes1(uint8(i + 1)));
        }
    }

    function deleteArray() public {
        // Should clear 3 slots without touching canary
        delete testArray;
    }

    function canaryValue() public view returns (uint256) {
        return getCanary().value;
    }
}
// ----
// getArrayDataAreaSlot() -> 0x290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e563
// getCanarySlot() -> 0x290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e566
// checkSlots() -> 0, 0, 0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// fillArray()
// gas irOptimized: 197286
// gas legacy: 220574
// gas legacyOptimized: 206839
// gas ssaCFGOptimized: 196707
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// deleteArray()
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
