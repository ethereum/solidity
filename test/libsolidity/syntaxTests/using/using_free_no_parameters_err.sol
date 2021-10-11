function one() pure returns(uint) {
    return 1;
}

using {one} for uint;
// ----
// TypeError 4731: (60-63): The function "one" does not have any parameters, and therefore cannot be bound to the type "uint256".
