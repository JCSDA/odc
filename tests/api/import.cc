/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <memory>

#include "eckit/testing/Test.h"
#include "eckit/io/MemoryHandle.h"

#include "odc/api/Odc.h"

using namespace eckit::testing;

// ------------------------------------------------------------------------------------------------------

CASE("We can import data") {

    const char* SOURCE_DATA =
        R"(col1:INTEGER,col2:REAL,col3:DOUBLE,col4:INTEGER,col5:BITFIELD[a:1;b:2;c:5]
        1,1.23,4.56,7,999
        123,0.0,0.0,321,888
        321,0.0,0.0,123,777
        0,3.25,0.0,0,666
        0,0.0,3.25,0,555)";

    eckit::MemoryHandle dh_in(SOURCE_DATA, ::strlen(SOURCE_DATA));
    eckit::MemoryHandle dh_out;

    ::odc::api::importText(dh_in, dh_out);

    eckit::Log::info() << "Imported length: " << dh_out.position() << std::endl;

    eckit::MemoryHandle readAgain(dh_out.data(), dh_out.position());
    readAgain.openForRead();
    odc::api::Odb o(readAgain);

    while (auto table = o.next()) {
        eckit::Log::info() << "Table: " << table.get().numRows() << std::endl;
    }



}

// ------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    return run_tests(argc, argv);
}
