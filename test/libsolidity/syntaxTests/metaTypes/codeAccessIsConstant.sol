contract Test {
    bytes constant c = type(B).creationCode;
    bytes constant r = type(B).runtimeCode;

}
contract B { function f() public pure {} }
// ----
