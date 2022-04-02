contract C {
    struct S {
        uint x;
    }

    enum E {A, B, C}

    mapping(uint => E) m;

    function f() public {
        bool b;
        uint u;
        uint8 u8;
        address a;
        address payable ap;
        function () external fext;
        function () internal fint;
        uint[] memory uDynamic;
        uint[2] memory uStatic;
        C c;
        S memory s;
        E e;

        string.concat(b, u, u8, a, ap, fext, fint, uDynamic, uStatic, c, s, e, m);
    }
}
// ----
// TypeError 9977: (426-427='b'): Invalid type for argument in the string.concat function call. string type is required, but t_bool provided.
// TypeError 9977: (429-430='u'): Invalid type for argument in the string.concat function call. string type is required, but t_uint256 provided.
// TypeError 9977: (432-434='u8'): Invalid type for argument in the string.concat function call. string type is required, but t_uint8 provided.
// TypeError 9977: (436-437='a'): Invalid type for argument in the string.concat function call. string type is required, but t_address provided.
// TypeError 9977: (439-441='ap'): Invalid type for argument in the string.concat function call. string type is required, but t_address_payable provided.
// TypeError 9977: (443-447='fext'): Invalid type for argument in the string.concat function call. string type is required, but t_function_external_nonpayable$__$returns$__$ provided.
// TypeError 9977: (449-453='fint'): Invalid type for argument in the string.concat function call. string type is required, but t_function_internal_nonpayable$__$returns$__$ provided.
// TypeError 9977: (455-463='uDynamic'): Invalid type for argument in the string.concat function call. string type is required, but t_array$_t_uint256_$dyn_memory_ptr provided.
// TypeError 9977: (465-472='uStatic'): Invalid type for argument in the string.concat function call. string type is required, but t_array$_t_uint256_$2_memory_ptr provided.
// TypeError 9977: (474-475='c'): Invalid type for argument in the string.concat function call. string type is required, but t_contract$_C_$86 provided.
// TypeError 9977: (477-478='s'): Invalid type for argument in the string.concat function call. string type is required, but t_struct$_S_$4_memory_ptr provided.
// TypeError 9977: (480-481='e'): Invalid type for argument in the string.concat function call. string type is required, but t_enum$_E_$8 provided.
// TypeError 9977: (483-484='m'): Invalid type for argument in the string.concat function call. string type is required, but t_mapping$_t_uint256_$_t_enum$_E_$8_$ provided.
