pragma experimental ABIEncoderV2;
contract Test {
    function f(uint256[] calldata s) external { s[0] = 4; }
}
// ----
// TypeError 6182: (98-102): Calldata arrays are read-only.
