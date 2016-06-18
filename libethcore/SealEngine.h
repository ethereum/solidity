/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file SealEngine.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 *
 * Determines the PoW algorithm.
 */

#pragma once

#include <functional>
#include <unordered_map>
#include <libdevcore/Guards.h>
#include <libdevcore/RLP.h>
#include "BlockHeader.h"
#include "Common.h"

namespace dev
{
namespace eth
{

class BlockHeader;
struct ChainOperationParams;
class Interface;
class PrecompiledFace;
class TransactionBase;
class EnvInfo;

class SealEngineFace
{
public:
	virtual ~SealEngineFace() {}

	virtual std::string name() const = 0;
	virtual unsigned revision() const { return 0; }
	virtual unsigned sealFields() const { return 0; }
	virtual bytes sealRLP() const { return bytes(); }
	virtual StringHashMap jsInfo(BlockHeader const&) const { return StringHashMap(); }

	/// Don't forget to call Super::verify when subclassing & overriding.
	virtual void verify(Strictness _s, BlockHeader const& _bi, BlockHeader const& _parent = BlockHeader(), bytesConstRef _block = bytesConstRef()) const;
	/// Additional verification for transactions in blocks.
	virtual void verifyTransaction(ImportRequirements::value _ir, TransactionBase const& _t, BlockHeader const& _bi) const;
	/// Don't forget to call Super::populateFromParent when subclassing & overriding.
	virtual void populateFromParent(BlockHeader& _bi, BlockHeader const& _parent) const;

	bytes option(std::string const& _name) const { Guard l(x_options); return m_options.count(_name) ? m_options.at(_name) : bytes(); }
	bool setOption(std::string const& _name, bytes const& _value) { Guard l(x_options); try { if (onOptionChanging(_name, _value)) { m_options[_name] = _value; return true; } } catch (...) {} return false; }

	virtual strings sealers() const { return { "default" }; }
	virtual std::string sealer() const { return "default"; }
	virtual void setSealer(std::string const&) {}

	virtual bool shouldSeal(Interface*) { return true; }
	virtual void generateSeal(BlockHeader const& _bi) = 0;
	virtual void onSealGenerated(std::function<void(bytes const& s)> const& _f) = 0;
	virtual void cancelGeneration() {}

	ChainOperationParams const& chainParams() const { return m_params; }
	void setChainParams(ChainOperationParams const& _params) { m_params = _params; }
	SealEngineFace* withChainParams(ChainOperationParams const& _params) { setChainParams(_params); return this; }
	virtual EVMSchedule const& evmSchedule(EnvInfo const&) const { return m_params.evmSchedule; }

	virtual bool isPrecompiled(Address const& _a) const { return m_params.precompiled.count(_a); }
	virtual bigint costOfPrecompiled(Address const& _a, bytesConstRef _in) const { return m_params.precompiled.at(_a).cost(_in); }
	virtual void executePrecompiled(Address const& _a, bytesConstRef _in, bytesRef _out) const { return m_params.precompiled.at(_a).execute(_in, _out); }

protected:
	virtual bool onOptionChanging(std::string const&, bytes const&) { return true; }
	void injectOption(std::string const& _name, bytes const& _value) { Guard l(x_options); m_options[_name] = _value; }

private:
	mutable Mutex x_options;
	std::unordered_map<std::string, bytes> m_options;

	ChainOperationParams m_params;
};

class SealEngineBase: public SealEngineFace
{
public:
	void generateSeal(BlockHeader const& _bi) override
	{
		RLPStream ret;
		_bi.streamRLP(ret);
		if (m_onSealGenerated)
			m_onSealGenerated(ret.out());
	}
	void onSealGenerated(std::function<void(bytes const&)> const& _f) override { m_onSealGenerated = _f; }

private:
	std::function<void(bytes const& s)> m_onSealGenerated;
};

using SealEngineFactory = std::function<SealEngineFace*()>;

class SealEngineRegistrar
{
public:
	/// Creates the seal engine and uses it to "polish" the params (i.e. fill in implicit values) as necessary. Use this rather than the other two
	/// unless you *know* that the params contain all information regarding the seal on the Genesis block.
	static SealEngineFace* create(ChainOperationParams const& _params);
	static SealEngineFace* create(std::string const& _name) { if (!get()->m_sealEngines.count(_name)) return nullptr; return get()->m_sealEngines[_name](); }

	template <class SealEngine> static SealEngineFactory registerSealEngine(std::string const& _name) { return (get()->m_sealEngines[_name] = [](){return new SealEngine;}); }
	static void unregisterSealEngine(std::string const& _name) { get()->m_sealEngines.erase(_name); }

private:
	static SealEngineRegistrar* get() { if (!s_this) s_this = new SealEngineRegistrar; return s_this; }

	std::unordered_map<std::string, SealEngineFactory> m_sealEngines;
	static SealEngineRegistrar* s_this;
};

#define ETH_REGISTER_SEAL_ENGINE(Name) static SealEngineFactory __eth_registerSealEngineFactory ## Name = SealEngineRegistrar::registerSealEngine<Name>(#Name)

class NoProof: public eth::SealEngineBase
{
public:
	std::string name() const override { return "NoProof"; }
	static void init();
};

}
}
