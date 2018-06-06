contract Interface {
    enum MyEnum { One, Two }
}
contract Impl {
    function test() public returns (Interface.MyEnum) {
        return Interface.MyEnum.One;
    }
}
// ----
// Warning: (72-166): Function state mutability can be restricted to pure
