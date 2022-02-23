contract c {
    function bignum() public {
        uint256 a;
        a = 1e1233 / 1e1233; // 1e1233 is still fine
        a = 1e1234; // 1e1234 is too big
    }
}
// ----
// TypeError 2826: (128-134): Invalid literal value.
