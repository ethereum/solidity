contract C {
    event E();
}

contract D {
    function test() public {
        emit C.E();
    }
}

// ----
// test() ->
// ~ emit E()
