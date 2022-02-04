contract C {
    function j() external {
        string memory a = "hello";
        string memory b = " world";

        string memory d = string.concat(bytes(a), bytes(b));
        string memory e = string.concat(a, 0);
    }
}
// ----
// TypeError 9977: (153-161): Invalid type for argument in the string.concat function call. string type is required, but t_bytes_memory_ptr provided.
// TypeError 9977: (163-171): Invalid type for argument in the string.concat function call. string type is required, but t_bytes_memory_ptr provided.
// TypeError 9977: (217-218): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
