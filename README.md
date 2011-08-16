OVERVIEW
=========
node-clucene is a native Node.js module that wraps up the CLucene library.  CLucene is a C++ port of the ubiquitous Java Lucene library.  The Lucene ecosystem is vibrant, and lots of great code has come out of it.  However, for those who do not wish to run a JVM-based information retrieval system, CLucene makes a lot of sense.

CLucene is based on a port of Lucene 2.3.  Unfortunately, there's been a lot of new updates and features added since then.  The goal is to add features to this module as necessary, without rewriting it to map method-to-method with the current version of Lucene.

This library begun as a project from Tyler Gillies (http://github.com/tjgillies/node-lucene).  Mad props to him for getting this library bootstrapped and running!


HOW IT WORKS:
============

Indexing information into the index
-------------------------------
    var cl = require('clucene').CLucene;
    var doc = new cl.Document();
    doc.addField('_id', '1', cl.Store.STORE_YES|cl.Store.INDEX_UNTOKENIZED);
    doc.addField('name', 'Eric Jennings', cl.Store.STORE_YES|cl.Store.INDEX_TOKENIZED);
    lucene.addDocument(doc, '/path/where/to/store/index', function(err, indexTime) {
        if (err) {
            console.log('Error indexing document: ' + err);
        }
        console.log('Indexed document in ' + indexTime + ' ms');
    });


Querying information out of the index
-------------------------------
    var cl = require("./clucene").CLucene;
    var lucene = new cl.Lucene();
    var util = require('util');

    lucene.search("tweets.lucene", "content:(" + process.argv[2] + ")", function(err, results) {
        if (err) {
            console.log("Search error: " + err);
            return;
        }
				
				console.log('Search results: ');
				for (var i=0; i<results.length; i++) {
					console.log(results[i]);
				}
		});
		

REQUIREMENTS AND BUILD:
=============
node-clucene has the CLucene source added as a git submodule, and it resides in dep/clucene.  CLucene has its own build requirements.

CLucene Build Instructions (from the CLucene README file)
---------------------------

Dependencies:
* CMake version 2.4.2 or later (2.6 or later recommended due to an improved FindBoost module).
* Boost libs (http://www.boost.org/), latest version recommended.
* A functioning and fairly new C++ compiler. We test mostly on GCC and Visual Studio 6+.
Anything other than that may not work.
* Something to unzip/untar the source code.

Build instructions:
1.) Download the latest source code of the HEAD of the master branch in our git repository.
2.) Unpack the tarball/zip/bzip/whatever if downloaded as such.
3.) Open a command prompt, terminal window, or Cygwin session.
4.) Change directory into the root of the source code (from now on referred as ).
     # cd 
5.) Create and change directory into an 'out-of-source' directory for your build. 
    [This is by far the easiest way to build. It has the benefit of being able to 
    create different types of builds in the same source-tree.]
     # mkdir /build-name
     # cd /build-name
5*.) Windows users: make sure Boost environment variables are defined at least for the current
   command prompt session. You can check this by typing "set" (no quotes). If you have any doubts,
   just type the following 3 commands to set those variables, as follows (boost_1_40_0 being the
   current Boost version):
     set BOOST_BUILD_PATH=C:\{parent directory for boost}\boost_1_40_0\tools\build\v2
     set BOOST_PATH=C:\{parent directory for boost}
     set BOOST_ROOT=C:\{parent directory for boost}\boost_1_40_0
6.) Configure using CMake. This can be done many different ways, but the basic syntax is
     # cmake [-G "Script name"] ..
    [Where "Script name" is the name of the scripts to build (e.g. Visual Studio 8 2005).
    A list of supported build scripts can be found by]
     # cmake --help
7.) You can configure several options such as the build type, debugging information, 
    mmap support, etc., by using the CMake GUI or by calling 
     # ccmake ..
    Make sure you call configure again if you make any changes.
8.) Start the build. This depends on which build script you specified, but it would be something like
     # make
or
     # nmake
    Or open the solution files with your IDE.

    [You can also specify to just build a certain target (such as cl_test, cl_demo, 
    clucene-core (shared library), clucene-core-static (static library).]
9.) The binary files will be available in build-name/bin.
10.)Test the code. (After building the tests - this is done by default, or by calling make cl_test)
     # ctest -V
11.)At this point you can install the library:
     # make install
    [There are options to do this from the IDE, but I find it easier to create a 
    distribution (see instructions below) and install that instead.]
or
     # make cl_demo
    [This creates the demo application, which demonstrates a simple text indexing and searching].
or
	Adjust build values using ccmake or the CMake GUI and rebuild.
	

node-clucene build instructions
-------------------
  
    cd <node-clucene top-level directory>
    node-waf configure build
