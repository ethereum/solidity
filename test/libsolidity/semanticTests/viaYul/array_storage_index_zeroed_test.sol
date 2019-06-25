contract C {
	uint[] storageArray;
	function test_zeroed_indicies(uint256 len) public
	{
		storageArray.length = len;

		for (uint i = 0; i < len; i++)
			storageArray[i] = i + 1;

		if (len > 3)
		{
			storageArray.length = 3;

			for (uint i = 3; i < len; i++)
			{
				assembly {
					mstore(0, storageArray_slot)
					let pos := add(keccak256(0, 0x20), i)

					if iszero(eq(sload(pos), 0)) {
						revert(0, 0)
					}
				}
			}

		}

		storageArray.length = 0;
		storageArray.length = len;

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
// ====
// compileViaYul: true
// ----
// test_zeroed_indicies(uint256): 1 ->
// test_zeroed_indicies(uint256): 5 ->
// test_zeroed_indicies(uint256): 10 ->
// test_zeroed_indicies(uint256): 15 ->
// test_zeroed_indicies(uint256): 0xFF ->
