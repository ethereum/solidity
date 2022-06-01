contract C {
    function f() public {
        string.concat([], [], []);
    }
}
// ----
// TypeError 9977: (61-63): Invalid type for argument in the string.concat function call. string type is required, but t_inline_array$__$ provided.
// TypeError 9977: (65-67): Invalid type for argument in the string.concat function call. string type is required, but t_inline_array$__$ provided.
// TypeError 9977: (69-71): Invalid type for argument in the string.concat function call. string type is required, but t_inline_array$__$ provided.
