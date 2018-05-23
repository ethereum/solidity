contract c {
    enum validEnum { Value1, Value2, Value3, Value4 }
    function c() {
        a = validEnum.Value3;
    }
    validEnum a;
}
// ----
// Warning: (71-121): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (71-121): No visibility specified. Defaulting to "public". 
