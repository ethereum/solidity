contract test {
    uint constant x = 2;
    function f() pure public {
        assembly {
            let r := x.offset
        }
    }
}
// ----
// TypeError 6617: (112-120): The suffixes .offset and .slot can only be used on non-constant storage variables.
