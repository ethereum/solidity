contract C {
    fallback() external {
        assembly {
            let x_offset := 0
            let x_slot := 0
            {
                let x_offset := 1
                let x_slot := 1
            }
        }
    }
}
// ----
// DeclarationError 1395: (146-163): Variable name x_offset already taken in this scope.
// DeclarationError 1395: (180-195): Variable name x_slot already taken in this scope.
