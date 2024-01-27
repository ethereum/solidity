contract C {
    fallback() external {
        assembly {
            let x_offset := 0
            let x_slot := 0
            {
                x_offset := 1
                x_slot := 1
            }
        }
    }
}
// ----
