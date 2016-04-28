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
/** @file $NAME.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2015
 */

#pragma once

#include "Plugin.h"

namespace Ui
{
class $NAME;
}

namespace dev
{
namespace aleth
{
namespace zero
{

class $NAME: public QObject, public Plugin
{
	Q_OBJECT

public:
	$NAME(ZeroFace* _z);
	~$NAME();

private:
	void onAllChange() override {}
	void readSettings(QSettings const&) override {}
	void writeSettings(QSettings&) override {}

private:
	Ui::$NAME* m_ui;
};

}
}
}
