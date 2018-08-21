#!/usr/bin/env node

"use strict";

var util = require('util')
var exec = util.promisify(require('child_process').exec)
var mktemp = require('mktemp');
var download = require('download')
var JSONPath = require('JSONPath')
var fs = require('fs')
var bugs = JSON.parse(fs.readFileSync(__dirname + '/../docs/bugs.json', 'utf8'))

var bugsByName = {}
for (var i in bugs)
{
    if (bugs[i].name in bugsByName)
    {
        throw "Duplicate bug name: " + bugs[i].name
    }
    bugsByName[bugs[i].name] = bugs[i]
}

var tests = fs.readFileSync(__dirname + '/buglist_test_vectors.md', 'utf8')

var testVectorParser = /\s*#\s+(\S+)\s+## buggy\n([^#]*)## fine\n([^#]*)/g

runTests()

async function runTests()
{
	var result;
	while ((result = testVectorParser.exec(tests)) !== null)
	{
		var name = result[1]
		var buggy = result[2].split('\n--\n')
		var fine = result[3].split('\n--\n')
		console.log("Testing " + name + " with " + buggy.length + " buggy and " + fine.length + " fine instances")

		try {
			await checkRegex(name, buggy, fine)
			await checkJSONPath(name, buggy, fine)
		} catch (err) {
			console.error("Error: " + err)
		}
	}
}

function checkRegex(name, buggy, fine)
{
    return new Promise(function(resolve, reject) {
		var regexStr = bugsByName[name].check['regex-source']
		if (regexStr !== undefined)
		{
			var regex = RegExp(regexStr)
			for (var i in buggy)
			{
				if (!regex.exec(buggy[i]))
				{
					reject("Bug " + name + ": Buggy source does not match: " + buggy[i])
				}
			}
			for (var i in fine)
			{
				if (regex.exec(fine[i]))
				{
					reject("Bug " + name + ": Non-buggy source matches: " + fine[i])
				}
			}
		}
		resolve()
	})
}

async function checkJSONPath(name, buggy, fine)
{
	var jsonPath = bugsByName[name].check['ast-compact-json-path']
	if (jsonPath !== undefined)
	{
		var url = "http://github.com/ethereum/solidity/releases/download/v" + bugsByName[name].introduced + "/solc-static-linux"
		try {
			var tmpdir = await mktemp.createDir('XXXXX')
			var binary = tmpdir + "/solc-static-linux"
			await download(url, tmpdir)
			exec("chmod +x " + binary)
			for (var i in buggy)
			{
				var result = await checkJsonPathTest(buggy[i], tmpdir, binary, jsonPath, i)
				if (!result)
					throw "Bug " + name + ": Buggy source does not contain path: " + buggy[i]
			}
			for (var i in fine)
			{
				var result = await checkJsonPathTest(fine[i], tmpdir, binary, jsonPath, i + buggy.length)
				if (result)
					throw "Bug " + name + ": Non-buggy source contains path: " + fine[i]
			}
			exec("rm -r " + tmpdir)
		} catch (err) {
			throw err
		}
	}
}

function checkJsonPathTest(code, tmpdir, binary, query, idx) {
    return new Promise(function(resolve, reject) {
        var solFile = tmpdir + "/jsonPath" + idx + ".sol"
        var astFile = tmpdir + "/ast" + idx + ".json"
        writeFilePromise(solFile, code)
        .then(() => {
            return exec(binary + " --ast-compact-json " + solFile + " > " + astFile)
        })
        .then(() => {
            var jsonRE = /(\{[\s\S]*\})/
            var ast = JSON.parse(jsonRE.exec(fs.readFileSync(astFile, 'utf8'))[0])
            var result = JSONPath({json: ast, path: query})
            if (result.length > 0)
                resolve(true)
            else
                resolve(false)
        })
        .catch((err) => {
            reject(err)
        })
    })
}

function writeFilePromise(filename, data) {
    return new Promise(function(resolve, reject) {
        fs.writeFile(filename, data, 'utf8', function(err) {
            if (err) reject(err)
            else resolve(data)
        })
    })
}
