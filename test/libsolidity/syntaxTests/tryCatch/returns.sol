contract C {
    function f() public returns (uint, uint) {
        try this.f() returns (uint a, uint b) {
            a = 1;
            b = 2;
        } catch {

        }
    }
}
