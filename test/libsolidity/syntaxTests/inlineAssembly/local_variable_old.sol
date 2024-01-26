contract C {
    address someAddress = 0xCAfEcAfeCAfECaFeCaFecaFecaFECafECafeCaFe;
    bytes4 private constant _SOME_SELECTOR = bytes4(keccak256("hey()"));

    fallback() external {
        (bool success, bytes memory result) = someAddress.call(
            abi.encodePacked(_SOME_SELECTOR, msg.sender, uint8(0xff))
        );

        assembly {
            // `mload(result)` -> offset in memory where `result.length` is located
            // `add(result, 32)` -> offset in memory where `result` data starts
            let resultdata_size := mload(result)
            let resultdata_offset := add(result, 32)

            // if call failed, revert
            if eq(success, 0) {
                revert(resultdata_offset, resultdata_size)
            }

            // otherwise return the data returned by the external call
            return(resultdata_offset, resultdata_size)
        }
    }
}
// ----
