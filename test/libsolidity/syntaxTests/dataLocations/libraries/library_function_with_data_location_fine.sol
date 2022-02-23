library L {
    struct S { uint x; }
    function f(uint[] memory, uint[] storage, S storage) private pure
        returns (mapping(uint => uint) storage a, S memory b, uint[] storage c) { return (a, b, c); }
    function g(uint[] memory, uint[] storage) internal pure
        returns (mapping(uint => uint) storage a, S memory b, uint[] storage c) { return (a, b, c); }
    function h(uint[] memory, uint[] storage) public pure returns (S storage x) { return x; }
    function i(uint[] calldata, uint[] storage) external pure returns (S storage x) {return x; }
}
// ----
// TypeError 3464: (197-198): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (203-204): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (359-360): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (365-366): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (460-461): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (557-558): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
