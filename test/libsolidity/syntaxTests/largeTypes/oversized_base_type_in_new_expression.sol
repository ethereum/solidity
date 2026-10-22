contract C {
    function f() public {
        new uint256[57896044618658097711785492504343953926634992332820282019728792003956564819967][](1);
    }
}
// ----
// TypeError 9964: (51-139): Type too large for memory.
