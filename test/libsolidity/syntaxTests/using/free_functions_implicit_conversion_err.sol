struct S {
    uint8 x;
}

function id(uint16 x) pure returns(uint16) {
    return x;
}
contract C {
    using {id} for uint256;
    using {id} for S;
}
// ----
// TypeError 3100: (112-114): The function "id" cannot be bound to the type "uint256" because the type cannot be implicitly converted to the first argument of the function ("uint16").
// TypeError 3100: (140-142): The function "id" cannot be bound to the type "S" because the type cannot be implicitly converted to the first argument of the function ("uint16").
