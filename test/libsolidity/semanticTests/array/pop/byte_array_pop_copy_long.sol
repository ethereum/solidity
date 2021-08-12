contract c {
    bytes data;

    function test() public returns (bytes memory) {
        for (uint256 i = 0; i < 33; i++) data.push(0x03);
        for (uint256 j = 0; j < 4; j++) data.pop();
        return data;
    }
}

// ====
// compileViaYul: also
// ----
// test() -> 0x20, 29, 0x0303030303030303030303030303030303030303030303030303030303000000
// gas irOptimized: 109503
// gas legacy: 127309
// gas legacyOptimized: 123625
