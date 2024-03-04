contract C {
    constructor() payable {}

    function terminate() external {
        // NOTE: The contract `c` should still exist in the test below
        // when the call to selfdestruct is executed in a transaction
        // different from the one in which the contract was created.
        // However, it should still send all Ether in `c` to the beneficiary.
        selfdestruct(payable(msg.sender));
        assert(false);
    }
}

contract D {
    C public c;

    constructor() payable {}

    function deploy_create() public payable {
        c = new C{value: 1 ether}();
    }

    function deploy_create2() public payable {
        c = new C{value: 1 ether, salt: hex"1234"}();
    }

    function terminate() public {
        // NOTE: A second call to `c.terminate()` or any other function of the contract `c` will succeed if the
        // previous selfdestruct was performed in a different transaction, since the contract will still exists.
        c.terminate();
    }

    function test_create_and_terminate() public {
        deploy_create();
        assert(exists());
        test_balance_after_create();
        terminate();
        test_balance_after_selfdestruct();
    }

    function test_create2_and_terminate() public {
        deploy_create2();
        assert(exists());
        test_balance_after_create();
        terminate();
        test_balance_after_selfdestruct();
    }

    function test_balance_after_create() public view {
        assert(address(this).balance == 0);
        assert(address(c).balance == 1 ether);
    }

    function test_balance_after_selfdestruct() public view {
        assert(address(this).balance == 1 ether);
        assert(address(c).balance == 0);
    }

    function exists() public view returns (bool) {
        return address(c).code.length != 0;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// constructor(), 1 ether ->
// gas irOptimized: 455954
// gas legacy: 375495
// gas legacyOptimized: 507630
// exists() -> false
// test_create_and_terminate() ->
// exists() -> false
// terminate() -> FAILURE
// deploy_create() ->
// test_balance_after_create() ->
// exists() -> true
// terminate() ->
// test_balance_after_selfdestruct() ->
// exists() -> true
// test_create2_and_terminate() ->
// exists() -> false
// deploy_create2() ->
// test_balance_after_create() ->
// exists() -> true
// terminate() ->
// test_balance_after_selfdestruct() ->
// exists() -> true
// terminate() ->
