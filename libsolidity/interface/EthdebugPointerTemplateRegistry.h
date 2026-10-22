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
// SPDX-License-Identifier: GPL-3.0
/**
 * The pointer templates of ethdebug/format/info/resources, derived from the
 * storage layout of the types of the frontend.
 */

#pragma once

#include <libevmasm/EthdebugSchema.h>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace solidity::frontend
{
class Type;
class ArrayType;
class MappingType;
class StructType;
}

namespace solidity::frontend::ethdebug
{

/// The pointer templates of the storage types state variables have and are
/// composed of, one per type, keyed by the type identifier: how a value of the
/// type is laid out from a base slot. Every template expects `slot`, the base
/// slot of the value; a mapping's expects `key` as well, the key of the entry
/// it locates. Value types need no template, they are one region wherever they
/// occur. A mapping nested in another type is the region of its base slot,
/// from which the mapping's own template locates the entries, so that a type
/// nested in itself through a mapping expects one key per level a consumer
/// descends, rather than a list of keys the template would have to fix.
///
/// The region names a template produces are relative to the value it
/// describes: a struct's members by their names, an array's elements `item`,
/// the parts of a `bytes` or `string` `length-flag`, `long-length` and `data`.
/// A template referencing another prefixes the names that one produces with
/// the member or element it describes, so that `from-x` and `to-x` tell the
/// two `Point` members of a `Line` apart.
///
/// Templates reference each other by name, including a type nested in itself,
/// so the pointers making up a template form a tree.
class PointerTemplateRegistry
{
public:
	/// Registers the template of @a _type if it is a struct, an array or a
	/// mapping, and of every such type it composes.
	void registerType(Type const& _type);

	std::map<std::string, evmasm::ethdebug::schema::Pointer::Template> takeTemplates();

	/// Whether values of @a _type are described by a template rather than by a
	/// single region.
	static bool needsTemplate(Type const& _type);

private:
	using Pointer = evmasm::ethdebug::schema::Pointer;
	using Expression = evmasm::ethdebug::schema::Pointer::Expression;

	struct Built
	{
		std::vector<std::string> expect;
		Pointer body;
		/// The region names the body produces, for the references to prefix.
		std::vector<std::string> produced;
	};

	/// Adds @a _name to @a _produced unless it is there already.
	static void produce(std::vector<std::string>& _produced, std::string const& _name);

	Built build(Type const& _type);

	/// The pointer of a value of @a _type at @a _slot that is part of another
	/// value: the member of a struct, the element of an array or the value of a
	/// mapping, called @a _name. @a _layoutOffset is the byte offset of the
	/// value in its slot as the storage layout counts it, from the least
	/// significant byte. The region names it produces are added to @a _produced.
	Pointer component(
		Type const& _type,
		Expression _slot,
		unsigned _layoutOffset,
		std::string const& _name,
		std::vector<std::string>& _produced
	);

	/// The members of a struct at their slot and byte offsets from the base
	/// slot, with the types they have in storage, which key their templates.
	Built buildStruct(StructType const& _structType);

	Built buildArray(ArrayType const& _arrayType);

	/// The value of the entry for `key` lives at `keccak256(pad(key) . slot)`.
	Built buildMapping(MappingType const& _mappingType);

	/// Dynamic arrays store their element count in the base slot and their data
	/// starting at `keccak256(slot)`.
	Pointer dynamicArray(ArrayType const& _arrayType, std::vector<std::string>& _produced);

	/// A list of element pointers laid out from @a _dataStart, each called
	/// `item`. Value-type elements narrower than a word are packed multiple to
	/// a slot from its least significant byte; everything else advances in
	/// whole slots.
	Pointer elementList(ArrayType const& _arrayType, Expression _dataStart, Expression _count, std::vector<std::string>& _produced);

	/// `bytes` and `string` use the compact encoding: short values keep their
	/// data in the base slot with the doubled length in the last byte; long
	/// values keep `2 * length + 1` in the base slot and their data starting at
	/// `keccak256(slot)`. Either way the data is the region `data`.
	Pointer bytesOrString(std::vector<std::string>& _produced);

	std::map<std::string, Built> m_templates;
	/// The types whose templates are being built, so that a type nested in
	/// itself references its own template instead of building it again.
	std::set<std::string> m_building;
};

}
