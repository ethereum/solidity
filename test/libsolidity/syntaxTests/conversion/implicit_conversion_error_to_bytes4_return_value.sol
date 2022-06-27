interface MyInterface {
    error MyCustomError(uint256, bool);
}

contract Test {
    function test() public returns(bytes4) {
        return (MyInterface.MyCustomError);
    }
}
// ----
// TypeError 6359: (143-170): Return argument type error MyCustomError(uint256,bool) is not implicitly convertible to expected type (type of first return variable) bytes4.
