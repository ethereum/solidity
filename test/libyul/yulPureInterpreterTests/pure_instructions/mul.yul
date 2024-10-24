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
    checkEq(mul(0x0, 0x0), 0x0)
    checkEq(mul(0x0, 0x1), 0x0)
    checkEq(mul(0x0, 0x2), 0x0)
    checkEq(mul(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    checkEq(mul(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    checkEq(mul(0x0, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0x0)
    checkEq(mul(0x0, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0x0)
    checkEq(mul(0x0, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0x0)
    checkEq(mul(0x1, 0x0), 0x0)
    checkEq(mul(0x1, 0x1), 0x1)
    checkEq(mul(0x1, 0x2), 0x2)
    checkEq(mul(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(mul(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(mul(0x1, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a)
    checkEq(mul(0x1, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4)
    checkEq(mul(0x1, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16)
    checkEq(mul(0x2, 0x0), 0x0)
    checkEq(mul(0x2, 0x1), 0x2)
    checkEq(mul(0x2, 0x2), 0x4)
    checkEq(mul(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(mul(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    checkEq(mul(0x2, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0xb20876cdcf7b2d6bc6d0eece4ef9f8030aade0af57846dea12e23725d4e106f4)
    checkEq(mul(0x2, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0x328d50f6f6022108b3ca926e011be58e75a8bf8ad9e39ea26d94a99fec577d68)
    checkEq(mul(0x2, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0x6bc1f5f0b1d04e000899c7ae9f19a91c2b747c79e74ce5fedd36b8b2466db62c)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0x0)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x2)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0x26fbc4991842694a1c978898d88303fe7aa90fa8543dc90af68ee46d158f7c86)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0x66b9578484feef7ba61ab6c8ff720d38c52ba03a930e30aec935ab3009d4414c)
    checkEq(mul(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0xca1f0507a717d8fffbb31c28b0732b71ea45c1c30c598d009164a3a6dcc924ea)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0x0)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x2)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x4)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0x4df789323084d294392f1131b10607fcf5521f50a87b9215ed1dc8da2b1ef90c)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0xcd72af0909fddef74c356d91fee41a718a574075261c615d926b566013a88298)
    checkEq(mul(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0x943e0a0f4e2fb1fff766385160e656e3d48b838618b31a0122c9474db99249d4)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0x0), 0x0)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0x1), 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0x2), 0xb20876cdcf7b2d6bc6d0eece4ef9f8030aade0af57846dea12e23725d4e106f4)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x26fbc4991842694a1c978898d88303fe7aa90fa8543dc90af68ee46d158f7c86)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x4df789323084d294392f1131b10607fcf5521f50a87b9215ed1dc8da2b1ef90c)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0x5e212ad86e15d56a8b8dc59640978d62434da71e22e251daa07794f656461624)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0x72b7563189e6fb81a6e0c7e6ac6ae5cf9f65d32c803940a15b56442faa2efdc8)
    checkEq(mul(0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0x2c78a699a972d189ec393219bb4f65066d56fdf47055c43b7095704dcfe0aa7c)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0x0), 0x0)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0x1), 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0x2), 0x328d50f6f6022108b3ca926e011be58e75a8bf8ad9e39ea26d94a99fec577d68)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x66b9578484feef7ba61ab6c8ff720d38c52ba03a930e30aec935ab3009d4414c)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xcd72af0909fddef74c356d91fee41a718a574075261c615d926b566013a88298)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0x72b7563189e6fb81a6e0c7e6ac6ae5cf9f65d32c803940a15b56442faa2efdc8)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0x55fa1bc0326d6284b14f80cb3633885cd52b77b32521d9e1a0aca1138e87ae90)
    checkEq(mul(0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0xaaaa01c250e883022bf70da9614aa53fd00e10bbbc7bb19e3b31165769de5f78)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0x0), 0x0)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0x1), 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0x2), 0x6bc1f5f0b1d04e000899c7ae9f19a91c2b747c79e74ce5fedd36b8b2466db62c)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xca1f0507a717d8fffbb31c28b0732b71ea45c1c30c598d009164a3a6dcc924ea)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x943e0a0f4e2fb1fff766385160e656e3d48b838618b31a0122c9474db99249d4)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0xd9043b66e7bd96b5e3687767277cfc018556f057abc236f509711b92ea70837a), 0x2c78a699a972d189ec393219bb4f65066d56fdf47055c43b7095704dcfe0aa7c)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0x9946a87b7b01108459e54937008df2c73ad45fc56cf1cf5136ca54cff62bbeb4), 0xaaaa01c250e883022bf70da9614aa53fd00e10bbbc7bb19e3b31165769de5f78)
    checkEq(mul(0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16, 0x35e0faf858e82700044ce3d74f8cd48e15ba3e3cf3a672ff6e9b5c592336db16), 0xd9d221a1c3d2f657ff36ba234d8aff13efcc67f6767406e1231830f52cc6a5e4)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
