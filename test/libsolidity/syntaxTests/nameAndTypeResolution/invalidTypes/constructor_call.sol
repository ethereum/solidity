contract C {
    constructor(bytes32 _arg) {
    }
}

contract A {
    function f() public {
        new  C((1234));
    }
}
// ----
// TypeError 9553: (108-114): Invalid type for argument in function call. Invalid implicit conversion from int_const 1234 to bytes32 requested.
