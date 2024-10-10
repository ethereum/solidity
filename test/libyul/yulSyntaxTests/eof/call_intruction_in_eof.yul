object "a" {
    code {
        let success := call(gas(), 0x1, 0, 128, 4, 128, 0)
    }
}
// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
// ----
// TypeError 9132: (47-51): The "call" instruction is only available in legacy bytecode VMs (you are currently compiling to EOF).
// TypeError 9132: (52-55): The "gas" instruction is only available in legacy bytecode VMs (you are currently compiling to EOF).
// TypeError 3950: (52-57): Expected expression to evaluate to one value, but got 0 values instead.
// DeclarationError 3812: (32-82): Variable count mismatch for declaration of "success": 1 variables and 0 values.
