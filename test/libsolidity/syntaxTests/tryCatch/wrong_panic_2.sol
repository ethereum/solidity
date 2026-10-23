contract C {
    function f() public {
        try this.f() {
        } catch Panic(bytes memory) {
        }
    }
}
// ----
// TypeError 1271: (72-109): Expected `catch Panic(uint ...) { ... }`.
