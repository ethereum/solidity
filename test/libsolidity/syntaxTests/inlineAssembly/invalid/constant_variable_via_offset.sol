contract test {
    uint constant x = 2;
    function f() pure public {
        assembly {
            let r := x_offset
        }
    }
}
// ----
// TypeError: (112-120): The suffixes _offset and _slot can only be used on non-constant storage variables.
