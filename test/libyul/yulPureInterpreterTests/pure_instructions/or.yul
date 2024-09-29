{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(or(0x0, 0x0), 0x0)
    check(or(0x0, 0x1), 0x1)
    check(or(0x0, 0x2), 0x2)
    check(or(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0x0, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1)
    check(or(0x0, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52)
    check(or(0x0, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe)
    check(or(0x1, 0x0), 0x1)
    check(or(0x1, 0x1), 0x1)
    check(or(0x1, 0x2), 0x3)
    check(or(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x1, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1)
    check(or(0x1, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b53)
    check(or(0x1, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cff)
    check(or(0x2, 0x0), 0x2)
    check(or(0x2, 0x1), 0x3)
    check(or(0x2, 0x2), 0x2)
    check(or(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0x2, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f3)
    check(or(0x2, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52)
    check(or(0x2, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x0), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x2), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f3)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0xbb5dfd12555ffe715f4e766f7ffed9fb9bfffbee9ff6bfb7f18e9ebeeffb9bf3)
    check(or(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x7addff63f69ee6e55f5eedef7fdffbfef3b77fee9ff46bd5f1fe0cfccf8fdeff)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x0), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x1), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b53)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x2), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0xbb5dfd12555ffe715f4e766f7ffed9fb9bfffbee9ff6bfb7f18e9ebeeffb9bf3)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52)
    check(or(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0xf1dd7773f7df7eb54f1ebfe17fff6bb7fb7bf7ae8f76fdf7f0fe96eaef7f5ffe)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x0), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x1), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cff)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x2), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x7addff63f69ee6e55f5eedef7fdffbfef3b77fee9ff46bd5f1fe0cfccf8fdeff)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0xf1dd7773f7df7eb54f1ebfe17fff6bb7fb7bf7ae8f76fdf7f0fe96eaef7f5ffe)
    check(or(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
