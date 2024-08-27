contract C {
    uint[] storageArray;
    function test_zeroed_indices(uint256 len) public
    {
        while(storageArray.length < len)
            storageArray.push();
        while(storageArray.length > len)
            storageArray.pop();

        for (uint i = 0; i < len; i++)
            storageArray[i] = i + 1;

        if (len > 3)
        {
            while(storageArray.length > 0)
                storageArray.pop();
            while(storageArray.length < 3)
                storageArray.push();

            for (uint i = 3; i < len; i++)
            {
                assembly {
                    mstore(0, storageArray.slot)
                    let pos := add(keccak256(0, 0x20), i)

                    if iszero(eq(sload(pos), 0)) {
                        revert(0, 0)
                    }
                }
            }

        }

        while(storageArray.length > 0)
            storageArray.pop();
        while(storageArray.length < len)
            storageArray.push();

        for (uint i = 0; i < len; i++)
        {
            require(storageArray[i] == 0);

            uint256 val = storageArray[i];
            uint256 check;

            assembly { check := iszero(val) }

            require(check == 1);
        }
    }
}
// ----
// test_zeroed_indices(uint256): 1 ->
// test_zeroed_indices(uint256): 5 ->
// gas irOptimized: 133763
// gas legacy: 131664
// gas legacyOptimized: 129990
// test_zeroed_indices(uint256): 10 ->
// gas irOptimized: 228556
// gas legacy: 225215
// gas legacyOptimized: 222351
// test_zeroed_indices(uint256): 15 ->
// gas irOptimized: 327360
// gas legacy: 322899
// gas legacyOptimized: 318907
// test_zeroed_indices(uint256): 0xFF ->
// gas irOptimized: 5180120
// gas legacy: 5093135
// gas legacyOptimized: 5020523
