error MyCustomError(uint, bool);

contract C {
    function f() pure public {
        bytes.concat(MyCustomError, MyCustomError);
    }
}
// ----
// TypeError 8015: (99-112): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but error MyCustomError(uint256,bool) provided.
// TypeError 8015: (114-127): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but error MyCustomError(uint256,bool) provided.
