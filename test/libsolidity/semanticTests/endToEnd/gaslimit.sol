contract C {
    function f() public returns(uint) {
        return block.gaslimit;
    }
}

// ====
// compileViaYul: also
// ----
f(): "" // resultgasLimit(
//  -> "0"
