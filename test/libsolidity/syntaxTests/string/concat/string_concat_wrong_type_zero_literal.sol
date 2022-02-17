contract C {
    function f() public pure {
        string.concat(
            0,
            -0,
            0.0,
            -0.0,
            0e10,
            -0e10,
            0e-10,
            -0e-10,
            (0),
            0x00,
            -0x00,
            0x0000000000000000000000000000000000000000000000000000000000000000, // exactly 32 bytes
            -0x0000000000000000000000000000000000000000000000000000000000000000 // exactly 32 bytes
        );
    }
}
// ----
// TypeError 9977: (79-80): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (94-96): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (110-113): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (127-131): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (145-149): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (163-168): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (182-187): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (201-207): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (221-224): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (238-242): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (256-261): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (275-341): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
// TypeError 9977: (375-442): Invalid type for argument in the string.concat function call. string type is required, but t_rational_0_by_1 provided.
