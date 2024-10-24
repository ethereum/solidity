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
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x0)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0xe1a9b9f1388a813deee43e2ae1eaf63d095398f17d3fa26e6d0a2e7c83d0e08)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x477b5e7b39240975dafc6cded0d12525798037e87adc18634564fe3f9fee21e)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0), 0x0)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x70d4dcf89c45409ef7721f1570f57b1e84a9cc78be9fd1373685173e41e8704)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x88b889e995139d59e267f8a786a4134fe0832522c8b44677b0b7b5d6e71e89c)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0), 0x0)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x1ecdc9ab7857d465fe0234e63d434e5e1b7dc9888e1a4450ef4675381f9503a0)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x23bdaf3d9c9204baed7e366f68689292bcc01bf43d6e0c31a2b27f1fcff710f)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0), 0x0)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x0)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x70d4dcf89c45409ef7721f1570f57b1e84a9cc78be9fd1373685173e41e8704)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x23bdaf3d9c9204baed7e366f68689292bcc01bf43d6e0c31a2b27f1fcff710f)
    checkEq(addmod(0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0, 0x0), 0x0)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x70d4dcf89c45409ef7721f1570f57b1e84a9cc78be9fd1373685173e41e8704)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x88b889e995139d59e267f8a786a4134fe0832522c8b44677b0b7b5d6e71e89c)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0), 0x0)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x9b22427d1685353c35dfaa0468ff3820d7ab6dabde8a72b9a8f57d7047abdac2)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x0)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0xc9f5b557f103313de9d384703c77017a4786125d168c748c1c0a6d6e2e4ef1a)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0), 0x0)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x65519d1a79d61afa297ae7f71ab392bc9f08e396f175809a5058e27c5f4c69fd)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x64fadaabf881989ef4e9c2381e3b80bd23c3092e8b463a460e0536b7172778d)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0), 0x0)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x0)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x64fadaabf881989ef4e9c2381e3b80bd23c3092e8b463a460e0536b7172778d)
    checkEq(addmod(0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0, 0x0), 0x0)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x1ecdc9ab7857d465fe0234e63d434e5e1b7dc9888e1a4450ef4675381f9503a0)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x23bdaf3d9c9204baed7e366f68689292bcc01bf43d6e0c31a2b27f1fcff710f)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0), 0x0)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x65519d1a79d61afa297ae7f71ab392bc9f08e396f175809a5058e27c5f4c69fd)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x64fadaabf881989ef4e9c2381e3b80bd23c3092e8b463a460e0536b7172778d)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0), 0x0)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x2f80f7b7dd2700b81d1625e9cc67ed586666598204608e7af7bc478876ecf938)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x2f80f7b7dd2700b81d1625e9cc67ed586666598204608e7af7bc478876ecf938)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x0)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0), 0x0)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x0)
    checkEq(addmod(0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0, 0x0), 0x0)
    checkEq(addmod(0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x0)
    checkEq(addmod(0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x70d4dcf89c45409ef7721f1570f57b1e84a9cc78be9fd1373685173e41e8704)
    checkEq(addmod(0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x23bdaf3d9c9204baed7e366f68689292bcc01bf43d6e0c31a2b27f1fcff710f)
    checkEq(addmod(0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27, 0x0), 0x0)
    checkEq(addmod(0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61)
    checkEq(addmod(0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x0)
    checkEq(addmod(0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x64fadaabf881989ef4e9c2381e3b80bd23c3092e8b463a460e0536b7172778d)
    checkEq(addmod(0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61, 0x0), 0x0)
    checkEq(addmod(0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c)
    checkEq(addmod(0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x0)
    checkEq(addmod(0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c, 0x0), 0x0)
    checkEq(addmod(0x0, 0x0, 0xefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c4fa04f27), 0x0)
    checkEq(addmod(0x0, 0x0, 0x4d91213e8b429a9e1aefd502347f9c106bd5b6d5ef45395cd47abeb823d5ed61), 0x0)
    checkEq(addmod(0x0, 0x0, 0x17c07bdbee93805c0e8b12f4e633f6ac33332cc10230473d7bde23c43b767c9c), 0x0)
    checkEq(addmod(0x0, 0x0, 0x0), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
