contract A {
    receive() external payable { }
}
contract C is A {
    fallback() external payable { }
}
