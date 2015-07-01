var chai = require('chai');
var assert = chai.assert;
var filter = require('../lib/web3/filter');
var u = require('./helpers/test.utils.js');

var empty = function () {};
var implementation = {
    newFilter: empty,
    getLogs: empty,
    uninstallFilter: empty,
    startPolling: empty,
    stopPolling: empty,
};

describe('web3.eth.filter', function () {
    describe('methods', function () {
        // var f = filter({}, implementation);

        // u.methodExists(f, 'watch');
        // u.methodExists(f, 'stopWatching');
        // u.methodExists(f, 'get');
    });
});
