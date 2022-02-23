contract DepositContract {
    uint constant MAX_DEPOSIT_COUNT = 2**32 - 1;

    uint256 deposit_count;
    uint256 deposit_count_2;

    function deposit() external {
        require(deposit_count < MAX_DEPOSIT_COUNT);
        deposit_count += 1;
        deposit_count_2 += 10; // should fail
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (256-277): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 2661: (256-277): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
