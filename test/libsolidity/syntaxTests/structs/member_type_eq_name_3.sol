contract C {
    enum E {a, b, c}
    struct S {function (E X) external f; uint E;}
    struct T {function (E T) external f; uint E;}
    struct U {function (E E) external f;}
}
// ----
// Warning 6162: (58-61): Naming function type parameters is deprecated.
// Warning 6162: (108-111): Naming function type parameters is deprecated.
// Warning 6162: (158-161): Naming function type parameters is deprecated.
// Warning 2519: (108-111): This declaration shadows an existing declaration.
// Warning 2519: (158-161): This declaration shadows an existing declaration.
