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
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0xc076c544b658b9d6433b79099ab9c4c7976555fdec509370b531aa7e28aeddcf)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x12756ffcb72a0531763804b77c2da3fe0e3b14730ad1afae666f4f2b869d629)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x70b529595cd9a1b371790b5c7beacc40b501d82593a2190546eb66ce891015a)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x3b3d831ad300110bf8379e6722d718a5ba80ab4afc07d13b032b210e6afcac45)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0, 0x0), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x70b529595cd9a1b371790b5c7beacc40b501d82593a2190546eb66ce891015a)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x10bb2b36f81271b8cb131c23524e86ba668d4c82d4ee6ef2c68fd0e096d26e09)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0xcbe26ab4785bd7f90f7d61625b3b6e743a80f512e6052ca8e637cfdf517530)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x9fcfbcf2c672d5e341133c2e4e5c35997c85db652854dda117de560ba058659)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0, 0x0), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x3b3d831ad300110bf8379e6722d718a5ba80ab4afc07d13b032b210e6afcac45)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x9fcfbcf2c672d5e341133c2e4e5c35997c85db652854dda117de560ba058659)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x369336d99c45a673f35d496d007849c4402bce437c8086a327910a164256f41)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x8088cc58bec941a7296a95574cf1226bba8ace6e98e83077cbc0e407f2172708)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0, 0x0), 0x0)
    checkEq(mulmod(0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824, 0x0), 0x0)
    checkEq(mulmod(0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29, 0x0), 0x0)
    checkEq(mulmod(0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9, 0x0), 0x0)
    checkEq(mulmod(0x0, 0x0, 0x149b1d571568ae031cb636085e0b262218434cb40a6b1a0645e34118b6318824), 0x0)
    checkEq(mulmod(0x0, 0x0, 0xc27553129f2f5e3d6fda36b027315d930e590b0d3a74a7a11cd03cb98a695d29), 0x0)
    checkEq(mulmod(0x0, 0x0, 0xe561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4efd9c6b9), 0x0)
    checkEq(mulmod(0x0, 0x0, 0x0), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
