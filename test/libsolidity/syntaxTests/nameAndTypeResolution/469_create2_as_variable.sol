contract c { function f() public { uint create2; assembly { create2(0, 0, 0, 0) } }}
// ----
// Warning: (35-47): Variable is shadowed in inline assembly by an instruction of the same name
// Warning: (60-79): The "create2" instruction is not supported by the VM version "byzantium" you are currently compiling for. It will be interpreted as an invalid instruction on this VM.
// Warning: (60-79): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
// DeclarationError: (58-81): Unbalanced stack at the end of a block: 1 surplus item(s).
