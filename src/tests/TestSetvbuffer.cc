/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// \file TestSetvbuffer.h
///
/// @author Piotr Kuchta, ECMWF, Feb 2009

#include "eckit/utils/Timer.h"
#include "odblib/ODBAPISettings.h"
#include "odblib/ToolFactory.h"
#include "odblib/Writer.h"
#include "TestSetvbuffer.h"

using namespace std;
using namespace eckit;



namespace odb {
namespace tool {
namespace test {

ToolFactory<TestSetvbuffer> testSetvbuffer("TestSetvbuffer");

TestSetvbuffer::TestSetvbuffer(int argc, char **argv)
: TestCase(argc, argv)
{}

TestSetvbuffer::~TestSetvbuffer() { }

void TestSetvbuffer::setUp() { }

void TestSetvbuffer::test()
{
	size_t cols = 400;
	long long rows = 1000;
	size_t buffSize = 8 * 1024 * 1024;

	for (size_t i = 0; i < 10; ++i)
	{
		stringstream s;
		s << "TestSetvbuffer::setUp(): createFile(" << cols << ", " << rows << ", " << buffSize << ")" << std::endl;
		Timer t(s.str());
		createFile(cols, rows, buffSize);
	}
}

void TestSetvbuffer::createFile(size_t numberOfColumns, long long numberOfRows, size_t setvbufferSize)
{

	ODBAPISettings::instance().setvbufferSize(setvbufferSize);

	odb::Writer<> oda("TestSetvbuffer.odb");
	odb::Writer<>::iterator row = oda.begin();

	MetaData& md = row->columns();
	md.setSize(numberOfColumns);
	for (size_t i = 0; i < numberOfColumns; ++i)
	{
		stringstream name;
		name << "Column" << i;
		row->setColumn(i, name.str().c_str(), odb::REAL);
	}
	row->writeHeader();

	for (long long i = 1; i <= numberOfRows; ++i, ++row)
		for (size_t c = 0; c < numberOfColumns; ++c)
			(*row)[c] = c;
}

void TestSetvbuffer::tearDown()
{
	int catStatus = system("ls -l TestSetvbuffer.odb");
	ASSERT(WEXITSTATUS(catStatus) == 0);
}


} // namespace test 
} // namespace tool 
} // namespace odb 
