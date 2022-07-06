contract C {
    event MyCustomEvent(uint);
    function f() pure public {
        bytes.concat(MyCustomEvent, MyCustomEvent);
    }
}
// ----
// TypeError 8015: (96-109): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but event MyCustomEvent(uint256) provided.
// TypeError 8015: (111-124): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but event MyCustomEvent(uint256) provided.
