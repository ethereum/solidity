contract Factory {
    event Deployed(address, bytes32);

    function deploy(bytes32 _salt) external payable returns (address implAddr) {
        // NOTE: The bytecode of contract C is used here instead of `type(C).creationCode` since the address calculation depends on the precise init code
        // and that will change in our test framework between legacy and via-IR codegen and via optimized vs non-optimized.
        //contract C {
        //    constructor() payable {}
        //    function terminate() external {
        //        selfdestruct(payable(msg.sender));
        //    }
        //}
        bytes memory initCode =
            hex"6080806040526068908160108239f3fe6004361015600b575f80fd5b5f3560e0"
            hex"1c630c08bf8814601d575f80fd5b34602e575f366003190112602e5733ff5b5f"
            hex"80fdfea2646970667358221220fe3c4fe66c1838016e2efdc5b65538e5ff3dbf"
            hex"ced7eff135da3556db4bd841aa64736f6c63430008180033";

        address target = address(uint160(uint256(keccak256(abi.encodePacked(
            bytes1(0xff),
            address(this),
            _salt,
            keccak256(abi.encodePacked(initCode))
        )))));

        assembly {
            implAddr := create2(callvalue(), add(initCode, 0x20), mload(initCode), _salt)
            if iszero(extcodesize(implAddr)) {
                revert(0, 0)
            }
        }
        assert(address(implAddr) == target);
        emit Deployed(implAddr, _salt);
    }
}

interface IC {
    function terminate() external;
}

contract D {
    Factory public factory = new Factory();
    IC public c;

    constructor() payable {}

    function deploy_create2() public payable {
        c = IC(factory.deploy{value: 1 ether}(hex"1234"));
    }

    function terminate() public {
        c.terminate();
    }

    function test_deploy_and_terminate() public {
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
// gas irOptimized: 133342
// gas irOptimized code: 298400
// gas legacy: 151644
// gas legacy code: 538600
// gas legacyOptimized: 131799
// gas legacyOptimized code: 281000
// exists() -> false
// test_deploy_and_terminate() ->
// ~ emit Deployed(address,bytes32) from 0x137aa4dfc0911524504fcd4d98501f179bc13b4a: 0x7e6580007e709ac52945fae182c61131d42634e8, 0x1234000000000000000000000000000000000000000000000000000000000000
// gas irOptimized: 96823
// gas irOptimized code: 20800
// gas legacy: 98095
// gas legacy code: 20800
// gas legacyOptimized: 96337
// gas legacyOptimized code: 20800
// exists() -> false
// deploy_create2() ->
// ~ emit Deployed(address,bytes32) from 0x137aa4dfc0911524504fcd4d98501f179bc13b4a: 0x7e6580007e709ac52945fae182c61131d42634e8, 0x1234000000000000000000000000000000000000000000000000000000000000
// test_balance_after_create() ->
// exists() -> true
// terminate() ->
// test_balance_after_selfdestruct() ->
// exists() -> false
// deploy_create2() ->
// ~ emit Deployed(address,bytes32) from 0x137aa4dfc0911524504fcd4d98501f179bc13b4a: 0x7e6580007e709ac52945fae182c61131d42634e8, 0x1234000000000000000000000000000000000000000000000000000000000000
