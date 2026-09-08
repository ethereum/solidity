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
        canary.value = type(uint256).max; // Should not be overwritten
    }

    function fillArray() public {
        // Fill testArray to exactly 96 bytes (3 slots in its data area)
        for (uint i = 0; i < 96; i++) {
            testArray.push(bytes1(uint8(i + 1)));
        }
    }

    function shrinkArray() public returns (uint256) {
        // Shrink from 96 to 50 bytes. Should clear slot 2 without touching canary
        bytes memory newData = new bytes(50);
        for (uint i = 0; i < 50; i++) {
            newData[i] = bytes1(uint8(i + 2));
        }
        testArray = newData;
        return testArray.length;
    }

    function canaryValue() public view returns (uint256) {
        return getCanary().value;
    }

    function arrayLength() public view returns (uint256) {
        return testArray.length;
    }

    function getDataSlotContent(uint256 index) public view returns (bytes32 value) {
        assembly {
            mstore(0, testArray.slot)
            let testArrayDataArea := keccak256(0, 0x20)
            let slot := add(testArrayDataArea, index)
            value := sload(slot)
        }
        return value;
    }

    function checkSlots() public view returns (bytes32, bytes32, bytes32, uint256, uint256) {
        return (
            getDataSlotContent(0),  // First data slot
            getDataSlotContent(1),  // Second data slot (partial cleanup expected)
            getDataSlotContent(2),  // Third data slot (should be cleared after shrink)
            canaryValue(),          // Canary value (should never change)
            arrayLength()           // Current array length
        );
    }

    function getSlot1LastBytes() public view returns (bytes14 lastBytes) {
        // Get the last 14 bytes of slot 1, which should be zero after partial cleanup
        bytes32 slot1 = getDataSlotContent(1);
        assembly {
            // Shift left by 18 bytes (144 bits) to move the last 14 bytes to the front
            lastBytes := shl(144, slot1)
        }
        return lastBytes;
    }

    function getArrayBytes(uint256 start, uint256 count) public view returns (bytes memory) {
        bytes memory result = new bytes(count);
        for (uint i = 0; i < count && start + i < testArray.length; i++) {
            result[i] = testArray[start + i];
        }
        return result;
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// arrayLength() -> 0
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// fillArray()
// gas irOptimized: 197349
// gas legacy: 220574
// gas legacyOptimized: 206839
// gas ssaCFGOptimized: 197537
// arrayLength() -> 96
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getArrayBytes(uint256,uint256): 0, 5 -> 0x20, 5, 0x0102030405000000000000000000000000000000000000000000000000000000
// getArrayBytes(uint256,uint256): 32, 5 -> 0x20, 5, 0x2122232425000000000000000000000000000000000000000000000000000000
// getArrayBytes(uint256,uint256): 64, 5 -> 0x20, 5, 0x4142434445000000000000000000000000000000000000000000000000000000
// shrinkArray() -> 50
// arrayLength() -> 50
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// getArrayBytes(uint256,uint256): 0, 5 -> 0x20, 5, 0x0203040506000000000000000000000000000000000000000000000000000000
// getArrayBytes(uint256,uint256): 32, 5 -> 0x20, 5, 0x2223242526000000000000000000000000000000000000000000000000000000
// getArrayBytes(uint256,uint256): 45, 5 -> 0x20, 5, 0x2f30313233000000000000000000000000000000000000000000000000000000
// getSlot1LastBytes() -> 0
// getDataSlotContent(uint256): 2 -> 0
// canaryValue() -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
