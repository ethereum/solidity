contract c {
    function fun() public returns (uint r) {
        uint _ = 8;
        return _ + 1;
    }
}
// ----
// DeclarationError 3726: (66-72): The name "_" is reserved.
