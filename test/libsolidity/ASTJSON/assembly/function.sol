contract C {
    function h() view public {
        assembly { function g() { pop(blockhash(20)) } g() }
    }
}

// ----
