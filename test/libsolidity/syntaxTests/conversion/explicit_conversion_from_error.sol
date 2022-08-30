interface MyInterface {
    enum MyEnum { E1, E2 }
    error CustomError1(
        uint256,
        bool,
        bool[],
        address payable,
        MyInterface,
        MyEnum,
        function (string memory) external returns (uint)
    );
}

contract Test {
    function testFunction(string memory) external returns (uint) {}

    function test() public {
        MyInterface instance = MyInterface(msg.sender);
        bool[] calldata arr;
        address payable addr;
        bytes4(MyInterface.CustomEror1);
        bytes4(MyInterface.CustomError1());
        bytes4(MyInterface.CustomError1(1, true, arr, addr, instance, MyInterface.MyEnum.E1, this.testFunction));
        address(MyInterface.CustomError1);
    }
}
// ----
// TypeError 9582: (495-518): Member "CustomEror1" not found or not visible after argument-dependent lookup in type(contract MyInterface).
