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
    checkEq(sub(0x0, 0x0), 0x0)
    checkEq(sub(0x0, 0x1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(sub(0x0, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(sub(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    checkEq(sub(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x2)
    checkEq(sub(0x0, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e84)
    checkEq(sub(0x0, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd6)
    checkEq(sub(0x0, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9de)
    checkEq(sub(0x1, 0x0), 0x1)
    checkEq(sub(0x1, 0x1), 0x0)
    checkEq(sub(0x1, 0x2), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(sub(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x2)
    checkEq(sub(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x3)
    checkEq(sub(0x1, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e85)
    checkEq(sub(0x1, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd7)
    checkEq(sub(0x1, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9df)
    checkEq(sub(0x2, 0x0), 0x2)
    checkEq(sub(0x2, 0x1), 0x1)
    checkEq(sub(0x2, 0x2), 0x0)
    checkEq(sub(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x3)
    checkEq(sub(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x4)
    checkEq(sub(0x2, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e86)
    checkEq(sub(0x2, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd8)
    checkEq(sub(0x2, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9e0)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e83)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd5)
    checkEq(sub(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9dd)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xd0681363049e176333ffae5ed5a5b2f077e305311faf79e69cb4f3c4ecaf0e82)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0xf2f019ba143678a0e2806f6052982fcf5a6c1999d63f0ccd7cb9c441a32bfdd4)
    checkEq(sub(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x65cee5a56f31b2bb03ab90d9a58900b65d3682240348bea44d0e68edb4b8e9dc)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x0), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x1), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17b)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x2), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17a)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17d)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17e)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0x0)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0x228806570f98613dae80c1017cf27cdee2891468b68f92e6e004d07cb67cef52)
    checkEq(sub(0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x9566d2426a939b57cfabe27acfe34dc5e5537cf2e39944bdb0597528c809db5a)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x0), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x1), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd40229)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x2), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd40228)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022b)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022c)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0xdd77f9a8f0679ec2517f3efe830d83211d76eb9749706d191ffb2f83498310ae)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0x0)
    checkEq(sub(0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x72decbeb5afb3a1a212b217952f0d0e702ca688a2d09b1d6d054a4ac118cec08)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x0), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x1), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471621)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x2), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471620)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471623)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471624)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x2f97ec9cfb61e89ccc0051a12a5a4d0f881cfacee0508619634b0c3b1350f17c), 0x6a992dbd956c64a830541d85301cb23a1aac830d1c66bb424fa68ad737f624a6)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0xd0fe645ebc9875f1d7f909fad67d030a593e66629c0f33283463bbe5cd4022a), 0x8d213414a504c5e5ded4de86ad0f2f18fd359775d2f64e292fab5b53ee7313f8)
    checkEq(sub(0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622, 0x9a311a5a90ce4d44fc546f265a76ff49a2c97ddbfcb7415bb2f197124b471622), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
