contract C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    function f(StructType memory StructType) external {}
    function g(EnumType EnumType) external {}
    function h(EnumType StructType, StructType memory EnumType) external {}

    function z(EnumType e) external returns (uint EnumType) {}
}
// ----
// Warning 2519: (104-132='StructType memory StructType'): This declaration shadows an existing declaration.
// Warning 2519: (161-178='EnumType EnumType'): This declaration shadows an existing declaration.
// Warning 2519: (207-226='EnumType StructType'): This declaration shadows an existing declaration.
// Warning 2519: (228-254='StructType memory EnumType'): This declaration shadows an existing declaration.
// Warning 2519: (314-327='uint EnumType'): This declaration shadows an existing declaration.
// TypeError 5172: (104-114='StructType'): Name has to refer to a struct, enum or contract.
