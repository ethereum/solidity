contract Base {
    function f() public pure returns (uint) {}
}
contract Test1 is Base {
    function creation() public pure returns (bytes memory) {
        return type(Test1).creationCode;
    }
}
contract Test2 is Base {
    function runtime() public pure returns (bytes memory) {
        return type(Test2).runtimeCode;
    }
}
contract Test3 is Base {
    function creationBase() public pure returns (bytes memory) {
        return type(Base).creationCode;
    }
}
contract Test4 is Base {
    function runtimeBase() public pure returns (bytes memory) {
        return type(Base).runtimeCode;
    }
}
// ----
// TypeError 7813: (166-190): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
// TypeError 7813: (300-323): Circular reference to contract bytecode either via "new" or "type(...).creationCode" / "type(...).runtimeCode".
