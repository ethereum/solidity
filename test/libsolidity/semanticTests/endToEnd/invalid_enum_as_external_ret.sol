contract C {
    enum X {
        A,
        B
    }

    function test_return() public returns(X) {
        X garbled;
        assembly {
            garbled: = 5
        }
        return garbled;
    }

    function test_inline_assignment() public returns(X _ret) {
        assembly {
            _ret: = 5
        }
    }

    function test_assignment() public returns(X _ret) {
        X tmp;
        assembly {
            tmp: = 5
        }
        _ret = tmp;
    }
}

// ====
// compileViaYul: also
// ----
// test_return() -> 
// test_return():"" -> ""
// test_inline_assignment() -> 
// test_inline_assignment():"" -> ""
// test_assignment() -> 
// test_assignment():"" -> ""
