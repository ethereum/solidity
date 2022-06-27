error MyCustomError(uint, bool);

contract C {
    function f() public returns (uint8, uint8, int) {
        return ((MyCustomError, 8, MyCustomError));
    }
}

// ----
// TypeError 5992: (116-151): Return argument type tuple(error MyCustomError(uint256,bool),int_const 8,error MyCustomError(uint256,bool)) is not implicitly convertible to expected type tuple(uint8,uint8,int256).
