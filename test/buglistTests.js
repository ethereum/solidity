#!/usr/bin/env node

"use strict";

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

var result;
while ((result = testVectorParser.exec(tests)) !== null)
{
    var name = result[1]
    var buggy = result[2].split('\n--\n')
    var fine = result[3].split('\n--\n')
    console.log("Testing " + name + " with " + buggy.length + " buggy and " + fine.length + " fine instances")

    var regex = RegExp(bugsByName[name].check['regex-source'])
    for (var i in buggy)
    {
        if (!regex.exec(buggy[i]))
        {
            throw "Bug " + name + ": Buggy source does not match: " + buggy[i]
        }
    }
    for (var i in fine)
    {
        if (regex.exec(fine[i]))
        {
            throw "Bug " + name + ": Non-buggy source matches: " + fine[i]
        }
    }
}
