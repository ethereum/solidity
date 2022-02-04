contract C {
    string s;
    function f(string calldata c, string calldata c1) public {
        string memory a;
        bytes16 b;
        uint8[] memory num;
        bytes1[] memory m;
        string memory d = string.concat(a, b, c, num, s, "abc", m, c1, bytes(c1));
    }
}
// ----
// TypeError 9977: (232-233): Invalid type for argument in the string.concat function call. string type is required, but t_bytes16 provided.
// TypeError 9977: (238-241): Invalid type for argument in the string.concat function call. string type is required, but t_array$_t_uint8_$dyn_memory_ptr provided.
// TypeError 9977: (253-254): Invalid type for argument in the string.concat function call. string type is required, but t_array$_t_bytes1_$dyn_memory_ptr provided.
// TypeError 9977: (260-269): Invalid type for argument in the string.concat function call. string type is required, but t_bytes_calldata_ptr provided.
