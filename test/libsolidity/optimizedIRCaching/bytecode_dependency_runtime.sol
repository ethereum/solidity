contract A {}

contract C {
    function f() public returns (A) {
        return new A();
    }
}
// ----
// cachedObjects: 4
