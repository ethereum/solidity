contract Test {
    function f() public pure returns (string memory) {
        return type(Test).name;
    }
}
