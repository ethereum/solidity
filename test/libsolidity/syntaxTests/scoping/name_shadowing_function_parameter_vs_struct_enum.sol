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
// Warning 2519: (104-132): This declaration shadows an existing declaration.
// Warning 2519: (161-178): This declaration shadows an existing declaration.
// Warning 2519: (207-226): This declaration shadows an existing declaration.
// Warning 2519: (228-254): This declaration shadows an existing declaration.
// Warning 2519: (314-327): This declaration shadows an existing declaration.
// TypeError 5172: (104-114): Name has to refer to a user-defined value type, struct, enum or contract.
