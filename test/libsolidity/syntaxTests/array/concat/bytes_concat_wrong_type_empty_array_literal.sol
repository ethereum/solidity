contract C {
    function f() public {
        bytes.concat([], [], []);
    }
}
// ----
// TypeError 8015: (60-62): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but inline_array() provided.
// TypeError 8015: (64-66): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but inline_array() provided.
// TypeError 8015: (68-70): Invalid type for argument in the bytes.concat function call. bytes or fixed bytes type is required, but inline_array() provided.
