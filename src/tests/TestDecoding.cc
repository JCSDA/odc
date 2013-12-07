/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file TestDecoding.h
///
/// @author Piotr Kuchta, ECMWF, June 2009

#include "eckit/utils/Timer.h"
#include "odblib/Reader.h"
#include "odblib/ToolFactory.h"
#include "TestDecoding.h"

using namespace std;
using namespace eckit;

namespace odb {
namespace tool {
namespace test {

ToolFactory<TestDecoding> _TestDecoding("TestDecoding");

TestDecoding::TestDecoding(int argc, char **argv)
: TestCase(argc, argv)
{}

TestDecoding::~TestDecoding() { }

/// Tests DispatchingWriter
///
void TestDecoding::test()
{
	const string fileName = "2000010106.odb";

	Timer t(string("TestDecoding::test: reading file '") + fileName + "'");

	odb::Reader f(fileName);
	odb::Reader::iterator it = f.begin();
	odb::Reader::iterator end = f.end();

	unsigned long long n = 0;	
	for (; it != end; ++it)
		++n;

	Log::info() << "TestDecoding::test decoded " << n << " lines." << std::endl;
}

void TestDecoding::setUp() {}
void TestDecoding::tearDown() {}

} // namespace test
} // namespace tool 
} // namespace odb 
