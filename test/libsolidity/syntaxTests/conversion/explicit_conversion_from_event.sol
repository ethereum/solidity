interface MyInterface {
    enum MyEnum { E1, E2 }
}

contract Test {
    function testFunction(string memory) external returns (uint) {}

    event CustomEvent1(
        uint256,
        bool,
        bool[],
        address payable,
        MyInterface,
        MyInterface.MyEnum,
        function (string memory) external returns (uint)
    );


    function test() public {
        MyInterface instance = MyInterface(msg.sender);
        bool[] calldata arr;
        address payable addr;
        bytes4(CustomEvent1);
        bytes4(CustomEvent1());
        bytes4(CustomEvent1(1, true, arr, addr, instance, MyInterface.MyEnum.E1, this.testFunction));
        address(CustomEvent1);
    }
}
// ----
// TypeError 9640: (502-522): Explicit type conversion not allowed from "event CustomEvent1(uint256,bool,bool[],address payable,contract MyInterface,enum MyInterface.MyEnum,function (string) external returns (uint256))" to "bytes4".
// TypeError 6160: (539-553): Wrong argument count for function call: 0 arguments given but expected 7.
// TypeError 9640: (532-554): Explicit type conversion not allowed from "tuple()" to "bytes4".
// TypeError 9640: (564-656): Explicit type conversion not allowed from "tuple()" to "bytes4".
// TypeError 9640: (666-687): Explicit type conversion not allowed from "event CustomEvent1(uint256,bool,bool[],address payable,contract MyInterface,enum MyInterface.MyEnum,function (string) external returns (uint256))" to "address".
