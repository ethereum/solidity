contract Sample {
    function f() public pure {
        assembly {
            let a := "test"
            let b := hex"112233445566778899aabbccddeeff6677889900"
            let c := hex"1234_abcd"
        }
    }
}

// ----
