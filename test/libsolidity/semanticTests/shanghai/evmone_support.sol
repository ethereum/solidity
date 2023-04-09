contract ShortReturn {
    constructor() {
        assembly {
            // return(0, 32)
            // PUSH1 0x20 PUSH0 RETURN
            mstore(0, hex"60205ff3")
            return(0, 4)
        }
    }
}

interface DoesItReturnZero {
    function foo() external pure returns (uint256);
}

contract Test {
    ShortReturn immutable shortReturn = new ShortReturn();
    function bytecode() external view returns(bytes memory) {
        return address(shortReturn).code;
    }
    function isPush0Supported() external view returns (bool) {
        assert(DoesItReturnZero(address(shortReturn)).foo() == 0);
        return true;
    }
}
// ====
// compileViaYul: also
// EVMVersion: >=shanghai
// ----
// bytecode() -> 0x20, 4, 0x60205ff300000000000000000000000000000000000000000000000000000000
// isPush0Supported() -> true
