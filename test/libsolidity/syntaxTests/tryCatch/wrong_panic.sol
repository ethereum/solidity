contract C {
    function f() public {
        try this.f() {
        } catch Panic() {
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 1271: (72-97): Expected `catch Panic(uint ...) { ... }`.
