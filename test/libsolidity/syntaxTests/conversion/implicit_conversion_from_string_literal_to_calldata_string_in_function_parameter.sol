contract C {
    function g(string calldata _s) public {}
    function h(bytes calldata _b) public {}

    function f() public {
        g("hello");
        g(unicode"hello");
        h(hex"68656c6c6f");
    }
}
// ----
// TypeError 9553: (139-146): Invalid type for argument in function call. Invalid implicit conversion from literal_string "hello" to string calldata requested.
// TypeError 9553: (159-173): Invalid type for argument in function call. Invalid implicit conversion from literal_string "hello" to string calldata requested.
// TypeError 9553: (186-201): Invalid type for argument in function call. Invalid implicit conversion from literal_string "hello" to bytes calldata requested.
