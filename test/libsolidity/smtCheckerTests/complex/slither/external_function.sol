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
// ====
// SMTEngine: all
// ----
// Warning 9302: (727-782): Return value of low-level calls not used.
// Warning 2072: (84-93): Unused local variable.
// Warning 2072: (227-236): Unused local variable.
// Warning 2072: (634-643): Unused local variable.
// Warning 2018: (42-104): Function state mutability can be restricted to pure
// Warning 2018: (185-247): Function state mutability can be restricted to pure
// Warning 2018: (437-506): Function state mutability can be restricted to pure
// Warning 2018: (1111-1173): Function state mutability can be restricted to pure
// Warning 2018: (1179-1241): Function state mutability can be restricted to pure
// Warning 2018: (1247-1309): Function state mutability can be restricted to pure
// Warning 8729: (681-716): Contract deployment is only supported in the trusted mode for external calls with the CHC engine.
// Warning 8729: (854-886): Contract deployment is only supported in the trusted mode for external calls with the CHC engine.
// Warning 5729: (1370-1375): BMC does not yet implement this type of function call.
