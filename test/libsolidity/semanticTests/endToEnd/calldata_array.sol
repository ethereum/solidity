pragma experimental ABIEncoderV2;
contract C {
    function f(uint[2] calldata s) external pure returns(uint256 a, uint256 b) {
        a = s[0];
        b = s[1];
    }
}

// ----
// f(uint256[2]): encodeArgs(42), 23)) -> 42, 23
// f(uint256[2]):"[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2a,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,17]" -> "42, 23"
