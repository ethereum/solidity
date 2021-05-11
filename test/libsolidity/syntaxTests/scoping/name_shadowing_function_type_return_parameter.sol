contract C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    function () external returns (StructType memory StructType) ext1;
    function () external returns (EnumType EnumType) ext2;
    function () external returns (EnumType StructType, StructType memory EnumType) ext3;
}
// ----
// SyntaxError 7304: (123-151): Return parameters in function types may not be named.
// SyntaxError 7304: (193-210): Return parameters in function types may not be named.
// SyntaxError 7304: (252-271): Return parameters in function types may not be named.
// SyntaxError 7304: (273-299): Return parameters in function types may not be named.
// Warning 2519: (123-151): This declaration shadows an existing declaration.
// Warning 2519: (193-210): This declaration shadows an existing declaration.
// Warning 2519: (252-271): This declaration shadows an existing declaration.
// Warning 2519: (273-299): This declaration shadows an existing declaration.
