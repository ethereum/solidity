contract C {
    function f() public returns (uint, uint) {
        try this.f() {

        } catch {

        }
    }
}