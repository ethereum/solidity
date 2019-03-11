contract C {
    function f(uint[] calldata s1, uint[] calldata s2, bool which) external pure returns (uint256 a, uint256 b, uint256 c) {
        uint256 memPtr_before = 0;
        assembly {
            memPtr_before := mload(0x40)
        }

        uint[] calldata l = which ? s1 : s2;

        a = l.length;
        b = l[0];
        c = l[1];

        uint256 memPtr_after = 0;
        assembly {
            memPtr_after := mload(0x40)
        }

        assert(memPtr_before == memPtr_after);
    }
}
// ----
// f(uint256[],uint256[],bool): 96, 192, 1, 2, 23, 42, 2, 27, 99 -> 2, 23, 42
// f(uint256[],uint256[],bool): 96, 192, 0, 2, 23, 42, 2, 27, 99 -> 2, 27, 99
