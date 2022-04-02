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
// Warning 5667: (49-55='uint b'): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (89-95='uint a'): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (122-143='string memory message'): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (165-183='bytes memory error'): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
