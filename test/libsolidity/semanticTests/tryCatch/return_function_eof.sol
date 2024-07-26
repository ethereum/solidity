contract C {
    function g() public returns (uint a, function() external h, uint b) {
        a = 1;
        h = this.fun;
        b = 9;
    }
    function f() public returns (uint, function() external, uint) {
        // Note that the function type uses two stack slots.
        try this.g() returns (uint a, function() external h, uint b) {
            return (a, h, b);
        } catch {
        }
    }
    function fun() public pure {}
}
// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f() -> 0x1, 0x67831474d284bf49471795e23b524e80a6b386a0946644cd0000000000000000, 9
