contract C {
    function f() public returns (uint, uint) {
        try this {
        } catch {
        }
        try gasleft() {
        } catch {
        }
        try type(address) {
        } catch {
        }
    }
}
// ----
// TypeError 5347: (72-76='this'): Try can only be used with external function calls and contract creation calls.
// TypeError 2536: (119-128='gasleft()'): Try can only be used with external function calls and contract creation calls.
// TypeError 4259: (176-183='address'): Invalid type for argument in the function call. An enum type, contract type or an integer type is required, but type(address) provided.
