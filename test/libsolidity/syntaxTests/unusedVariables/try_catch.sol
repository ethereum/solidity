contract test {
    function f() public returns (uint b) {
        try this.f() returns (uint a) {

        } catch Error(string memory message) {

        } catch (bytes memory error) {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// Warning 5667: (49-55): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (89-95): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (122-143): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (165-183): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
