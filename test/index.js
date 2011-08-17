var path = require('path');
var fs = require('fs');
var wrench = require('wrench');

var cl = require('../clucene').CLucene;
var clucene = new cl.Lucene();

var indexPath = './test.index';

exports['add new document'] = function (test) {
    if (path.existsSync(indexPath)) {
        wrench.rmdirSyncRecursive(indexPath);
    }
    
    var doc = new cl.Document();
    var docId = '1';

    doc.addField('name', 'Eric Jennings', cl.STORE_YES|cl.INDEX_TOKENIZED);
    doc.addField('timestamp', '1293765885000', cl.STORE_YES|cl.INDEX_UNTOKENIZED);

    clucene.addDocument(docId, doc, indexPath, function(err, indexTime, docsReplaced) {
        test.equal(err, null);
        test.ok(is('Number', indexTime));
        test.equal(docsReplaced, 0);
        test.done();
    });
};

exports['query newly-added document'] = function (test) {
    clucene.search(indexPath, '1', function(err, results) {
        test.equal(err, null);
        test.ok(is('Array', results));
        test.equal(results[0]._id, 1);
        test.equal(results[0].name, 'Eric Jennings');
        test.equal(results[0].timestamp, '1293765885000');
        test.done();
    });
};

exports['update existing document'] = function (test) {
    var doc = new cl.Document();
    var docId = '1';

    doc.addField('name', 'Thomas Anderson', cl.STORE_YES|cl.INDEX_TOKENIZED);
    doc.addField('timestamp', '129555555555555', cl.STORE_YES|cl.INDEX_UNTOKENIZED);

    clucene.addDocument(docId, doc, indexPath, function(err, indexTime, docsReplaced) {
        test.equal(err, null);
        test.ok(is('Number', indexTime));
        test.equal(docsReplaced, 1);
        test.done();
    });
};

exports['query updated document'] = function (test) {
    clucene.search(indexPath, '1', function(err, results) {
        test.equal(err, null);
        test.ok(is('Array', results));
        test.equal(results[0]._id, 1);
        test.equal(results[0].name, 'Thomas Anderson');
        test.equal(results[0].timestamp, '129555555555555');
        test.done();
    });
};

exports['query by full field name'] = function (test) {
    clucene.search(indexPath, 'name:"Thomas"', function(err, results) {
        test.equal(err, null);
        test.ok(is('Array', results));
        test.equal(results[0]._id, 1);
        test.equal(results[0].name, 'Thomas Anderson');
        test.equal(results[0].timestamp, '129555555555555');
        test.done();
    });
};

exports['query by wildcard'] = function (test) {
    clucene.search(indexPath, 'name:Thom*', function(err, results) {
        test.equal(err, null);
        test.ok(is('Array', results));
        test.equal(results[0]._id, 1);
        test.equal(results[0].name, 'Thomas Anderson');
        test.equal(results[0].timestamp, '129555555555555');
        test.done();
    });
};

exports['cleanup'] = function (test) {
    if (path.existsSync(indexPath)) {
        wrench.rmdirSyncRecursive(indexPath);
    }
    test.done();
}

function is(type, obj) {
    var clas = Object.prototype.toString.call(obj).slice(8, -1);
    return obj !== undefined && obj !== null && clas === type;
}