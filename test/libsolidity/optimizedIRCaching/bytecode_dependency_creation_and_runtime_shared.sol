contract A {}

contract C {
    A a = new A();

    function f() public returns (A) {
        return new A();
    }
}
// ----
// cachedObjects: 4
