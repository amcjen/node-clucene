OVERVIEW 
=========
node-clucene is a native Node.js module that wraps up the CLucene library.	CLucene is a C++ port of the ubiquitous Java Lucene library.	The Lucene ecosystem is vibrant, and lots of great code has come out of it.	 However, for those who do not wish to run a JVM-based information retrieval system, CLucene makes a lot of sense.

CLucene is based on a port of Lucene 2.3.	 Unfortunately, there's been a lot of new updates and features added since then.	The goal is to add features to this module as necessary, without rewriting it to map method-to-method with the current version of Lucene.

Right now, adding a document with the same docId will *replace* the older document in the index that has the same docId.  This is intentional, and makes document updating simple.  In the future, this will probably get refactored into a separate updateDocument() method call instead, and revert addDocument() back to adding only.

This library began as a project from Tyler Gillies (http://github.com/tjgillies/node-lucene).	 Mad props to him for getting this library bootstrapped and running!


HOW IT WORKS:
============

Indexing information into the index
-------------------------------
```javascript
var cl = require('clucene').CLucene,
    clucene = new cl.Lucene();
    
var indexPath = './test.index',
    data = [ {'name': 'Eric Jennings', 'timestamp': '1293765885000'},
             {'name': 'Thomas Anderson', 'timestamp': '129555555555555'} ];

var addItem = function(contact, index) {
    var doc = new cl.Document(),
        docId = index;
    
    doc.addField('name', contact.name, cl.STORE_YES|cl.INDEX_TOKENIZED);
    doc.addField('_type', 'contact', cl.STORE_YES|cl.INDEX_UNTOKENIZED);
    doc.addField('timestamp', contact.timestamp, cl.STORE_YES|cl.INDEX_UNTOKENIZED);

    clucene.addDocument(docId, doc, indexPath, function(err, indexTime, docsReplaced) {
        if (err) {
            console.log('Error indexing document: ' + err);
        }

        console.log('Indexed document in ' + indexTime + ' ms');

        if (docsReplaced > 0) {
            console.log('Updated ' + docsReplaced + ' existing document(s)');
        }
        
        clucene.closeWriter();
    });
}

var i = 0,
    m = setInterval(function () {
    addItem(data[i], (i - 1).toString(10));
    i += 1;
    if (i >= data.length) {
        clearInterval(m);
    }
}, 500);
```


Querying information out of the index
-------------------------------
```javascript
var cl = require('clucene').CLucene,
    clucene = new cl.Lucene();

var indexPath = './test.index';

var queryTerm = 'name:jenn*';

clucene.search(indexPath, queryTerm, function(err, results, searchTime) {
    if (err) {
        console.log('Search error: ' + err);
        return;
    }

    console.log('Search results: ');

    for (var i=0; i<results.length; i++) {
        console.log(results[i]);
    }
    
    console.log('Searched in ' + searchTime + ' ms');
});
```
		

REQUIREMENTS:
=============
node-clucene requires the CLucene library.	This is not included in this module, you must install it on your own.	 Instructions can be found here (http://clucene.sourceforge.net/)

LICENSE:
=============
Apache/LGPL licensed (which is what the CLucene library is licensed with as well)
