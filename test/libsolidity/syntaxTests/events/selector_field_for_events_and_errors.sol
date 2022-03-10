interface MyInterface {
    event someEvent();
    error someError();
    function someFunction() external;
}

contract Test {
    function nameIsIrrelevant() pure public {
        bytes4  x = MyInterface.someFunction.selector; // This works
        bytes32 y = MyInterface.someEvent.selector; // This doesn't, that is what I'd like to add
        bytes32 z = MyInterface.someError.selector; // This doesn't, that is what I'd like to add
    }
}
// ----
// TypeError 9582: (262-283): Member "someEvent" not found or not visible after argument-dependent lookup in type(contract MyInterface).
