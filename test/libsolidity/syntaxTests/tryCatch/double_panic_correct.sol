contract C {
    function f() public {
        try this.f() {
        } catch Panic(uint) {
        } catch Panic(uint) {
        }
    }
}
// ----
// TypeError 6732: (102-131): This try statement already has a "Panic" catch clause.
