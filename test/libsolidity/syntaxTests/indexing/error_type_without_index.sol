interface MyInterface {
    error MyCustomError(uint256, bool);
}

contract MyContract {
    function test() public {
        MyInterface.MyCustomError[];
    }
}
// ----
// TypeError 2614: (126-151): Indexed expression has to be a type, mapping or array (is error MyCustomError(uint256,bool))
