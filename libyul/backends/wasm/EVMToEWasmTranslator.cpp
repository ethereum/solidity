/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Translates Yul code from EVM dialect to eWasm dialect.
 */

#include <libyul/backends/wasm/EVMToEWasmTranslator.h>

#include <libyul/backends/wasm/WordSizeTransform.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/NameDisplacer.h>

#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/Object.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>
#include <liblangutil/SourceReferenceFormatter.h>

using namespace std;
using namespace dev;
using namespace yul;
using namespace langutil;

namespace
{
static string const polyfill{R"({
function or_bool(a, b, c, d) -> r {
	r := i64.ne(0, i64.or(i64.or(a, b), i64.or(c, d)))
}
// returns a + y + c plus carry value on 64 bit values.
// c should be at most 2
function add_carry(x, y, c) -> r, r_c {
	let t := i64.add(x, y)
	r := i64.add(t, c)
	r_c := i64.or(
		i64.lt_u(t, x),
		i64.lt_u(r, t)
	)
}
function add(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	let carry
	r4, carry := add_carry(x4, y4, 0)
	r3, carry := add_carry(x3, y3, carry)
	r2, carry := add_carry(x2, y2, carry)
	r1, carry := add_carry(x1, y1, carry)
}
function bit_negate(x) -> y {
	y := i64.xor(x, 0xffffffffffffffff)
}
function sub(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	// x - y = x + (~y + 1)
	let carry
	r4, carry := add_carry(x4, bit_negate(y4), 1)
	r3, carry := add_carry(x3, bit_negate(y3), i64.add(carry, 1))
	r2, carry := add_carry(x2, bit_negate(y2), i64.add(carry, 1))
	r1, carry := add_carry(x1, bit_negate(y1), i64.add(carry, 1))
}
function split(x) -> hi, lo {
	hi := i64.shr_u(x, 32)
	lo := i64.and(x, 0xffffffff)
}
// Multiplies two 64 bit values resulting in a 128 bit
// value split into two 64 bit values.
function mul_64x64_128(x, y) -> hi, lo {
	let xh, xl := split(x)
	let yh, yl := split(y)

	let t0 := i64.mul(xl, yl)
	let t1 := i64.mul(xh, yl)
	let t2 := i64.mul(xl, yh)
	let t3 := i64.mul(xh, yh)

	let t0h, t0l := split(t0)
	let u1 := i64.add(t1, t0h)
	let u1h, u1l := split(u1)
	let u2 := i64.add(t2, u1l)

	lo := i64.or(i64.shl(u2, 32), t0l)
	hi := i64.add(t3, i64.add(i64.shr_u(u2, 32), u1h))
}
// Add three 64-bit values plus carry (at most 2).
// Return the sum and the new carry value.
function add3_carry(a, b, c, carr) -> x, carry {
	let c1, c2
	x, c1 := add_carry(a, b, carr)
	x, c2 := add_carry(x, c, 0)
	carry := i64.add(c1, c2)
}
// Multiplies two 128 bit values resulting in a 256 bit
// value split into four 64 bit values.
function mul_128x128_256(x1, x2, y1, y2) -> r1, r2, r3, r4 {
	let ah, al := mul_64x64_128(x1, y1)
	let     bh, bl := mul_64x64_128(x1, y2)
	let     ch, cl := mul_64x64_128(x2, y1)
	let         dh, dl := mul_64x64_128(x2, y2)
	let carry
	r4 := dl
	r3, carry := add3_carry(bl, cl, dh, 0)
	r2, carry := add3_carry(al, bh, ch, carry)
	r1 := i64.add(ah, carry)
}
function mul(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
}
function div(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {}
function mod(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {}
function smod(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {}
function exp(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {}

function byte(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	if i64.eqz(i64.or(i64.or(x1, x2), x3)) {
		let component
		switch i64.div_u(x4, 8)
		case 0 { component := y1 }
		case 1 { component := y2 }
		case 2 { component := y3 }
		case 3 { component := y4 }
		x4 := i64.mul(i64.rem_u(x4, 8), 8)
		r4 := i64.shr_u(component, i64.sub(56, x4))
		r4 := i64.and(0xff, r4)
	}
}
function xor(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	r1 := i64.xor(x1, y1)
	r2 := i64.xor(x2, y2)
	r3 := i64.xor(x3, y3)
	r4 := i64.xor(x4, y4)
}
function or(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	r1 := i64.or(x1, y1)
	r2 := i64.or(x2, y2)
	r3 := i64.or(x3, y3)
	r4 := i64.or(x4, y4)
}
function and(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	r1 := i64.and(x1, y1)
	r2 := i64.and(x2, y2)
	r3 := i64.and(x3, y3)
	r4 := i64.and(x4, y4)
}
function not(x1, x2, x3, x4) -> r1, r2, r3, r4 {
	let mask := 0xffffffffffffffff
	r1, r2, r3, r4 := xor(x1, x2, x3, x4, mask, mask, mask, mask)
}
function iszero(x1, x2, x3, x4) -> r1, r2, r3, r4 {
	r4 := i64.eqz(i64.or(i64.or(x1, x2), i64.or(x3, x4)))
}
function eq(x1, x2, x3, x4, y1, y2, y3, y4) -> r1, r2, r3, r4 {
	if i64.eq(x1, y1) {
		if i64.eq(x2, y2) {
			if i64.eq(x3, y3) {
				if i64.eq(x4, y4) {
					r4 := 1
				}
			}
		}
	}
}


// TODO
function lt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function gt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function slt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function sgt(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}

function shl(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function shr(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function sar(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function addmod(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function mulmod(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function signextend(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}
function keccak256(x1, x2, x3, x4, y1, y2, y3, y4) -> z1, z2, z3, z4 {}

function address() -> z1, z2, z3, z4 {}
function balance(x1, x2, x3, x4) -> z1, z2, z3, z4 {}
function origin() -> z1, z2, z3, z4 {}
function caller() -> z1, z2, z3, z4 {}
function callvalue() -> z1, z2, z3, z4 {}
function calldataload(x1, x2, x3, x4) -> z1, z2, z3, z4 {}
function calldatasize() -> z1, z2, z3, z4 {}
function calldatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {}

// Needed?
function codesize() -> z1, z2, z3, z4 {}
function codecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {}
function datacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {}

function gasprice() -> z1, z2, z3, z4 {}
function extcodesize(x1, x2, x3, x4) -> z1, z2, z3, z4 {}
function extcodehash(x1, x2, x3, x4) -> z1, z2, z3, z4 {}
function extcodecopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {}

function returndatasize() -> z1, z2, z3, z4 {}
function returndatacopy(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) {}

function blockhash(x1, x2, x3, x4) -> z1, z2, z3, z4 {}
function coinbase() -> z1, z2, z3, z4 {}
function timestamp() -> z1, z2, z3, z4 {}
function number() -> z1, z2, z3, z4 {}
function difficulty() -> z1, z2, z3, z4 {}
function gaslimit() -> z1, z2, z3, z4 {}

function pop(x1, x2, x3, x4) {}

function mload(x1, x2, x3, x4) -> z1, z2, z3, z4 {}
function mstore(x1, x2, x3, x4, y1, y2, y3, y4) {}
function mstore8(x1, x2, x3, x4, y1, y2, y3, y4) {}
// Needed?
function msize() -> z1, z2, z3, z4 {}
function sload(x1, x2, x3, x4) -> z1, z2, z3, z4 {}
function sstore(x1, x2, x3, x4, y1, y2, y3, y4) {}

// Needed?
function pc() -> z1, z2, z3, z4 {}
function gas() -> z1, z2, z3, z4 {}

function log0(p1, p2, p3, p4, s1, s2, s3, s4) {}
function log1(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14
) {}
function log2(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14,
	t21, t22, t23, t24
) {}
function log3(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14,
	t21, t22, t23, t24,
	t31, t32, t33, t34
) {}
function log4(
	p1, p2, p3, p4, s1, s2, s3, s4,
	t11, t12, t13, t14,
	t21, t22, t23, t24,
	t31, t32, t33, t34,
	t41, t42, t43, t44,
) {}

function create(x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4) -> a1, a2, a3, a4 {}
function call(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4,
	g1, g2, g3, g4
) -> x1, x2, x3, x4 {}
function callcode(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4,
	g1, g2, g3, g4
) -> x1, x2, x3, x4 {}
function delegatecall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {}
function staticcall(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4,
	e1, e2, e3, e4,
	f1, f2, f3, f4
) -> x1, x2, x3, x4 {}
function create2(
	a1, a2, a3, a4,
	b1, b2, b3, b4,
	c1, c2, c3, c4,
	d1, d2, d3, d4
) -> x1, x2, x3, x4 {}
function selfdestruct(a1, a2, a3, a4) {}

function return(x1, x2, x3, x4, y1, y2, y3, y4) {}
function revert(x1, x2, x3, x4, y1, y2, y3, y4) {}
function invalid() {
	unreachable()
}
})"};

}

Object EVMToEWasmTranslator::run(Object const& _object)
{
	if (!m_polyfill)
		parsePolyfill();

	Block ast = boost::get<Block>(Disambiguator(m_dialect, *_object.analysisInfo)(*_object.code));
	NameDispenser nameDispenser{m_dialect, ast};
	FunctionHoister{}(ast);
	FunctionGrouper{}(ast);
	MainFunction{}(ast);
	ExpressionSplitter{m_dialect, nameDispenser}(ast);
	WordSizeTransform::run(m_dialect, ast, nameDispenser);

	NameDisplacer{nameDispenser, m_polyfillFunctions}(ast);
	for (auto const& st: m_polyfill->statements)
		ast.statements.emplace_back(ASTCopier{}.translate(st));

	Object ret;
	ret.code = make_shared<Block>(move(ast));
	ret.analysisInfo = make_shared<AsmAnalysisInfo>();

	ErrorList errors;
	ErrorReporter errorReporter(errors);
	AsmAnalyzer analyzer(*ret.analysisInfo, errorReporter, boost::none, WasmDialect::instance());
	if (!analyzer.analyze(*ret.code))
	{
		// TODO the errors here are "wrong" because they have invalid source references!
		string message;
		for (auto const& err: errors)
			message += langutil::SourceReferenceFormatter::formatErrorInformation(*err);
		yulAssert(false, message);
	}

	for (auto const& subObjectNode: _object.subObjects)
		if (Object const* subObject = dynamic_cast<Object const*>(subObjectNode.get()))
			ret.subObjects.push_back(make_shared<Object>(run(*subObject)));
		else
			ret.subObjects.push_back(make_shared<Data>(dynamic_cast<Data const&>(*subObjectNode)));
	ret.subIndexByName = _object.subIndexByName;

	return ret;
}

void EVMToEWasmTranslator::parsePolyfill()
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	shared_ptr<Scanner> scanner{make_shared<Scanner>(CharStream(polyfill, ""))};
	m_polyfill = Parser(errorReporter, WasmDialect::instance()).parse(scanner, false);
	if (!errors.empty())
	{
		string message;
		for (auto const& err: errors)
			message += langutil::SourceReferenceFormatter::formatErrorInformation(*err);
		yulAssert(false, message);
	}

	m_polyfillFunctions.clear();
	for (auto const& statement: m_polyfill->statements)
		m_polyfillFunctions.insert(boost::get<FunctionDefinition>(statement).name);
}

