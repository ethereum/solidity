contract C {
    struct S { uint x; }
    S s;
    struct T { uint y; }
    T t;
    function f() public view {
        bytes32 a = sha256(s, t);
        a;
    }
}
// ----
// Warning: (132-144): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// TypeError: (139-140): This type cannot be encoded.
// TypeError: (142-143): This type cannot be encoded.
