contract C {
    constructor() payable {}

    function terminate() external {
        // NOTE: The contract `c` should still exists in the test below,
        // since the call to the selfdestruct method was done in a tx that is
        // not the same tx that the contract was created.
        // However, it should send all Ether in `c` to the beneficiary.
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
// EVMVersion: =shanghai
// ----
// constructor(), 1 ether ->
// gas irOptimized: 242428
// gas legacy: 373563
// gas legacyOptimized: 234516
// exists() -> false
// test_create_and_terminate() ->
// exists() -> false
// terminate() -> FAILURE
// deploy_create() ->
// test_balance_after_create() ->
// exists() -> true
// terminate() ->
// test_balance_after_selfdestruct() ->
// exists() -> false
// test_create2_and_terminate() ->
// exists() -> false
// deploy_create2() ->
// test_balance_after_create() ->
// exists() -> true
// terminate() ->
// test_balance_after_selfdestruct() ->
// exists() -> false
// terminate() -> FAILURE
