contract C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    function (StructType memory StructType) external ext1;
    function (EnumType EnumType) external ext2;
    function (EnumType StructType, StructType memory EnumType) external ext3;
}
// ----
// Warning 6162: (103-131): Naming function type parameters is deprecated.
// Warning 6162: (162-179): Naming function type parameters is deprecated.
// Warning 6162: (210-229): Naming function type parameters is deprecated.
// Warning 6162: (231-257): Naming function type parameters is deprecated.
// Warning 2519: (103-131): This declaration shadows an existing declaration.
// Warning 2519: (162-179): This declaration shadows an existing declaration.
// Warning 2519: (210-229): This declaration shadows an existing declaration.
// Warning 2519: (231-257): This declaration shadows an existing declaration.
