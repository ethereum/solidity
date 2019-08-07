pragma experimental SMTChecker;

contract ContractWithFunctionCalled {
    function funcCalled() external  {
        uint256 i = 0;
    }
}

contract ContractWithFunctionCalledSuper is ContractWithFunctionCalled {
    function callWithSuper() public {
        uint256 i = 0;
    }
}

contract ContractWithFunctionNotCalled {

    function funcNotCalled3() public {

    }

    function funcNotCalled2() public {

    }

    function funcNotCalled() public {

    }

    function my_func() internal returns(bool){
        return true;
    }

}

contract ContractWithFunctionNotCalled2 is ContractWithFunctionCalledSuper {
    function funcNotCalled() public {
        uint256 i = 0;
        address three = address(new ContractWithFunctionNotCalled());
        three.call(abi.encode(bytes4(keccak256("helloTwo()"))));
        super.callWithSuper();
        ContractWithFunctionCalled c = new ContractWithFunctionCalled();
        c.funcCalled();
    }
}

contract InternalCall {

    function() returns(uint) ptr;

    function set_test1() external{
        ptr = test1;
    }

    function set_test2() external{
        ptr = test2;
    }

    function test1() public returns(uint){
        return 1;
    }

    function test2() public returns(uint){
        return 2;
    }

    function test3() public returns(uint){
        return 3;
    }

    function exec() external returns(uint){
        return ptr();
    }

}
// ----
// Warning: (760-815): Return value of low-level calls not used.
// Warning: (117-126): Unused local variable.
// Warning: (260-269): Unused local variable.
// Warning: (667-676): Unused local variable.
// Warning: (75-137): Function state mutability can be restricted to pure
// Warning: (218-280): Function state mutability can be restricted to pure
// Warning: (470-539): Function state mutability can be restricted to pure
// Warning: (1144-1206): Function state mutability can be restricted to pure
// Warning: (1212-1274): Function state mutability can be restricted to pure
// Warning: (1280-1342): Function state mutability can be restricted to pure
// Warning: (782-813): Type conversion is not yet fully supported and might yield false positives.
// Warning: (771-814): Assertion checker does not yet implement this type of function call.
// Warning: (825-830): Assertion checker does not yet support the type of this variable.
// Warning: (1403-1408): Assertion checker does not yet implement this type of function call.
