contract C {
    event E();
}

contract D {
    function test() public {
        emit C.E();
    }
}

// ====
// targetContract: D
// ----
// test() ->
// ~ emit E()
