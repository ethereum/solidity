contract C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    function f() external returns (StructType memory StructType) {}
    function g() external returns (EnumType EnumType) {}
    function h() external returns (EnumType StructType, StructType memory EnumType) {}

    function z(uint EnumType) external returns (EnumType e) {}
}
// ----
// Warning 2519: (124-152='StructType memory StructType'): This declaration shadows an existing declaration.
// Warning 2519: (192-209='EnumType EnumType'): This declaration shadows an existing declaration.
// Warning 2519: (249-268='EnumType StructType'): This declaration shadows an existing declaration.
// Warning 2519: (270-296='StructType memory EnumType'): This declaration shadows an existing declaration.
// Warning 2519: (317-330='uint EnumType'): This declaration shadows an existing declaration.
// TypeError 5172: (124-134='StructType'): Name has to refer to a struct, enum or contract.
