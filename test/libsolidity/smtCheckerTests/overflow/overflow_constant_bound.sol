pragma experimental SMTChecker;

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
// ----
// Warning 1218: (289-310): CHC: Error trying to invoke SMT solver.
// Warning 4984: (289-310): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 2661: (289-310): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
