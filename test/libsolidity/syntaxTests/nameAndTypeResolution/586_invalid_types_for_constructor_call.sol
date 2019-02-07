contract C {
    constructor(bytes32 _arg) public {
    }
}

contract A {
    function f() public {
        new  C((1234));
    }
}
// ----
// TypeError: (115-121): Invalid type for argument in function call. Invalid implicit conversion from int_const 1234 to bytes32 requested.
