type T is uint;

contract C {
    struct S { T x; }
    type T is bytes32; // Shadows the file-level type despite the order

    struct R { uint y; }
    uint y; // No shadowing (struct members are not visible as unqualified names)

    bytes32 b;
    S s = S(T.wrap(b));
}
// ----
// Warning 2519: (56-74): This declaration shadows an existing declaration.
