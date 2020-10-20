contract C {
    function f() public {
        try this.f() {
        } catch Panic(bytes memory) {
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 1271: (72-109): Expected `catch Panic(uint ...) { ... }`.
