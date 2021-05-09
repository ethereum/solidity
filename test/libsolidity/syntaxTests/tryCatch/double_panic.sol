contract C {
    function f() public {
        try this.f() {
        } catch Panic(bytes memory) {
        } catch Panic(uint) {
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// TypeError 1271: (72-109): Expected `catch Panic(uint ...) { ... }`.
// TypeError 6732: (110-139): This try statement already has a "Panic" catch clause.
