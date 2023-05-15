contract C {
    function test(bytes memory buf) public view returns (bool same, bool inplaceDecoded) {
        (uint256[] memory arr1, uint256[] memory arr2) = abi.decode(buf, (uint256[],uint256[]));
        assembly {
            // Check whether arr1 and arr2 end up at the same memory location.
            // This used to be the case, if both tail pointers in buf pointed to the
            // same memory region, i.e. this used to be false in the first two, but true
            // in the last three calls below. The desired behaviour is to always get distinct
            // memory regions, i.e. this should be false.
            same := eq(arr1, arr2)
            // Check whether (given the particular tail pointer of 0x40 for arr1 in the calls below)
            // arr1 points to the part of buf containing the encoding of arr1.
            // The position of the encoding of arr1 in buf is at offset 0x20 (length) + 0x40 (tail pointer)
            // of buf.
            // This used to be the case for all the test calls below, whereas now arr1 is always copied
            // from buf to a new memory area. Should always be false.
            inplaceDecoded := eq(arr1, add(buf, 0x60))
        }
    }
}
// ----
// test(bytes): 0x20, 0x80, 0x40, 0x60, 0, 0 -> false, false
// test(bytes): 0x20, 0xC0, 0x40, 0x80, 1, 0x42, 1, 0x42 -> false, false
// test(bytes): 0x20, 0x80, 0x40, 0x40, 1, 0x42 -> false, false
// test(bytes): 0x20, 0x60, 0x40, 0x40, 0 -> false, false
// test(bytes): 0x20, 0x80, 0x40, 0x40, 1, 0x42 -> false, false
