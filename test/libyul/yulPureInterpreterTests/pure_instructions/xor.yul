/*
# pyright: strict
from itertools import product
from random import seed, randrange
import os
from typing import Callable, Iterable, Union

MX = 2**256
generator_file_content = ['/*'] + list(map(lambda line: line.rstrip(), open(__file__, "r").readlines())) + ['*' + '/']

def rand_int(n: int, start: int, end: Union[int, None] = None) -> list[int]:
    return [randrange(start, end) for _ in range(n)]

def u256(num: int):
    return int(num) % MX

def u2s(num: int):
    return int(num) - MX if (int(num) >> 255) > 0 else int(num)

def gen_test(
    fn_name: str,
    *,
    param_cnt: int,
    calc: Callable[[tuple[int, ...]], int],
    test_numbers: Union[Iterable[int], None] = None,
    evm_version: Union[str, None] = None,
    setup: str = "    function checkEq(a, b)\n    { if eq(a, b) { leave } revert(0, 0) }",
    format_case: Callable[
        [str, tuple[int, ...], int], str
    ] = lambda fn_name, params, res: f"checkEq({fn_name}({', '.join(hex(i) for i in params)}), {hex(res)})",
):
    print("Generating test for", fn_name)
    if test_numbers is None:
        if param_cnt == 1:
            test_numbers = [0, 1, 2, 3] + [MX - 1, MX - 2, MX - 3] + rand_int(4, MX)
        else:
            test_numbers = [0, 1, 2] + [MX - 1, MX - 2] + rand_int(3, MX)

    src: list[str] = []
    src.extend(generator_file_content)
    src.append("{")
    src.append(f"{setup}")
    for params in product(test_numbers, repeat=param_cnt):
        res = u256(calc(params))
        src.append(f"    {format_case(fn_name, params, res)}")
    src.append("}")

    src.append("// ====")
    src.append("// maxTraceSize: 0")
    if evm_version is not None:
        src.append(f"// EVMVersion: {evm_version}")

    with open(fn_name + ".yul", "w", encoding="utf-8") as f:
        print("\n".join(src), file=f)

def signed_div(p: tuple[int, ...]):
    (a, b) = u2s(p[0]), u2s(p[1])
    if b == 0:
        return 0
    res = abs(a) // abs(b)
    if (a < 0) != (b < 0):
        res = -res
    return res

def signed_mod(p: tuple[int, ...]):
    (a, b) = u2s(p[0]), u2s(p[1])
    if b == 0:
        return 0
    res = abs(a) % abs(b)
    if a < 0:
        res = -res
    return res

def byte(p: tuple[int, ...]):
    return 0 if p[0] >= 32 else (p[1] >> (8 * (31 - p[0]))) & 0xFF

def shl(p: tuple[int, ...]):
    return p[1] << p[0] if p[0] < 32 else 0

def shr(p: tuple[int, ...]):
    return p[1] >> p[0]

def sar(p: tuple[int, ...]):
    (sh, val) = p
    high_bit = val >> 255
    val >>= sh
    val |= u256(-high_bit) << max(0, 255 - sh)
    return val

def signextend(p: tuple[int, ...]):
    (byte_pos, val) = p
    byte_pos = min(byte_pos, 32)
    mask_shift = byte_pos * 8 + 8
    high_bit = (val >> (mask_shift - 1)) & 1
    if high_bit:
        return val | (u256(-1) << mask_shift)
    return val & ((1 << mask_shift) - 1)

def addmod(p: tuple[int, ...]):
    return (p[0] + p[1]) % p[2] if p[2] != 0 else 0

def mulmod(p: tuple[int, ...]):
    return (p[0] * p[1]) % p[2] if p[2] != 0 else 0

seed(20240823)
os.chdir(os.path.dirname(os.path.abspath(__file__)))
gen_test("add", param_cnt=2, calc=lambda p: p[0] + p[1])
gen_test("mul", param_cnt=2, calc=lambda p: p[0] * p[1])
gen_test("sub", param_cnt=2, calc=lambda p: p[0] - p[1])
gen_test("div", param_cnt=2, calc=lambda p: p[0] // p[1] if p[1] != 0 else 0)
gen_test("sdiv", param_cnt=2, calc=signed_div)
gen_test("mod", param_cnt=2, calc=lambda p: p[0] % p[1] if p[1] != 0 else 0)
gen_test("smod", param_cnt=2, calc=signed_mod)
gen_test("exp", param_cnt=2, calc=lambda p: pow(p[0], p[1], MX))
gen_test("not", param_cnt=1, calc=lambda p: ~p[0])
gen_test("lt", param_cnt=2, calc=lambda p: p[0] < p[1])
gen_test("gt", param_cnt=2, calc=lambda p: p[0] > p[1])
gen_test("slt", param_cnt=2, calc=lambda p: u2s(p[0]) < u2s(p[1]))
gen_test("sgt", param_cnt=2, calc=lambda p: u2s(p[0]) > u2s(p[1]))
gen_test("iszero", param_cnt=1, calc=lambda p: p[0] == 0)
gen_test("and", param_cnt=2, calc=lambda p: p[0] & p[1])
gen_test("or", param_cnt=2, calc=lambda p: p[0] | p[1])
gen_test("xor", param_cnt=2, calc=lambda p: p[0] ^ p[1])
gen_test("byte", param_cnt=2, test_numbers=rand_int(3, MX) + rand_int(3, 1, 32) + [0], calc=byte)
gen_test("shl", evm_version=">=constantinople", param_cnt=2, calc=shl)
gen_test("shr", evm_version=">=constantinople", param_cnt=2, calc=shr)
gen_test("sar", evm_version=">=constantinople", param_cnt=2, calc=sar)
gen_test("addmod", param_cnt=3, test_numbers=rand_int(3, MX) + [0], calc=addmod)
gen_test("mulmod", param_cnt=3, test_numbers=rand_int(3, MX) + [0], calc=mulmod)
gen_test("signextend", param_cnt=2, calc=signextend)
# The other uses `eq` to test. We need to test `eq` a bit differently.
gen_test(
    "eq",
    param_cnt=2,
    calc=lambda p: p[0] == p[1],
    setup= """    function checkEq(a, b)
    { if eq(a, b) { leave } revert(0, 0) }
    function checkNe(a, b)
    { if eq(a, b) { revert(0, 0) } }
""",
    format_case=lambda _, params, res: f"{"checkEq" if res == 1 else "checkNe"}({hex(params[0])}, {hex(params[1])})"
)
*/
{
    function checkEq(a, b)
    { if eq(a, b) { leave } revert(0, 0) }
    checkEq(xor(0x0, 0x0), 0x0)
    checkEq(xor(0x0, 0x1), 0x1)
    checkEq(xor(0x0, 0x2), 0x2)
    checkEq(xor(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(xor(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(xor(0x0, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1)
    checkEq(xor(0x0, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52)
    checkEq(xor(0x0, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe)
    checkEq(xor(0x1, 0x0), 0x1)
    checkEq(xor(0x1, 0x1), 0x0)
    checkEq(xor(0x1, 0x2), 0x3)
    checkEq(xor(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(xor(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(xor(0x1, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f0)
    checkEq(xor(0x1, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b53)
    checkEq(xor(0x1, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cff)
    checkEq(xor(0x2, 0x0), 0x2)
    checkEq(xor(0x2, 0x1), 0x3)
    checkEq(xor(0x2, 0x2), 0x0)
    checkEq(xor(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd)
    checkEq(xor(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    checkEq(xor(0x2, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f3)
    checkEq(xor(0x2, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b50)
    checkEq(xor(0x2, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfc)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0xc5f303ffaff31b9ea0b19b9090b926b56e69a511682bd4ee2e75f74b327d7d0e)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x6ea2beedaaa8c1ceb8f9cddea801f64ef5861cdb728d6b484f796975dc86e4ad)
    checkEq(xor(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x9f22c89c1961b95bf3e5523e86229c490cccca71f0db962b1f01fb9730f0a301)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0xc5f303ffaff31b9ea0b19b9090b926b56e69a511682bd4ee2e75f74b327d7d0f)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x6ea2beedaaa8c1ceb8f9cddea801f64ef5861cdb728d6b484f796975dc86e4ac)
    checkEq(xor(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x9f22c89c1961b95bf3e5523e86229c490cccca71f0db962b1f01fb9730f0a300)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x0), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x1), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f0)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x2), 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f3)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xc5f303ffaff31b9ea0b19b9090b926b56e69a511682bd4ee2e75f74b327d7d0e)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xc5f303ffaff31b9ea0b19b9090b926b56e69a511682bd4ee2e75f74b327d7d0f)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x0)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0xab51bd12055bda501848564e38b8d0fb9befb9ca1aa6bfa6610c9e3eeefb99a3)
    checkEq(xor(0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x5ad1cb63b692a2c55354c9ae169bbafc62a56f6098f042c531740cdc028dde0f)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x0), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x1), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b53)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x2), 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b50)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x6ea2beedaaa8c1ceb8f9cddea801f64ef5861cdb728d6b484f796975dc86e4ad)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x6ea2beedaaa8c1ceb8f9cddea801f64ef5861cdb728d6b484f796975dc86e4ac)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0xab51bd12055bda501848564e38b8d0fb9befb9ca1aa6bfa6610c9e3eeefb99a3)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0x0)
    checkEq(xor(0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0xf1807671b3c978954b1c9fe02e236a07f94ad6aa8256fd63507892e2ec7647ac)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x0), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x1), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cff)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x2), 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfc)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x9f22c89c1961b95bf3e5523e86229c490cccca71f0db962b1f01fb9730f0a301)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x9f22c89c1961b95bf3e5523e86229c490cccca71f0db962b1f01fb9730f0a300)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x3a0cfc00500ce4615f4e646f6f46d94a91965aee97d42b11d18a08b4cd8282f1), 0x5ad1cb63b692a2c55354c9ae169bbafc62a56f6098f042c531740cdc028dde0f)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x915d411255573e314706322157fe09b10a79e3248d7294b7b086968a23791b52), 0xf1807671b3c978954b1c9fe02e236a07f94ad6aa8256fd63507892e2ec7647ac)
    checkEq(xor(0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe, 0x60dd3763e69e46a40c1aadc179dd63b6f333358e0f2469d4e0fe0468cf0f5cfe), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
