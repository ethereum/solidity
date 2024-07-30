contract A {}
contract B {}

contract C {
    A a = new A();

    function f() public returns (B) {
        return new B();
    }
}
// ----
// cachedObjects: 6
