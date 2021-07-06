library C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    function f1(function (StructType memory StructType) external f) external {}
    function f2(function (EnumType EnumType) external f) external {}
    function f3(function (EnumType StructType, StructType memory EnumType) external f) external {}
}
// ----
// Warning 6162: (114-142): Naming function type parameters is deprecated.
// Warning 6162: (194-211): Naming function type parameters is deprecated.
// Warning 6162: (263-282): Naming function type parameters is deprecated.
// Warning 6162: (284-310): Naming function type parameters is deprecated.
// Warning 2519: (114-142): This declaration shadows an existing declaration.
// Warning 2519: (194-211): This declaration shadows an existing declaration.
// Warning 2519: (263-282): This declaration shadows an existing declaration.
// Warning 2519: (284-310): This declaration shadows an existing declaration.
