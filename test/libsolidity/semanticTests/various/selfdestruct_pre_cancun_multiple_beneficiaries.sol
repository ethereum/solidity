contract C {
    constructor() payable {}
    function terminate(address _beneficiary) public {
        selfdestruct(payable(_beneficiary));
        assert(false);
    }
}

contract D {
    address account1 = payable(0x1111111111111111111111111111111111111111);
    address account2 = payable(0x2222222222222222222222222222222222222222);
    C public c;

    constructor() payable {}

    function deploy() public payable {
        c = new C{value: 1 ether}();
    }

    function terminate(address _beneficiary) public {
        c.terminate(_beneficiary);
    }

    function test_deploy_and_terminate_twice() public {
        deploy();
        terminate(account1);
        terminate(account2);
    }

    function exists() public view returns (bool) {
        return address(c).code.length != 0;
    }
}
// ====
// EVMVersion: <=shanghai
// ----
// constructor(), 2 ether ->
// gas irOptimized: 108104
// gas irOptimized code: 119200
// gas legacy: 120424
// gas legacy code: 253600
// gas legacyOptimized: 109015
// gas legacyOptimized code: 130800
// balance: 0x1111111111111111111111111111111111111111 -> 0
// balance: 0x2222222222222222222222222222222222222222 -> 0
// balance -> 2000000000000000000
// exists() -> false
// test_deploy_and_terminate_twice() ->
// gas irOptimized: 121395
// gas irOptimized code: 14000
// gas legacy: 122384
// gas legacy code: 43200
// gas legacyOptimized: 121596
// gas legacyOptimized code: 22800
// exists() -> false
// balance: 0x1111111111111111111111111111111111111111 -> 1000000000000000000
// balance: 0x2222222222222222222222222222222222222222 -> 0
// balance -> 1000000000000000000
// deploy() ->
// gas legacy: 58491
// gas legacy code: 43200
// exists() -> true
// balance: 0x1111111111111111111111111111111111111111 -> 1000000000000000000
// balance: 0x2222222222222222222222222222222222222222 -> 0
// balance -> 0
// terminate(address): 0x1111111111111111111111111111111111111111 ->
// balance: 0x1111111111111111111111111111111111111111 -> 2000000000000000000
// balance: 0x2222222222222222222222222222222222222222 -> 0
// balance -> 0
// terminate(address): 0x2222222222222222222222222222222222222222 -> FAILURE
// balance: 0x1111111111111111111111111111111111111111 -> 2000000000000000000
// balance: 0x2222222222222222222222222222222222222222 -> 0
// balance -> 0
// exists() -> false
