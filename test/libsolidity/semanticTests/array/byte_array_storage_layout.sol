contract c {
    bytes data;
    function test_short() public returns (uint256 r) {
        assembly {
            sstore(data.slot, 0)
        }
        for (uint8 i = 0; i < 15; i++) {
            data.push(bytes1(i));
        }
        assembly {
            r := sload(data.slot)
        }
    }

    function test_long() public returns (uint256 r) {
        assembly {
            sstore(data.slot, 0)
        }
        for (uint8 i = 0; i < 33; i++) {
            data.push(bytes1(i));
        }
        assembly {
            r := sload(data.slot)
        }
    }

    function test_pop() public returns (uint256 r) {
        assembly {
            sstore(data.slot, 0)
        }
        for (uint8 i = 0; i < 32; i++) {
            data.push(bytes1(i));
        }
        data.pop();
        data.pop();
        assembly {
            r := sload(data.slot)
        }
    }
}
// ====
// compileViaYul: also
// ----
// storageEmpty -> 1
// test_short() -> 1780731860627700044960722568376587075150542249149356309979516913770823710
// gas legacy: 59838
// gas legacyOptimized: 58291
// storageEmpty -> 0
// test_long() -> 67
// gas irOptimized: 89148
// gas legacy: 103590
// gas legacyOptimized: 100341
// storageEmpty -> 0
// test_pop() -> 1780731860627700044960722568376592200742329637303199754547598369979433020
// gas legacy: 61930
// gas legacyOptimized: 58714
// storageEmpty -> 0
