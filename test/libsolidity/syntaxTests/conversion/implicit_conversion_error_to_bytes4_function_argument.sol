interface MyInterface {
    error MyCustomError(uint256, bool);
}

contract MyContract {
    function f(bytes4 arg) public {}
    function test() public {
        f(MyInterface.MyCustomError);
    }
}
// ----
//  TypeError 9553: (165-190): Invalid type for argument in function call. Invalid implicit conversion from error MyCustomError(uint256,bool) to bytes4 requested.
