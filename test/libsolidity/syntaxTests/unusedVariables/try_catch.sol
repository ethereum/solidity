contract test {
    function f() public returns (uint b) {
        try this.f() returns (uint a) {

        } catch Error(string memory message) {

        } catch (bytes memory error) {

        }
    }
}
// ----
// Warning 6335: (165-183): "error" will be promoted to keyword in the future and will not be allowed as an identifier anymore.
// Warning 5667: (49-55): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (89-95): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (122-143): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
// Warning 5667: (165-183): Unused try/catch parameter. Remove or comment out the variable name to silence this warning.
