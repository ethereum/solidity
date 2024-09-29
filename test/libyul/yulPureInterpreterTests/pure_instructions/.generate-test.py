# pyright: strict

from itertools import product
import random
import os

from typing import Callable, Iterable, Union

os.chdir(os.path.dirname(os.path.abspath(__file__)))

random.seed(20240823)

MX = 2**256

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
):
    print('Generating test for', fn_name)

    src: list[str] = []
    src.append('{')
    src.append("""
    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }
""")

    if test_numbers is None:
        if param_cnt == 1:
            test_numbers = [
                0, 1, 2, 3,
                MX - 1, MX - 2, MX - 3,
                random.randrange(MX), random.randrange(MX), random.randrange(MX), random.randrange(MX)
            ]
        else:
            test_numbers = [0, 1, 2, MX - 1, MX - 2, random.randrange(MX), random.randrange(MX), random.randrange(MX)]

    param_set = list(product(test_numbers, repeat=param_cnt))

    for p in param_set:
        res = u256(calc(p))
        # printing hex to save space
        src.append(f'    check({fn_name}({', '.join(hex(i) for i in p)}), {hex(res)})')
    src.append('}')

    src.append('// ====')
    src.append('// maxTraceSize: 0')
    if evm_version is not None:
        src.append(f'// EVMVersion: {evm_version}')

    with open(fn_name + '.yul', 'w', encoding='utf-8') as f:
        print('\n'.join(src), file=f)

def main():
    gen_test('add',
             param_cnt = 2,
             calc = lambda p: p[0] + p[1])
    gen_test('mul',
             param_cnt = 2,
             calc = lambda p: p[0] * p[1])
    gen_test('sub',
             param_cnt = 2,
             calc = lambda p: p[0] - p[1])
    gen_test('div',
             param_cnt = 2,
             calc = lambda p: p[0] // p[1] if p[1] != 0 else 0)
    gen_test('sdiv',
             param_cnt = 2,
             calc = signed_div)
    gen_test('mod',
             param_cnt = 2,
             calc = lambda p: p[0] % p[1] if p[1] != 0 else 0)
    gen_test('smod',
             param_cnt = 2,
             calc = signed_mod)
    gen_test('exp',
             param_cnt = 2,
             calc = lambda p: pow(p[0], p[1], MX))
    gen_test('not',
             param_cnt = 1,
             calc = lambda p: ~p[0])
    gen_test('lt',
             param_cnt = 2,
             calc = lambda p: p[0] < p[1])
    gen_test('gt',
             param_cnt = 2,
             calc = lambda p: p[0] > p[1])
    gen_test('slt',
             param_cnt = 2,
             calc = lambda p: u2s(p[0]) < u2s(p[1]))
    gen_test('sgt',
             param_cnt = 2,
             calc = lambda p: u2s(p[0]) > u2s(p[1]))
    gen_test('eq',
             param_cnt = 2,
             calc = lambda p: p[0] == p[1])
    gen_test('iszero',
             param_cnt = 1,
             calc = lambda p: p[0] == 0)
    gen_test('and',
             param_cnt = 2,
             calc=lambda p: p[0] & p[1])
    gen_test('or',
             param_cnt = 2,
             calc = lambda p: p[0] | p[1])
    gen_test('xor',
             param_cnt = 2,
             calc = lambda p: p[0] ^ p[1])
    gen_test('byte',
             param_cnt = 2,
             test_numbers = [
                 random.randrange(MX), random.randrange(MX), random.randrange(MX),
                 random.randrange(1, 32), random.randrange(1, 32), random.randrange(1, 32),
                 0
             ],
             calc = lambda p: 0 if p[0] >= 32 else (p[1] >> (8 * (31 - p[0]))) & 0xff)
    gen_test('shl',
             evm_version='>=constantinople',
             param_cnt = 2,
             calc = lambda p: p[1] << p[0] if p[0] < 32 else 0)
    gen_test('shr',
             evm_version='>=constantinople',
             param_cnt = 2,
             calc = lambda p: p[1] >> p[0])
    gen_test('sar',
             evm_version='>=constantinople',
             param_cnt = 2,
             calc = sar)
    gen_test('addmod',
             param_cnt = 3,
             test_numbers = [random.randrange(0, MX) for _ in range(3)] + [0],
             calc = lambda p: (p[0] + p[1]) % p[2] if p[2] != 0 else 0)
    gen_test('mulmod',
             param_cnt = 3,
             test_numbers = [random.randrange(0, MX) for _ in range(3)] + [0],
             calc = lambda p: (p[0] * p[1]) % p[2] if p[2] != 0 else 0)
    gen_test('signextend',
             param_cnt = 2,
             calc = signextend)

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

if __name__ == '__main__':
    main()
