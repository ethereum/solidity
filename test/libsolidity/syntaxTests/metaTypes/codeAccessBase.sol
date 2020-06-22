contract Base {
    function f() public pure returns (uint) {}
}
contract Test is Base {
    function creation() public pure returns (bytes memory) {
        return type(Test).creationCode;
    }
    function runtime() public pure returns (bytes memory) {
        return type(Test).runtimeCode;
    }
    function creationBase() public pure returns (bytes memory) {
        return type(Base).creationCode;
    }
    function runtimeBase() public pure returns (bytes memory) {
        return type(Base).runtimeCode;
    }
}
// ----
// TypeError 4224: (165-188): Circular reference for contract code access.
// TypeError 4224: (271-293): Circular reference for contract code access.
// TypeError 4224: (381-404): Circular reference for contract code access.
// TypeError 4224: (491-513): Circular reference for contract code access.
