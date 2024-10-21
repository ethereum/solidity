object "a" {
    code {
        {
            auxdatastore(0, 32, 0x1122334455667788990011223344556677889900112233445566778899001122)
            return(0, 32)
        }
    }

    data "data1" hex"48656c6c6f2c20576f726c6421"
}

// ====
// bytecodeFormat: legacy
// ----
// DeclarationError 4619: (46-58): Function "auxdatastore" not found.