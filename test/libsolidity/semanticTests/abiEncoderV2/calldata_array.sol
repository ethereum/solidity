pragma experimental ABIEncoderV2;

contract C {
    function g(uint256[] calldata) external pure returns (bytes memory) {
        return msg.data;
    }
    function f(uint256[][1] calldata s) external view returns (bool) {
        bytes memory a = this.g(s[0]);
        uint256[] memory m = s[0];
        bytes memory b = this.g(m);
        assert(a.length == b.length);
        for (uint i = 0; i < a.length; i++)
            assert(a[i] == b[i]);
        return true;
    }
}
// ====
// EVMVersion: >homestead
// ----
// f(uint256[][1]): 32, 32, 0 -> true
// f(uint256[][1]): 32, 32, 1, 42 -> true
// f(uint256[][1]): 32, 32, 8, 421, 422, 423, 424, 425, 426, 427, 428 -> true
