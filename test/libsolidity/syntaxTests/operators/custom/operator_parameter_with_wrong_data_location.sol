struct S { uint128 x; }

using {add as +} for S;

function add(S memory, S storage) returns (S memory) {
    return S(0);
}

// ----
// TypeError 1884: (32-35): The function "add" needs to have two parameters of equal type to be used for the operator +.
